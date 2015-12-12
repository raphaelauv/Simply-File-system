#include "ll.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char *argv[]){
	 /*
	int i;

    for (i=0; i < argc; i++)
    {
        printf("Argument %ld : %s \n", i+1, argv[i]);
    }
*/

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

    				r = fopen(argv[3], "w"); // en cas d'echec renvoie NULL

    			} else { // default name
    				r = fopen("disk.tfs", "w"); // en cas d'echec renvoie NULL
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
    		fprintf(stderr,"tfs_create need 2 argument :\n -s follow by the size ( a POSITIVE NUMBER)of the new tfs\n [name] who is the name of the tfs\n");
    		return 1;
    	}
    	return 0;
}
