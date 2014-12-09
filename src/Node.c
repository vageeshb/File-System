/*
 * INode.c
 *
 *  Created on: Dec 5, 2014
 *      Author: vageesh
 */

#include "Node.h"
#include "LibFS.h"
#include "LibDisk.h"
#include "BitMap.h"
#include <string.h>

// Returns the root i_node
void Get_iRoot(i_node *i) {
	char *buffer = (char *)malloc(SECTOR_SIZE);
	Disk_Read(ROOT_INODE, buffer);
	memcpy(i, (i_node *)buffer, sizeof(i_node));
	free(buffer);
	return;
}

// Returns the i_node at an index
void Get_iNode(i_node *i, int index) {
	char *buffer = (char *)malloc(SECTOR_SIZE);
	int sector_num = ROOT_INODE + (index / 4);
	Disk_Read(sector_num, buffer);
	memcpy(i, (i_node *)buffer + (index % 4), sizeof(i_node));
	free(buffer);
	return;
}

// Returns the data_node from the i_node pointed by the index
void Get_dNode(d_node *d, i_node *n, int index) {
	char *buffer = (char *)malloc(SECTOR_SIZE);
	if( (int)n->data_ptr[index] != 0 ) {
		Disk_Read((int)n->data_ptr[index], buffer);
		memcpy(d, (d_node *)buffer, sizeof(d_node));
	}
	free(buffer);
	return;
}

// Helper function to see if a file/directory exists in a parent dir
int Entry_Exists(i_node *node, char *name, int flag) {
	int i, j;
	d_node *d = malloc(sizeof(d_node));

	for( i = 0; i < 30; i++ ) {
		if( node->data_ptr[i] != 0 ) {
			Get_dNode(d, node, i);
			for( j = 0; j < 25; j++ ) {
				if( strcmp(((dir_entry *)d + j)->name, name) == 0 ) {
					/*if ( flag == 1 )
						printf("Directory '%s' exists at '%d' index of block %d\n", name, j, (int)node->data_ptr[i]);*/
					free(d);
					return (10 * i) + j;
				}
			}

		}
	}
	free(d);
	return -1;
}

// Adds the I-node at index
int Add_INode(int index, i_node *node) {
	int sector_num = 5 + (index / 4);
	int sector_offset = index % 4;
	char *buffer = (char *)malloc(SECTOR_SIZE);
	Disk_Read(sector_num, buffer);
	memcpy((i_node *)buffer + sector_offset, node, sizeof(*node));
	Disk_Write(sector_num, buffer);
	free(buffer);
	return 0;
}

// Adds the entry to the parent directory
int Add_Entry_To_DNode(d_node *node, dir_entry *entry) {
	int count = 0;
	while(count < 20) {
		if( ((dir_entry *)node + count)->i_node_num == 0 ) {
			memcpy((dir_entry *)node + count, entry, sizeof(*entry));
			return 0;
		}
		else {
			count++;
		}
	}
	// Data node is full, return error
	return -1;
}

// Adds a new directory or file
int Add_New_Dir_or_File(d_node *parent_node, int location, char *name, int type) {
	int free_inode, free_dnode;

	free_inode = (int)Get_Free_INode();
	free_dnode = (int)Get_Free_DNode() + ROOT_DNODE;

	//printf("%s does not exist, creating '%s'.\n", type == FS_Dir ? "Directory" : "File", name);
	//printf("Assigning IBlock: %d, Assigning DBlock: %d\n", free_inode, free_dnode);

	i_node *i = malloc(sizeof(i_node));
	i->file_size = 0;
	i->file_type = (type == FS_Dir) ? FS_Dir : FS_File;
	i->data_ptr[0] = (int *)free_dnode;

	int j;
	for(j=1; j<30; j++)
		i->data_ptr[j] = 0;

	Add_INode(free_inode, i);

	dir_entry *entry = malloc(sizeof(dir_entry));
	memset(entry, 0, sizeof(dir_entry));
	entry->i_node_num = free_inode;
	strcpy(entry->name, name);

	// Handle return
	Add_Entry_To_DNode(parent_node, entry);
	//printf("Added new entry '%s' to Parent Location - '%d'\n", name, location);
	Disk_Write(location, (char *)parent_node);
	Toggle_BitMap(free_inode, free_dnode);
	free(i);
	free(entry);
	return 0;
}

// Helper function to traverse the node for file/directory name
int Traverse_Nodes(i_node *node, char *name) {
	int rc, index, offset;
	rc = Entry_Exists(node, name, 0);
	if(rc != -1) {
		index = rc / 10;
		offset = rc % 10;
		d_node *d = malloc(sizeof(d_node));
		Get_dNode(d, node, index);
		int i_node_num = ((dir_entry *)d + offset)->i_node_num;
		free(d);
		return i_node_num;
	}
	return (int)NULL;
}

// Creates a new file/directory
int Create_New_Dir_or_File(char *path, int type) {

	int rc, created = 0, parent_i_node = 0;

	i_node *current_i_node = malloc(sizeof(i_node));
	Get_iRoot(current_i_node);
	d_node *current_d_node = malloc(sizeof(d_node));
	Get_dNode(current_d_node, current_i_node, 0);

	char full_path[256] = "";
	char path_cpy[256] = "";
	strcpy(path_cpy, path);

	char *dir_names, *delim = "/";
	dir_names = strtok(path_cpy, delim);

	while(dir_names != NULL) {

		// Track current file path
		strcat(full_path, "/");
		//printf("Checking %s in %s.\n", dir_names, full_path);
		strcat(full_path, dir_names);

		rc = Entry_Exists(current_i_node, dir_names, 1);
		// Directory did not exist, create one
		if(rc == -1) {
			Add_New_Dir_or_File(current_d_node, (int)current_i_node->data_ptr[0], dir_names, type);
			parent_i_node = Traverse_Nodes(current_i_node, dir_names);
			Get_iNode(current_i_node, parent_i_node);
			Get_dNode(current_d_node, current_i_node, 0);
			created = 1;
		}
		// Directory exists, traverse that
		else {
			parent_i_node = Traverse_Nodes(current_i_node, dir_names);
			Get_iNode(current_i_node, parent_i_node);
			Get_dNode(current_d_node, current_i_node, 0);
		}
		dir_names = strtok(NULL, delim);
	}

	if (created == 0) {
		//printf("%s '%s' already exists.\n\n", type == FS_Dir ? "Directory" : "File", full_path);
		osErrno = E_CREATE;
		rc = -1;
	}
	else
		rc = 0;
	//	printf("Created %s '%s'.\n\n", type == FS_Dir ? "directory" : "file", full_path);
	free(current_i_node);
	free(current_d_node);
	return rc;
}
