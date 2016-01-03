#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "ll.h"



// the numerotation of block start at 0 in every partition , the block 0 is the TTTFS Description Block
error readBlockOfPartition (partition p,block b,uint32_t numberBlock);
error writeBlockOfPartition (partition p,block b,uint32_t numberBlock);



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

descriptionBlock* initDescriptionBlock();

#define TTTFS_MAGIC_NUMBER 827541076
#define TTTFS_VOLUME_BLOCK_SIZE TFS_VOLUME_BLOCK_SIZE

error getDescriptionBlock(partition p, descriptionBlock* dB);
error writeDescriptionBlock(partition p,descriptionBlock* dB);

error add_OF_FLAG_FreeListe(partition p, uint32_t numberOfValueToAdd, int FLAG);
uint32_t remove_OF_FLAG_FreeListe(partition p, int FLAG);



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
	uint32_t nbFile;
}file;

#define TFS_REGULAR 0
#define TFS_DIRECTORY 1
#define TFS_PSEUDO 2
#define TFS_DATE 0
#define TFS_DISK 1

file* initFile();

file* getFile_Of_FileTab(partition p,uint32_t nbFile);
error writeFile_Of_FileTab(partition p,file* file);

uint32_t getblockNumber_Of_File(uint32_t nbFile);
uint32_t positionInBlock_Of_File(uint32_t nbFile ,uint32_t blockNumber);

#define SIZE_ENTRY_IN_FOLDER 32
#define NUMBER_OF_ENTRY_IN_ONE_BLOCK TFS_VOLUME_BLOCK_SIZE/SIZE_ENTRY_IN_FOLDER

#define ASCII_FOR_POINT 46

#define FLAG_BLOCK 1
#define FLAG_FILE 2

/********************************************************/
/**
 * TTTFS File Table
 */


#define TTTFS_NUMBER_OF_INT_IN_KEY_OF_FILE_TABLE 16 // there is 16 int for 1 entrance in the file table
#define TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK TFS_VOLUME_BLOCK_SIZE/TTTFS_NUMBER_OF_INT_IN_KEY_OF_FILE_TABLE //  1024/16 =64


//only for tfs.h
#define TFS_V_N_V_B_B_2 TFS_VOLUME_NUMBER_VALUE_BY_BLOCK*TFS_VOLUME_NUMBER_VALUE_BY_BLOCK //256*256
#define TFS_V_N_V_B_B_3 TFS_V_N_V_B_B_2 * TFS_VOLUME_NUMBER_VALUE_BY_BLOCK // 256*256*256


#define DIRECT_MAX_SIZE 	DIRECT_TAB * TFS_VOLUME_NUMBER_VALUE_BY_BLOCK // 10*256 = 2560
#define INDIRECT_1_MAX_SIZE DIRECT_MAX_SIZE + TFS_V_N_V_B_B_2//2560 + 256*256 = 68096
#define INDIRECT_2_MAX_SIZE INDIRECT_1_MAX_SIZE + TFS_V_N_V_B_B_3 // 68096 +256*256*256 = 16845312



/********************************************************/
/**
 *  ADD and DELETE ENTRY in TTTFS
 */

#define SIZE_MAX_NAME_ENTRY 28


#define FLAG_ENTRY_FOLDER 1
#define FLAG_ENTRY_FILE 2

int createEmptyEntry(partition p, uint32_t parentFolder,int FLAG);

#define DELETE_FOLDER 1 // OPTION FOR RM

error deleteEntry(partition p, uint32_t nbFile, int FLAG_SECURE , int FOLDER_OPTION);
void delete_File_Indirect2(partition p, uint32_t nbBlock, int FLAG, int FLAG_SECURE);
void delete_File_Indirect1(partition p, uint32_t nbBlock,int FLAG, int FLAG_SECURE);
void delete_File_Direct(partition p, uint32_t nbBlock, int FLAG, int FLAG_SECURE);
error cleanBlock(partition p, uint32_t nbBlock , int FLAG,int FLAG_SECURE);

#define FLAG_DELETE_SECURE 1
#define FLAG_DELETE_SIMPLE 2

/********************************************************/
/**
 * API
 */

int tokenize_command(char* argl, char** argv);


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

