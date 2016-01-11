#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
	printf("here");

	int i;
	int aInt;
	char str[15];

	for(i=0;i<300;i++){

		aInt=i;

		sprintf(str, "%d", aInt);

		char* s = strcat("FILE://toto/0/momo", str);
		char *argv2[]={"mkdir",s};

		pid_t pid =fork();
		switch(pid){
		case 0 :
			execv("mkdir",argv2);
			break;
		case 1:
			wait();
			break;
		}
	}

	return 0;
}
