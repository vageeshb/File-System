/*
 * INode.h
 *
 *  Created on: Dec 5, 2014
 *      Author: vageesh
 */

#ifndef NODE_H_
#define NODE_H_

#define ROOT_INODE 5
#define ROOT_DNODE 256

// INode DS
typedef struct {
	int file_size, file_type;
	int *data_ptr[30];
} i_node;

// Data Node DS
typedef struct {
	char data[512];
} d_node;

// Dir Node DS
typedef struct {
	char name[16];
	int i_node_num;
} dir_entry;

typedef enum {
	FS_File,
	FS_Dir
} FS_File_Type;

void Get_iRoot(i_node *i);
void Get_iNode(i_node *i, int index);
void Get_dNode(d_node *d, i_node *node, int index);
int Entry_Exists(i_node *node, char *name, int flag);
int Add_INode(int index, i_node *node);
int Add_Entry_To_DNode(d_node *node, dir_entry *dir);
int Add_New_Dir_or_File(d_node *parent_node, int location, char *name, int type);
int Create_New_Dir_or_File(char *path, int type);
int Traverse_Nodes(i_node *node, char *name);
void Update_Data_Ptr(i_node *n, d_node *d);


#endif /* NODE_H_ */
