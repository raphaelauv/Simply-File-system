#include "tfs.h"

int main(int argc, char *argv[]) {

	/********************************************************/
	/**
	 * TEST OF ARGUMENTS
	 */

	if (argc < 5 || argc > 6 || strcmp(argv[1], "-p") != 0
			|| strcmp(argv[3], "-mf") != 0) {
		fprintf(stderr, "ERREUR WITH ARGUMENTS\n");
		return 1;

	}
	int nbPartition = 0;
	int valreturnSSCRANF = sscanf(argv[2], "%d", &nbPartition);

	if (valreturnSSCRANF != 1 || nbPartition < 0) {
		//printf("val de partition : %d\n",nbPartition);
		fprintf(stderr, "\n-p argument,"
				" the number of partition enter :  %d  is NOT CORRECT \n",
				nbPartition);
		return 1;
	}


	int maxFiles = 0;
	valreturnSSCRANF = sscanf(argv[4], "%d", &maxFiles);

	if (valreturnSSCRANF != 1 || maxFiles < 0) {
		fprintf(stderr, "\n-mf argument,"
				" the number of maximum files enter :  %d  is NOT CORRECT \n",
				nbPartition);
		return 1;
	}

	char* nameFile;
	if (argc != 6) {
		nameFile = "disk.tfs";
		//printf("name NOT give , we search for disk.tfs\n");
	} else {
		nameFile = argv[argc - 1];
		//printf("name give : %s\n", nameFile);
	}

	/**
	 * Call TFS.C format function
	 */
	return tfs_format(nameFile,nbPartition,maxFiles);
}
