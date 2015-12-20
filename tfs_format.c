#include "tfs.h"

int main(int argc, char *argv[]) {


	if (argc < 5 || argc > 6 || strcmp(argv[1], "-p") != 0
			|| strcmp(argv[3], "-mf") != 0) {
		printf("ERREUR WITH ARGUMENTS");
		return 1;

	}
	int nbPartition = 0;
	int valreturnSSCRANF = sscanf(argv[2], "%d", &nbPartition);

	if (valreturnSSCRANF != 1 || nbPartition < 1) {
		fprintf(stderr, "\n-p argument,"
				" the number of partition enter :  %d  is NOT CORRECT \n",
				nbPartition);
		return 1;
	}
	int file_count = 0;
	valreturnSSCRANF = sscanf(argv[4], "%d", &file_count);

	if (valreturnSSCRANF != 1 || file_count < 1) {
		fprintf(stderr, "\n-mf argument,"
				" the number of maximum files enter :  %d  is NOT CORRECT \n",
				nbPartition);
		return 1;
	}

	//printf("partition demande %d \n",nbPartition);
	disk_id *disk = malloc(sizeof(*disk));
	error er;
	char* nameFile;
	if (argc != 6) {
		nameFile = "disk.tfs";
		printf("name NOT give\n");
	} else {
		nameFile = argv[argc - 1];
		printf("name give : %s\n", nameFile);
	}

	/**
	 * TTTFS Description Block
	 */
	block *b;
	b = initBlock();
	er = start_disk(nameFile, disk);
	testerror(er);
	int positionFirstBlock=firstblockPositionOfPartition(nbPartition,*disk);

	if(positionFirstBlock ==-1){
		er.val=1;
		er.message="The partition selected do not exist in the tfs";
		testerror(er);
	}
	printf("position : %d\n",positionFirstBlock);

	partition *p=malloc(sizeof(partition));
	p->disque=disk;
	p->firstPositionInTFS=positionFirstBlock;

	er=readBlockOfPartition(*p,*b,p->firstPositionInTFS);

	er = read_block(*disk, *b,positionFirstBlock);
	testerror(er);


	b->valeur[0] = valueToNombre32bits(TTTFS_MAGIC_NUMBER);
	b->valeur[1] = valueToNombre32bits(TFS_VOLUME_BLOCK_SIZE);

	int sizePartition=getSizePartition(nbPartition,*disk);
	int nbFreeBlock =sizePartition-1;

	printf("size partition ask : %d\n",sizePartition);
	b->valeur[2] = valueToNombre32bits(sizePartition);
	b->valeur[3] = valueToNombre32bits(0);
	b->valeur[4] = valueToNombre32bits(0);
	b->valeur[5] = valueToNombre32bits(0);
	b->valeur[6] = valueToNombre32bits(0);
	b->valeur[7] = valueToNombre32bits(0);

	/**
	 * TTTFS File Table
	 */

	/**
	 * TTTFS Data blocks & Free Blocks Chain
	 */


	return 0;
}

