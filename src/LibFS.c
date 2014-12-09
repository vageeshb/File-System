#include "LibFS.h"
#include "LibDisk.h"
#include "BitMap.h"
#include "Node.h"
#include "File.h"
#include <string.h>

// global errno value here
int osErrno;
char MAGIC[SECTOR_SIZE] = "19101989";
char *FS_PATH;

int INode_Setup() {
	int i, j;

	// Allocate 255 blocks for I-Nodes
	char *buffer = (char *)malloc(SECTOR_SIZE);
	memset(buffer, 0, SECTOR_SIZE);

	for(i = ROOT_INODE; i < ROOT_DNODE; i++) {
		if( Disk_Write(i, buffer) == -1 ) {
			free(buffer);
			return -1;
		}
	}

	// Writing root i_node
	((i_node *)buffer)->file_size = 0;
	((i_node *)buffer)->file_type = FS_Dir;
	((i_node *)buffer)->data_ptr[0] = (int *)ROOT_DNODE;

	for(j=1; j<30; j++)
		((i_node *)buffer)->data_ptr[j] = 0;

	if( Disk_Write(ROOT_INODE, buffer) == -1 ) {
		printf("Could not create root i-node.\n");
		free(buffer);
		return -1;
	}

	/*printf("Root i-node created successfully.\n");
	printf("INode Setup completed.\n");*/

	free(buffer);
	return 0;
}

int FS_Boot(char *path)
{
    printf("FS_Boot %s\n", path);
    FS_PATH = path;

    // oops, check for errors
    if (Disk_Init() == -1) {
		printf("Disk_Init() failed\n");
		osErrno = E_GENERAL;
		return -1;
    }

    // Load disk into memory
    if(Disk_Load(path) == -1) {
    	// Could not open file, create new FS
    	if( diskErrno == 2 ) {
    		// Write Super block, bitmaps and i-nodes
    		if( Disk_Write(0, MAGIC) == -1 ||
    				BitMap_Setup() == -1 ||
    				INode_Setup() == -1 ) {
    			printf("Could not create new FS\n");
    			osErrno = E_GENERAL;
    			return -1;
    		}

    		if( Disk_Save(path) == -1 ) {
    			printf("Could not save new FS\n");
    			osErrno = E_GENERAL;
    			return -1;
    		}

    		printf("New FS created successfully!\n");
    	}
    	else {
    		printf("Disk_Load Failed!\n");
    		osErrno = E_GENERAL;
    		return -1;
    	}
    }
    // Disk load was successful, check size and super-block
    else {
    	char buffer[SECTOR_SIZE];
    	if( Disk_Read(0, buffer) == -1 ) {
    		printf("Could not read from disk.\n");
    		osErrno = E_GENERAL;
    		return -1;
    	}
    	if( strcmp(buffer, MAGIC) != 0 ) {
    		printf("Super-block corrupted.\n");
    		osErrno = E_GENERAL;
    		return -1;
    	}
    }

    Setup_FT();
    printf("FS Boot Successful!\n");
    return 0;
}

int FS_Sync()
{
	if( Disk_Save(FS_PATH) == -1 ) {
		printf("Could not save new FS\n");
		osErrno = E_GENERAL;
		return -1;
	}
	printf("FS Synchronization Successful!\n");
    return 0;
}


int File_Create(char *file)
{
    printf("File_Create %s\n", file);
    return Create_New_Dir_or_File(file, FS_File);
    return 0;
}

int File_Open(char *file)
{
    printf("File_Open %s\n", file);
    int rc = Open_File(file);
    switch(rc) {
    	case -1:
    		osErrno = E_NO_SUCH_FILE;
    		return -1;
    	case -2:
    		osErrno = E_TOO_MANY_OPEN_FILES;
    		return -1;
    	default:
    		//printf("File opened successfully, FD: %d\n", rc);
    		return rc;
    }
}

int File_Read(int fd, void *buffer, int size)
{

    if( Check_FT_Index_In_BM(fd) != 1 ) {
    	osErrno = E_BAD_FD;
    	return -1;
    }

    printf("File_Read: '%s'\n", File_Table[fd].path);

	i_node *node = malloc(sizeof(i_node));
	Get_iNode(node, File_Table[fd].i_node_num);

	int current_fp = File_Table[fd].r_f_p;

	if(node->file_size <= 0 || current_fp == node->file_size) {
		free(node);
		return 0;
	}

	char *read_buffer = (char *)malloc(SECTOR_SIZE);
	memset(read_buffer, 0, SECTOR_SIZE);

	int total_read_size, ptr_index, readable_buffer_size, sector_num;

	total_read_size = 0;

	ptr_index = current_fp / SECTOR_SIZE;

	if(node->file_size < size)
		size = node->file_size;

	if ((node->file_size - current_fp) < size)
		size = (node->file_size - current_fp);

	while(size > 0) {

		if(((SECTOR_SIZE - (current_fp % SECTOR_SIZE)) / size) > 0)
			readable_buffer_size = size;
		else
			readable_buffer_size = SECTOR_SIZE - (current_fp % SECTOR_SIZE);

		sector_num = (int)node->data_ptr[ptr_index];

		if( ptr_index >= 30 || sector_num == 0 )
			break;

		//printf("RFP: %d, RBZ: %d, Z: %d, TRZ: %d\n", current_fp, readable_buffer_size, size, total_read_size);

		Disk_Read(sector_num, read_buffer);
		memcpy(buffer, (char *)read_buffer + (current_fp %  SECTOR_SIZE), readable_buffer_size);

		ptr_index++;
		size -= readable_buffer_size;
		current_fp += readable_buffer_size;
		buffer += readable_buffer_size;
		total_read_size += readable_buffer_size;
	}

	File_Table[fd].r_f_p = current_fp;

	printf("Read %d bytes to buffer from file '%s'\n", total_read_size, File_Table[fd].path);

	free(node);
	free(read_buffer);

    return total_read_size;
}

int File_Write(int fd, void *buffer, int size)
{
	if( Check_FT_Index_In_BM(fd) != 1 ) {
		osErrno = E_BAD_FD;
		return -1;
	}

    int i_node_num = File_Table[fd].i_node_num;
    i_node *node = malloc(sizeof(i_node));
    Get_iNode(node, File_Table[fd].i_node_num);

    if(node->file_size >= SECTOR_SIZE * 30) {
		printf("ERROR: The file is full, file size: %d\n", node->file_size);
		osErrno = E_NO_SPACE;
		free(node);
		return -1;
	}
    else if(node->file_size + size >= SECTOR_SIZE * 30) {
		printf("ERROR: File too big!\n");
		osErrno = E_FILE_TOO_BIG;
		free(node);
		return -1;
	}

    char *read_buffer = malloc(SECTOR_SIZE);
    int data_size, ptr_index, writable_buffer_size, sector_num, d_node_index, current_fp;
    data_size = size;

    current_fp = File_Table[fd].w_f_p;

    d_node_index = (int)node->data_ptr[current_fp / SECTOR_SIZE];

    while(size > 0) {

    	writable_buffer_size = SECTOR_SIZE - current_fp % SECTOR_SIZE;

    	if( size < writable_buffer_size )
    		writable_buffer_size = size;

    	ptr_index = current_fp / SECTOR_SIZE;

    	if(ptr_index >= 30)
    		break;

    	if( node->data_ptr[ptr_index] == 0 ) {
    		d_node_index = ROOT_DNODE + Get_Free_DNode();
    		//printf("\nin here: %d\n", d_node_index);
    		Toggle_DNode_BitMap(d_node_index);
    	}
    	//printf("PTR: %d\n", ptr_index);
    	node->data_ptr[ptr_index] = (int *)d_node_index;
    	sector_num = d_node_index;

    	//printf("WBZ: %d, SN: %d\n", writable_buffer_size, sector_num);

    	Disk_Read(sector_num, read_buffer);
		memcpy((char *)read_buffer + (current_fp % SECTOR_SIZE), buffer, writable_buffer_size);
		Disk_Write(sector_num, read_buffer);

    	size -= writable_buffer_size;
    	current_fp += writable_buffer_size;
    	buffer += writable_buffer_size;
    }

    node->file_size = current_fp;

    int i_sector = ROOT_INODE + i_node_num / 4;
    int i_offset = i_node_num % 4;
    char *i_buffer = malloc(SECTOR_SIZE);
    Disk_Read(i_sector, i_buffer);
    memcpy((i_node *)i_buffer + i_offset, node, sizeof(i_node));
    Disk_Write(i_sector, i_buffer);

    File_Table[fd].w_f_p = current_fp;

    printf("Written %d bytes to file %s, final file size: %d\n", data_size, File_Table[fd].path, current_fp);

    free(node);
    free(read_buffer);
    free(i_buffer);

    return 0;
}

int File_Seek(int fd, int offset)
{
    printf("FS_Seek\n");
    return 0;
}

int File_Close(int fd)
{
    // Check FT in BM, 0 is not present, 1 is present
    if (Check_FT_Index_In_BM(fd) == 0) {
    	osErrno = E_BAD_FD;
    	return -1;
    }
    else {
    	printf("FS_Close '%s'\n", File_Table[fd].path);
    	Unset_FT_Index_In_BM(fd);
    	printf("File '%s' closed, Descriptor %d deleted.\n", File_Table[fd].path, fd);
    }
    return 0;
}

int File_Unlink(char *file)
{
    printf("FS_Unlink\n");
    return 0;
}

// directory ops
int Dir_Create(char *path)
{
	printf("Dir create %s\n", path);
	return Create_New_Dir_or_File(path, FS_Dir);
}

int Dir_Size(char *path)
{
    printf("Dir_Size\n");
    return 0;
}

int Dir_Read(char *path, void *buffer, int size)
{
    printf("Dir_Read: '%s'\n", path);

    // Root case
    i_node *current_i_node = malloc(sizeof(i_node));
    Get_iRoot(current_i_node);

	int parent_i_node_num = 0, rc;
	char path_cpy[MAX_PATH_SIZE] = "";

	strcpy(path_cpy, path);

    char *dir_names, *delim = "/";
	dir_names = strtok(path_cpy, delim);

	while(dir_names != NULL) {

		rc = Entry_Exists(current_i_node, dir_names, 0);
		// Directory did not exist, break
		if(rc == -1) {
			printf("Directory '%s' does not exist.\n", dir_names);
			free(current_i_node);
			return -1;
		}
		// Directory exists, traverse that
		else {
			parent_i_node_num = Traverse_Nodes(current_i_node, dir_names);
			Get_iNode(current_i_node, parent_i_node_num);
		}
		dir_names = strtok(NULL, delim);
	}

	// Read contents of the directory
	int available_size = size;
	int i, j, buffer_ptr = 0;
	d_node *d = malloc(sizeof(d_node));

	for(i = 0; i<30; i++) {
		if(current_i_node->data_ptr[i] != 0) {
			Get_dNode(d, current_i_node, i);
			for( j = 0; j < 25; j++ ) {
				if( strcmp(((dir_entry *)d + j)->name, "") != 0 ) {

					// Check for available buffer
					if(available_size < sizeof(dir_entry)) {
						osErrno = E_BUFFER_TOO_SMALL;
						free(d);
						return -1;
					}

					memcpy((dir_entry *)buffer + buffer_ptr, (dir_entry *)d+j, sizeof(dir_entry));
					available_size -= sizeof(dir_entry);
					buffer_ptr ++;
				}
			}

		}
	}

	free(current_i_node);
	free(d);
    return buffer_ptr;
}

int Dir_Unlink(char *path)
{
    printf("Dir_Unlink\n");
    return 0;
}
