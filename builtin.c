#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "disqueDur.h"

/* _____________________________________________________ */
// Synopsis:  exit [STATUS]
int execute_command_exit(int argc, char** argv) {
	if (argc == 1)
		exit(0);
	if (argc > 2) {
		fprintf(stderr, "exit: Un argument au maximum.\n");
		return 1;
	}
	char* endptr;  // Indiquera où 'strlol' s'est arrêté.
	int val = strtol(argv[1], &endptr, 10);
	if (*endptr != '\0') {
		fprintf(stderr, "exit: Argument doit etre un entier.\n");
		return 1;
	}
	if (val < 0) {
		fprintf(stderr, "exit: Argument doit etre >= 0.\n");
		return 1;
	}
	exit(val);
}
/* _____________________________________________________ */

int execute_command_tfs_create(int argc, char** argv) {



	if (argc == 2 && strcmp(argv[1], "-s") == 0) {

		fprintf(stderr, "tfs_create with the option -s need 1 argument :"
				" the size , who is the number of bloc of the new tfs\n");
		return 1;
	}
	else if (argc > 2 && strcmp(argv[1], "-s") == 0) {

		int valeur;
		int valretour = sscanf(argv[2], "%d", &valeur);
		if (valretour != 1 || valeur < 2) {
			fprintf(stderr, "tfs_create -s need 1 argument :"
					" the size enter :  %d  is not CORRECT \n", valeur);
			return 1;
		}
		else {
			FILE *r;
			if (argc > 3) {  //the user give a name to the file

				r = fopen(argv[3], "w"); // en cas d'échec renvoie NULL

			} else { // default name
				r = fopen("disk.tfs", "w"); // en cas d'échec renvoie NULL
			}
			if (r == NULL) {
				fprintf(stderr, "tfs_create , the name enter is creating a problem \n");
				return 1;
			}
			int sizeInOctal = valeur * 1024;


			/*
			 int i=0;
			 for(i; i<sizeInOctal;i++){
			 fputc(45,r);
			 }
			 */
			//ftruncate(fileno(r) sizeInOctal);
			//deplace le curseur a la position max-1



			fseek(r, (sizeInOctal - 1), SEEK_CUR);
			fputc(0, r); //ecris un octet en position max
			fseek(r, 0, SEEK_CUR);
			fclose(r);
			printf("succes\n");
			return 0;
		}
	} else {

		printf("ici\n");
		fprintf(stderr,"tfs_create need 2 argument :\n -s follow by the size ( a POSITIVE NUMBER)of the new tfs\n [name] who is the name of the tfs\n");
		return 1;
	}
	return 0;
}
