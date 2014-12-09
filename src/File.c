/*
 * File.c
 *
 *  Created on: Dec 6, 2014
 *      Author: vageesh
 */

#include "Node.h"
#include "File.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void Setup_FT() {
	int i;
	for(i=0; i < MAX_OPEN_FILES/8; i++)
		File_Table_BM[i] = 0x0;
}

int Get_FT_Index_From_BM() {
	int i, j;
	for(i=0; i < MAX_OPEN_FILES/8; i++)
		for( j=0; j<7; j++ ) {
			if( ((File_Table_BM[i] >> j) & 1) == 0 )
				return 8*i + j;
		}
	return -1;
}

void Set_FT_Index_In_BM(int index) {
	File_Table_BM[index/8] |= 1 << index%8;
}

void Unset_FT_Index_In_BM(int index) {
	File_Table_BM[index/8] &= ~(1 << index%8);
}

int Check_FT_Index_In_BM(int index) {
	return (((1 << index%8) & File_Table_BM[index/8])>>(index%8));
}

int FT_Lookup(char *path) {
	int i;
	for(i=0; i<MAX_OPEN_FILES; i++)
		if( Check_FT_Index_In_BM(i) == 1 && strcmp(File_Table[i].path, path) == 0 )
			return i;
	return -1;
}

int Add_FT_Entry(char *path, int i_node_num, int fp) {
	int ft_index = Get_FT_Index_From_BM();
	if( ft_index == -1 )
		return -1 * FE_MAX_FILES;
	else {
		File_Table_Entry ent = File_Table[ft_index];
		ent.i_node_num = i_node_num;
		ent.w_f_p = fp;
		ent.r_f_p = 0;
		strcpy(ent.path, path);
		File_Table[ft_index] = ent;
		Set_FT_Index_In_BM(ft_index);
		return ft_index;
	}
}

int Open_File(char *path) {
	int rc;

	rc = FT_Lookup(path);
	if( rc != -1) {
		return rc;
	}
	else {
		i_node *current_i_node = malloc(sizeof(i_node));
		Get_iRoot(current_i_node);

		int parent_i_node_num = 0;
		char full_path[MAX_PATH_SIZE] = "", path_cpy[MAX_PATH_SIZE] = "";

		strcpy(path_cpy, path);

		char *dir_names, *delim = "/";
		dir_names = strtok(path_cpy, delim);

		while(dir_names != NULL) {

			// Track current file path
			strcat(full_path, "/");
			//printf("Checking %s in %s.\n", dir_names, full_path);
			strcat(full_path, dir_names);

			rc = Entry_Exists(current_i_node, dir_names, 1);
			// Directory did not exist, break
			if(rc == -1) {
				//printf("Could not find file: '%s', because the path '%s' does not exist.\n", path, full_path);
				free(current_i_node);
				return -1 * FE_NOT_FOUND;
			}
			// Directory exists, traverse that
			else {
				parent_i_node_num = Traverse_Nodes(current_i_node, dir_names);
				Get_iNode(current_i_node, parent_i_node_num);
			}
			dir_names = strtok(NULL, delim);
		}

		// Add FT Entry
		int fd = Add_FT_Entry(path, parent_i_node_num, current_i_node->file_size);

		free(current_i_node);
		return fd;
	}
}
