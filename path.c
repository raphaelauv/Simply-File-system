#include "path.h"
#include "tfs.h"
/**
 * Test if the PATH start with FILE://
 */
void testStartOfPath(const char *path) {
	if (path[0] == 'F' && path[1] == 'I' && path[2] == 'L' && path[3] == 'E'
			&& path[4] == ':' && path[5] == '/' && path[6] == '/') {
	}
	else {
		error er;
		er.val = 1;
		er.message = "The PATH do not start with FILE://";
		testerror(er);
	}
}

/**
 * Tokenize argl every  "/" in array of tokens ( argv )
 * return the number of value in argv
 */
int tokenize_Path(char* argl, char** argv) {
    int i;
    argv[0] = strtok(argl, "/");
    for (i = 0; argv[i] != NULL; ++i)
        argv[i+1] = strtok(NULL, "/");
    return i;
}


/**
 * Print the array of tokens
 */
void printTokenPath(int size,char** tokens) {
	int i;
	for (i = 0; i < size; i++) {
		printf("%s \n", tokens[i]);
	}
}

/**
 * return the array of tokens with one step forward
 */
char** iterateInPath(char** tokens, int * size) {
	char ** tok2;
	if (*size > 0) {
		*size = *size - 1;
		tok2 = &(tokens[1]);

	}else{
		error er;
		er.val=1;
		er.message="iterate limit of Path OR PATH INCORRECT!!";
		testerror(er);
	}

	return tok2;
}


/**
 *Test SIZE of a nameEntry and if it end by \0
 *return the size of the nameEntry
 */
int testNameEntry(char* nameEntry){
	error er;
	int ASCII_END_FOUND = 0;
	int sizeNameEntry=0;
	int i;
	for (i = 0; i < SIZE_MAX_NAME_ENTRY; i++) {
		if (nameEntry[i] == '\0') {
			sizeNameEntry=i;
			ASCII_END_FOUND = 1;
			break;
		}
		sizeNameEntry++;
	}
	if(sizeNameEntry<1){
		er.val = 1;
		er.message = "the name of the folder is smaller than 1 characters :\n";
		testerror(er);
	}

	if (!ASCII_END_FOUND) {
		er.val = 1;
		er.message = "the name of the folder is bigger than  SIZE_MAX_NAME_ENTRY :\n ";
		testerror(er);
	}
	return sizeNameEntry;
}

/**
 *Do a copy of path in copyPath without the start of path
 */
void  copyConstPathWithoutStart(const char *path, char copyPath[]) {
	int i;
	//copy of the const path in copyPah
	for (i = 0; i < MAX_SIZE_PATH; i++) {
		if (path[i] != '\0') {

			//+7 -> we dont need FILE://
			copyPath[i] = path[i + 7];
		} else {
			break;
		}
	}
}

/**
 *  Verification of Path and creation of variable for the next operations
 */
int initPath_Size(const char *path, int *size,char **tabOfPath) {

	testStartOfPath(path);
	char copyPath[MAX_SIZE_PATH];

	copyConstPathWithoutStart(path, copyPath);

	*size = tokenize_Path(copyPath, tabOfPath);

	//the user want create or delete in the HOST
	if (strcmp(tabOfPath[0], "HOST") == 0) {
		perror("use the unix command instead");
		return -1;
		/*
		 //1 step
		 tabOfPath=iterateInPath(tabOfPath,&size);
		 int succes=-1;
		 if (FLAG == FLAG_RMDIR) {
		 succes = rmdir(path);
		 }else if(FLAG == FLAG_MKDIR){
		 succes = mkdir(path, mode);
		 }
		 if(succes==-1){
		 er.val=1;
		 er.message="ERROR MKDIR IN HOST";
		 //testerror(er);
		 }
		 return 0;
		 */
	}
	return 0;
}
