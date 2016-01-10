#ifndef PATH_H

#define PATH_H

#include "tfs.h"

void testStartOfPath(const char *path);
int  tokenize_Path(char* argl, char** argv);
void  printTokenPath(int size,char** tokens);
char** iterateInPath(char** tokens, int * size);
int testNameEntry(char* nameEntry);
void  copyConstPathWithoutStart(const char *path, char copyPath[]);
int initPath_Size(const char *path, int *size,char **tabOfPath);

#endif
