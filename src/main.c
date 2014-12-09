#include	<stdio.h>
#include	<string.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<unistd.h>
#include	<errno.h>
#include	<fcntl.h>

#include	"LibFS.h"
#include	"LibDisk.h"
#include 	"Node.h"
#include 	"File.h"

extern Disk_Error_t diskErrno;

void
usage( char *image )
{
	fprintf( stderr, "usage: %s <disk image file>\\n", image );
	exit( 1 );
}

int
main( int argc, char *argv[] )
{
	int rc, file_ptr, fd;
	ssize_t count;
	struct stat buf;
	char file_name[ 16 ], dir_name[256], actual_path[ 256 ], path[ 256 ], buffer[ SECTOR_SIZE ];

	if( argc != 2 )
	{
		usage( argv[ 0 ] );
	}

	char *image = argv[ 1 ];

	rc = stat( image, &buf );

	if( ( rc == -1 ) && ( errno == ENOENT ) )
	{
		// The named file does not exist, create the file system.

		printf( "The image %s does not exist, initialize it...\n", image );

		if( ( rc = FS_Boot( image ) ) == -1 )
		{
			printf( "Error initializing file system...\n" );
			printf( "\tdiskErrno = %d\n", diskErrno );
			osErrno = E_GENERAL;
			exit( -1 );
		}
	}
	else
	{
		if( ( rc = FS_Boot( image ) ) == -1 )
		{
			printf( "Error loading file system from disk image...\n" );
			printf( "\tdiskErrno = %d\n", diskErrno );
			osErrno = E_GENERAL;
			exit( -1 );
		}
	}

	// File system created or restored from the file image; synchronize it before working with the FS.

	if( ( rc = FS_Sync() ) == -1 )
	{
		printf( "Error synch-ing file systems to disk...\n" );
		printf( "\tdiskErrno = %d\n", diskErrno );
		osErrno = E_GENERAL;
		exit( -1 );
	}

	// Create some directories

	strcpy( dir_name, "/dir1" );

	if( ( rc = Dir_Create( dir_name ) ) == -1 )
	{
		printf( "Error creating directory %s...\n", dir_name );
		printf( "\tosErrno = %d\n", osErrno );
		exit( -1 );
	}

	strcpy( dir_name, "/dir2" );
	if( ( rc = Dir_Create( dir_name ) ) == -1 )
	{
		printf( "Error creating directory %s...\n", dir_name );
		printf( "\tosErrno = %d\n", osErrno );
		exit( -1 );
	}

	strcpy( dir_name, "/dir3/dir4" );
	if( ( rc = Dir_Create( dir_name ) ) == -1 )
	{
		printf( "Error creating directory %s...\n", dir_name );
		printf( "\tosErrno = %d\n", osErrno );
		exit( -1 );
	}

	strcpy( path, "/dir1/one.txt" );
	file_ptr = File_Create( path );
	file_ptr = File_Open( path );


	strcpy( actual_path, "./all-text/one.txt" );

	if( ( fd = open( actual_path, O_RDONLY) ) != -1 )
	{
		while( ( count = read( fd, buffer, SECTOR_SIZE ) ) != 0 )
		{
			buffer[ count ] = '\0';
			printf( "Read %s (%d bytes) from file %s, adding to file %s in our file system...\n", buffer, count, actual_path, path );
			File_Write( file_ptr, buffer, count );
		}

		File_Close( file_ptr );
		close( fd );
	}

	strcpy( path, "/dir1/two.txt" );
	file_ptr = File_Create( path );
	file_ptr = File_Open( path );

	strcpy( actual_path, "./all-text/two.txt" );

	if( ( fd = open( actual_path, O_RDONLY) ) != -1 )
	{
		while( ( count = read( fd, buffer, SECTOR_SIZE ) ) != 0 )
		{
			buffer[ count ] = '\0';
			printf( "Read %s (%d bytes) from file %s, adding to file %s in our file system...\n", buffer, count, actual_path, path );
			File_Write( file_ptr, buffer, count );
		}

		File_Close( file_ptr );
		close( fd );
	}

	strcpy( path, "/dir2/three.txt" );
	file_ptr = File_Create( path );
	file_ptr = File_Open( path );

	strcpy( actual_path, "./all-text/three.txt" );

	if( ( fd = open( actual_path, O_RDONLY) ) != -1 )
	{
		while( ( count = read( fd, buffer, SECTOR_SIZE ) ) != 0 )
		{
			buffer[ count ] = '\0';
			printf( "Read %s (%d bytes) from file %s, adding to file %s in our file system...\n", buffer, count, actual_path, path );
			File_Write( file_ptr, buffer, count );
		}

		File_Close( file_ptr );
		close( fd );
	}

	strcpy( path, "/dir1/four.txt" );
	file_ptr = File_Create( path );
	file_ptr = File_Open( path );

	strcpy( actual_path, "./all-text/four.txt" );

	if( ( fd = open( actual_path, O_RDONLY) ) != -1 )
	{
		while( ( count = read( fd, buffer, SECTOR_SIZE ) ) != 0 )
		{
			buffer[ count ] = '\0';
			printf( "Read %s (%d bytes) from file %s, adding to file %s in our file system...\n", buffer, count, actual_path, path );
			File_Write( file_ptr, buffer, count );
		}

		File_Close( file_ptr );
		close( fd );
	}

	strcpy( path, "/dir2/five.txt" );
	file_ptr = File_Create( path );
	file_ptr = File_Open( path );

	strcpy( actual_path, "./all-text/five.txt" );

	if( ( fd = open( actual_path, O_RDONLY) ) != -1 )
	{
		while( ( count = read( fd, buffer, SECTOR_SIZE ) ) != 0 )
		{
			buffer[ count ] = '\0';
			printf( "Read %s (%d bytes) from file %s, adding to file %s in our file system...\n", buffer, count, actual_path, path );
			File_Write( file_ptr, buffer, count );
		}

		File_Close( file_ptr );
		close( fd );
	}

	strcpy( path, "/dir1/six.txt" );
	file_ptr = File_Create( path );
	file_ptr = File_Open( path );

	strcpy( actual_path, "./all-text/six.txt" );

	if( ( fd = open( actual_path, O_RDONLY) ) != -1 )
	{
		while( ( count = read( fd, buffer, SECTOR_SIZE ) ) != 0 )
		{
			buffer[ count ] = '\0';
			printf( "Read %s (%d bytes) from file %s, adding to file %s in our file system...\n", buffer, count, actual_path, path );
			File_Write( file_ptr, buffer, count );
		}

		File_Close( file_ptr );
		close( fd );
	}

	strcpy( path, "/dir2/seven.txt" );
	file_ptr = File_Create( path );
	file_ptr = File_Open( path );

	strcpy( actual_path, "./all-text/seven.txt" );

	if( ( fd = open( actual_path, O_RDONLY) ) != -1 )
	{
		while( ( count = read( fd, buffer, SECTOR_SIZE ) ) != 0 )
		{
			buffer[ count ] = '\0';
			printf( "Read %s (%d bytes) from file %s, adding to file %s in our file system...\n", buffer, count, actual_path, path );

			File_Write( file_ptr, buffer, count );
		}

		File_Close( file_ptr );
		close( fd );
	}

	strcpy( path, "/dir1/eight.txt" );
	file_ptr = File_Create( path );
	file_ptr = File_Open( path );

	strcpy( actual_path, "./all-text/eight.txt" );

	if( ( fd = open( actual_path, O_RDONLY) ) != -1 )
	{
		while( ( count = read( fd, buffer, SECTOR_SIZE ) ) != 0 )
		{
			buffer[ count ] = '\0';
			printf( "Read %s (%d bytes) from file %s, adding to file %s in our file system...\n", buffer, count, actual_path, path );
			File_Write( file_ptr, buffer, count );
		}

		File_Close( file_ptr );
		close( fd );
	}

	strcpy( path, "/dir1/nine.txt" );
	file_ptr = File_Create( path );
	file_ptr = File_Open( path );

	strcpy( actual_path, "./all-text/nine.txt" );

	if( ( fd = open( actual_path, O_RDONLY) ) != -1 )
	{
		while( ( count = read( fd, buffer, SECTOR_SIZE ) ) != 0 )
		{
			buffer[ count ] = '\0';
			printf( "Read %s (%d bytes) from file %s, adding to file %s in our file system...\n", buffer, count, actual_path, path );
			File_Write( file_ptr, buffer, count );
		}

		File_Close( file_ptr );
		close( fd );
	}

	strcpy( path, "/dir1/ten.txt" );
	file_ptr = File_Create( path );
	file_ptr = File_Open( path );

	strcpy( actual_path, "./all-text/ten.txt" );

	if( ( fd = open( actual_path, O_RDONLY) ) != -1 )
	{
		while( ( count = read( fd, buffer, SECTOR_SIZE ) ) != 0 )
		{
			buffer[ count -1 ] = '\0';
			printf( "Read %s (%d bytes) from file %s, adding to file %s in our file system...\n", buffer, count, actual_path, path );
			File_Write( file_ptr, buffer, count );
		}

		File_Close( file_ptr );
		close( fd );
	}

	// Directory read
	strcpy( path, "/" );
	printf("Files found: %d\n", Dir_Read(path, buffer, SECTOR_SIZE));

	// File read
	strcpy( path, "/dir1/one.txt" );
	file_ptr = File_Open( path );
	printf("%d bytes read from file descriptor '%d'\n", File_Read(file_ptr, buffer,512), file_ptr);
	File_Close(file_ptr);

	strcpy( path, "/dir1/two.txt" );
	file_ptr = File_Open( path );
	printf("%d bytes read from file descriptor '%d'\n", File_Read(file_ptr, buffer,512), file_ptr);
	printf("%d bytes read from file descriptor '%d'\n", File_Read(file_ptr, buffer,512), file_ptr);
	printf("%d bytes read from file descriptor '%d'\n", File_Read(file_ptr, buffer,512), file_ptr);
	File_Close(file_ptr);

	// File system created or restored from the file image; synchronize it before working with the FS.

	if( ( rc = FS_Sync() ) == -1 )
	{
		printf( "Error synch-ing file systems to disk...\n" );
		printf( "\tdiskErrno = %d\n", diskErrno );
		osErrno = E_GENERAL;
		exit( -1 );
	}

	return( 0 );
}
