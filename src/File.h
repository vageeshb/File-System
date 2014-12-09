/*
 * File.h
 *
 *  Created on: Dec 6, 2014
 *      Author: vageesh
 */

#ifndef FILE_H_
#define FILE_H_

#define MAX_OPEN_FILES 256
#define MAX_PATH_SIZE 256

typedef struct {
	int i_node_num, w_f_p, r_f_p;
	char path[256];
} File_Table_Entry;

char File_Table_BM[MAX_OPEN_FILES];
File_Table_Entry File_Table[MAX_OPEN_FILES];

typedef enum {
	FE_NO_ERR,
	FE_NOT_FOUND,
	FE_MAX_FILES
} FILE_ERR;

void Setup_FT();
int Get_FT_Index_From_BM();
void Set_FT_Index_In_BM(int index);
void Unset_FT_Index_In_BM(int index);
int Check_FT_Index_In_BM(int index);
int FT_Lookup(char *path);
int Add_FT_Entry(char *path, int i_node_num, int fp);
int Open_File(char *path);

#endif /* FILE_H_ */
