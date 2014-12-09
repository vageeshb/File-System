/*
 * BitMap.c
 *
 *  Created on: Dec 5, 2014
 *      Author: vageesh
 */
#include "LibDisk.h"
#include "Node.h"
#include "BitMap.h"
#include <string.h>

void print(char bitmap) {
	int j;
	for(j=7;j>=0; j--) {
		printf("%d", (bitmap >> j) & 1);
	}
	printf("\n");
}

// Generates a bitmap
void Generate_Bitmap(char *bitmap) {
	int i, num;
	for( i=0; i < BITMAP_SIZE; i++ ) {
		// Initialize the bitmap to all 0
		bitmap[i] = 0x0;
		// Random number to corrupt
		num = rand() % 7;
		// Root
		if(i==0)
			num = 0;
		bitmap[i] |= 1 << num;
	}
}

// Sets up inode and dnode bitmaps
int BitMap_Setup() {

	char *bitmap = (char *)malloc(SECTOR_SIZE);
	memset(bitmap, 0, SECTOR_SIZE);

	Generate_Bitmap(bitmap);

	// Inode bitmap
	if( Disk_Write(1, bitmap) == -1)
		return -1;

	Generate_Bitmap(bitmap);
	// Data node bitmap
	if( Disk_Write(2, bitmap) == -1 )
		return -1;

	//printf("BitMap Setup completed.\n");
	free(bitmap);
	return 0;
}

// Fetches first free inode
int Get_Free_INode() {
	int i, j;
	char *bitmap = (char *)malloc(SECTOR_SIZE);

	if(Disk_Read(1, bitmap) == -1) {
		printf("Could not read bitmap.\n");
		return -1;
	}

	for ( i=0; i < BITMAP_SIZE; i++ )
		for( j=0; j<7; j++ )
			if( ((bitmap[i] >> j) & 1) == 0 ) {
				free(bitmap);
				return 8*i + j;
			}

	free(bitmap);
	return -1;
}

// Fetches first free dnode
int Get_Free_DNode() {
	int i, j;
	char *bitmap = (char *)malloc(SECTOR_SIZE);

	if(Disk_Read(2, bitmap) == -1) {
		printf("Could not read bitmap.\n");
		return -1;
	}
	for ( i=0; i < BITMAP_SIZE; i++ )
		for( j=0; j<7; j++ )
			if( ((bitmap[i] >> j) & 1) == 0 ) {
				free(bitmap);
				return 8*i + j;
			}

	free(bitmap);
	return -1;
}

int Toggle_BitMap(int i, int d) {
	char *bitmap = malloc(SECTOR_SIZE);

	// INode Bitmap
	Disk_Read(1, bitmap);
	bitmap[i/8] |= 1 << i%8;
	Disk_Write(1, bitmap);

	// DNode Bitmap
	d = d - ROOT_DNODE;
	Disk_Read(2, bitmap);
	bitmap[d/8] |= 1 << d%8;
	Disk_Write(2, bitmap);

	free(bitmap);
	return 0;
}

int Toggle_DNode_BitMap(int d) {
	char *bitmap = malloc(SECTOR_SIZE);

	// DNode Bitmap
	d = d - ROOT_DNODE;
	Disk_Read(2, bitmap);
	bitmap[d/8] |= 1 << d%8;
	Disk_Write(2, bitmap);

	free(bitmap);
	return 0;
}
