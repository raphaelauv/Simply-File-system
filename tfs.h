#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include "ll.h"

typedef struct {
} DIR;

typedef struct {
	int magic;
	int volumeBlockSize;
	int volumeFreeBlockNb;
	int volumeFirstFreeBlock;
	int volumeMaxFile;
	int volumeFreeFileNb;
	int volumeFirstFreeFile;
}descriptionBlock;

typedef struct {
	int tfs_size;
	int tfs_type;
	int tfs_subtype;
	int tfs_direct[10];
	int tfs_indirect1;
	int tfs_indirect2;
	int tfs_next_free;
}file;

#define FLAG_BLOCK 1
#define FLAG_FILE 2

#define TTTFS_NUMBER_OF_INT_IN_KEY_OF_FILE_TABLE 16 // there is 16 int for 1 entrance in the file table
#define TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK TFS_VOLUME_BLOCK_SIZE/TTTFS_NUMBER_OF_INT_IN_KEY_OF_FILE_TABLE //  1024/16 =64


#define TTTFS_MAGIC_NUMBER 827541076
#define TTTFS_VOLUME_BLOCK_SIZE TFS_VOLUME_BLOCK_SIZE
#define TFS_REGULAR 0
#define TFS_DIRECTORY 1
#define TFS_PSEUDO 2
#define TFS_DATE 0
#define TFS_DISK 1

error readBlockOfPartition (partition p,block b,uint32_t numberBlock);
error writeBlockOfPartition (partition p,block b,uint32_t numberBlock);

error getDescriptionBlock(partition p, descriptionBlock* dB);
error writeDescriptionBlock(partition p,descriptionBlock* dB);


error putThingInFreeList(partition p, descriptionBlock * dB,uint32_t numberOfValueToAdd, int FLAG);
error removeThingOfFreeListe(partition p,descriptionBlock * dB,uint32_t numberOfValueToAdd , int FLAG);

error getFile_Of_FileTab(partition p,uint32_t nbFile,file* file);
error writeFile_Of_FileTab(partition p,uint32_t nbFile,file* file);

uint32_t getblockNumber_Of_File(uint32_t nbFile);
uint32_t positionInBlock_Of_File(uint32_t nbFile ,uint32_t blockNumber);



file* initFile();
descriptionBlock* initDescriptionBlock();

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

