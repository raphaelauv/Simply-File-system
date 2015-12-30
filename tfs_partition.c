#include "ll.h"

int main(int argc, char *argv[]) {

	/********************************************************/
	/**
	 * TEST OF ARGUMENTS
	 */

	if (argc == 2 && strcmp(argv[1], "-p") == 0) {
		fprintf(stderr,
				"\ntfs_partition with the option -p \nneed at minimum 1 argument :"
						" the size , who is the number of bloc \nof the new parition \n");
		return 1;
	}
	if (argc > 2 && strcmp(argv[1], "-p") == 0) {

		int NameSelected;
		if ((argc - 1) % 2 == 0) {
			//there is no name give at the end
			NameSelected = 0;
		} else {
			NameSelected = 1;
		}

		if (NameSelected == 1 && strcmp(argv[argc - 1], "-p") == 0) {
			fprintf(stderr,
					"\nthe name of disk cant be -p , you probably forget a value for your last partition \n");
			return 1;
		}

		/********************************************************/
		/**
		 * TEST OF PARTITION(S) ASK BY USER
		 */

		int i;
		int nbPartitions = 0;

		int valSizeOfPartition;
		int valreturnSSCRANF;

		int sizeTotalOfPartition = 0;
		int sizeOfNewPartitions[253];

		for (i = 1; i < argc - 1 - NameSelected; i++) {

			if (strcmp(argv[i], "-p") == 0) {
				valSizeOfPartition = 0;
				valreturnSSCRANF = sscanf(argv[i + 1], "%d",
						&valSizeOfPartition);
				if (valreturnSSCRANF != 1 || valSizeOfPartition < 1) {
					fprintf(stderr, "\n-p argument,"
							" the size enter :  %d  is not CORRECT \n",
							valSizeOfPartition);
					return 1;
				}
				sizeOfNewPartitions[nbPartitions] = valSizeOfPartition;
				nbPartitions++;
				sizeTotalOfPartition = sizeTotalOfPartition
						+ valSizeOfPartition;

				i++;
			} else {
				fprintf(stderr, "\nerreur in arguments :  -p size ...\n");
				return 1;
			}

		}

		printf(
				"nb of new partitions : %d \n size total des nouvel Partitions : %d\n",
				nbPartitions, sizeTotalOfPartition);

		/********************************************************/
		/**
		 * TEST PARTITION(S)  WITH ASKED DISK
		 */

		disk_id *disk = malloc(sizeof(*disk));
		error er;
		char* nameFile;
		if (NameSelected == 0) {
			nameFile = "disk.tfs";

		} else {
			nameFile = argv[argc - 1];
		}
		block *b;
		b = initBlock();
		er = start_disk(nameFile, disk);
		testerror(er);
		er = read_block(*disk, *b, 0);
		testerror(er);

		int sizeActual = nombre32bitsToValue(b->valeur[0]);
		int nbParitionActual = nombre32bitsToValue(b->valeur[1]);

		printf("taille du disque : %d | nb of actual partitions  %d\n",
				sizeActual, nbParitionActual);

		int sizeOccupedByPartitions = 0;

		for (i = 0; i < nbParitionActual; i++) {
			sizeOccupedByPartitions = sizeOccupedByPartitions
					+ nombre32bitsToValue(b->valeur[2 + i]);
		}
		printf("taille occupé par partitions : %d\n", sizeOccupedByPartitions);

		int sizeAvailable = sizeActual - 1 - sizeOccupedByPartitions;
		if (sizeAvailable < sizeTotalOfPartition) {
			er.message = "\nthe total of size's partition(s)\n "
					"is bigger than the available size on the drive\n";
			er.val = 1;
			testerror(er);

		}
		int nbParitionTOtal = nbParitionActual + nbPartitions;

		if (nbParitionTOtal > TFS_MAX_NUMBER_OF_PARTITION) {
			er.message = "\nthe number of partitions is limited to %d\n",TFS_MAX_NUMBER_OF_PARTITION;

			er.val = 1;
			testerror(er);
		}

		/********************************************************/
		/**
		 * CREATE THE PARTITION(S)
		 */

		b->valeur[1] = valueToNombre32bits(nbParitionTOtal);

		int j = 0;

		printf("nb partitions actual %d\n", nbParitionActual);
		for (i = 0; i < nbPartitions; i++) {

			int val = 2 + nbParitionActual + i;

			b->valeur[val] = valueToNombre32bits(sizeOfNewPartitions[j]);

			printNombre32bits(b->valeur[val]);

			printf("partition nb : %d    | taille partition :%d\n", val - 1,
					sizeOfNewPartitions[j]);
			j++;
		}

		er = write_block(*disk, *b, 0);
		printf("\nposition partition 5 :%d\n",
				firstblockPositionOfPartition(5, *disk));
		testerror(er);
		er = stop_disk(*disk);
		testerror(er);

		/********************************************************/
		/**
		 * FREE MEMORY
		 */

		freeBlock(b);
		freeDisk(disk);

		return 0;

	} else {
		fprintf(stderr,
				"\ntfs_create need at minimum 1 argument :\n -p"
						"follow by the size ( a POSITIVE NUMBER)of "
						"the new partition \n [name] is optional,  it is the name of the tfs\n");
		return 1;
	}
}
