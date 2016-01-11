#ifndef TFS_H
#define TFS_H

#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "ll.h"

//#include "path.h"

typedef struct {
	uint32_t firstPositionInTFS;
	disk_id *disque;
} partition;

/********************************************************/
/**
 * TTTFS Description Block
 */
typedef struct {
	uint32_t magic;
	uint32_t volumeBlockSize;
	uint32_t volumeBlockCount;
	uint32_t volumeFreeBlockNb;
	uint32_t volumeFirstFreeBlock;
	uint32_t volumeMaxFile;
	uint32_t volumeFreeFileNb;
	uint32_t volumeFirstFreeFile;
}descriptionBlock;

#define TTTFS_MAGIC_NUMBER 827541076
#define TTTFS_VOLUME_BLOCK_SIZE TFS_VOLUME_BLOCK_SIZE

/********************************************************/
/**
 * FILE
 */

#define DIRECT_TAB 10

typedef struct {
	uint32_t tfs_size;
	uint32_t tfs_type;
	uint32_t tfs_subtype;
	uint32_t tfs_direct[DIRECT_TAB];
	uint32_t tfs_indirect1;
	uint32_t tfs_indirect2;
	uint32_t tfs_next_free;

	uint32_t nbFile;//useful only for the implementation
}file;

#define TFS_REGULAR 0
#define TFS_DIRECTORY 1
#define TFS_PSEUDO 2
#define TFS_DATE 0
#define TFS_DISK 1

#define SIZE_MAX_NAME_ENTRY 28
#define SIZE_ENTRY_IN_FOLDER TFS_VOLUME_DIVISION_OCTAL+SIZE_MAX_NAME_ENTRY //32
#define NUMBER_OF_ENTRY_IN_ONE_BLOCK TTTFS_VOLUME_BLOCK_SIZE/SIZE_ENTRY_IN_FOLDER

#define ASCII_FOR_POINT 46

#define FLAG_BLOCK 1
#define FLAG_FILE 2

/********************************************************/
/**
 * TTTFS File Table
 */

#define TTTFS_NUMBER_OF_INT_IN_KEY_OF_FILE_TABLE 16 // there is 16 int for 1 entrance in the file table
#define TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK (TTTFS_VOLUME_BLOCK_SIZE/(TTTFS_NUMBER_OF_INT_IN_KEY_OF_FILE_TABLE*TFS_VOLUME_DIVISION_OCTAL) )//  1024/16 =64

/*********************/
//only for tfs.h
#define TFS_V_N_V_B_B_2 TFS_VOLUME_NUMBER_VALUE_BY_BLOCK*TFS_VOLUME_NUMBER_VALUE_BY_BLOCK //256*256
#define TFS_V_N_V_B_B_3 TFS_V_N_V_B_B_2 * TFS_VOLUME_NUMBER_VALUE_BY_BLOCK // 256*256*256
/*********************/

#define DIRECT_MAX_SIZE 	DIRECT_TAB * TFS_VOLUME_NUMBER_VALUE_BY_BLOCK // 10*256 = 2560
#define INDIRECT_1_MAX_SIZE DIRECT_MAX_SIZE + TFS_V_N_V_B_B_2//2560 + 256*256 = 68096
#define INDIRECT_2_MAX_SIZE INDIRECT_1_MAX_SIZE + TFS_V_N_V_B_B_3 // 68096 +256*256*256 = 16845312

/********************************************************/
/**
 *  ADD and DELETE ENTRY in TTTFS
 */

#define FLAG_ENTRY_FOLDER 1
#define FLAG_ENTRY_FILE 2

#define FLAG_DELETE_FOLDER 1 // OPTION FOR RM to delete recursively folders
#define FLAG_DELETE_SECURE 1
#define FLAG_DELETE_SIMPLE 2

/********************************************************/
/**
 * API
 */

#define MAX_SIZE_PATH 500
#define MAX_SIZE_OF_FOLDER_IN_PATH 300

#define FLAG_RMDIR 	1
#define FLAG_MKDIR 	2
#define FLAG_RENAME 3
#define FLAG_OPEN 	4

int tfs_format(char* nameFile,int nbPartition,int maxFiles);
int tfs_mkdir(const char *path, mode_t mode);
int tfs_rmdir(const char *path);
int tfs_rename(const char *old, const char *new);
int tfs_open(const char *name,int oflag, ...);

ssize_t tfs_read(int fildes,void *buf,size_t nbytes);
ssize_t tfs_write(int fildes,void *buf,size_t nbytes);
int tfs_close(int fildes);
off_t tfs_lseek(int fildes,off_t offset,int whence);

DIR *opendir(const char *filename);
struct dirent *readdir(DIR *dirp);
void rewinddir(DIR *dirp);
int closedir(DIR *dirp);


/********************************************************/
/**
 * PATH SEARCHING
 */
#define FLAG_SEARCH_ONLY 1
#define FLAG_SEARCH_DELETE 2

/********************************************************/
/**
 * FILE DESCRIPTOR , from correction TP6
 */

#define MY_EOF 			(-1)  // Indicateur de fin de fichier.
#define tfs_FOPEN_MAX 	64    // Nombre maximal de fichiers ouverts.
#define tfs_BUFSIZ 		TTTFS_VOLUME_BLOCK_SIZE  // Taille d'un buffer

// Type de structure représentant un fichier.
typedef struct {
	partition p;		 // partition of the file
    int nbFile;          // number of file in fileTab
    int flags;           // Informations sur le fichier.
    unsigned char* buf;  // Début du buffer.
    unsigned char* pos;  // Position actuelle dans le buffer.
    int count;           // Nb d'octets/places restant ds le buffer.
} tfs_FILE;


// Les flags possibles d'une structure de type MY_FILE.
#define tfs_READ 1    // Fichier en mode lecture.
#define tfs_WRITE 2   // Fichier en mode écriture.
#define tfs_NOBUF 4   // On n'utilise pas de buffer.
#define tfs_EOF 8     // Fin de fichier atteinte.
#define tfs_ERROR 16  // Une erreur s'est produite.

#endif
