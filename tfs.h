
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
} DIR;




#define TTTFS_MAGIC_NUMBER 827541076
#define TTTFS_VOLUME_BLOCK_SIZE TFS_VOLUME_BLOCK_SIZE


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
