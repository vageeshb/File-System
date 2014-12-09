/*
 * BitMap.h
 *
 *  Created on: Dec 5, 2014
 *      Author: vageesh
 */

#ifndef BITMAP_H_
#define BITMAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BITMAP_SIZE 125

void Generate_BitMap(char *bitmap);
int BitMap_Setup();
int Get_Free_INode();
int Get_Free_DNode();
int Toggle_BitMap(int i, int d);
int Toggle_DNode_BitMap(int d);

#endif /* BITMAP_H_ */
