#include "tfs.h"

int main(int argc, char *argv[]) {



	/********************************************************/
	/**
	 * TEST OF ARGUMENTS
	 */

	if (argc < 5 || argc > 6 || strcmp(argv[1], "-p") != 0
			|| strcmp(argv[3], "-mf") != 0) {
		printf("ERREUR WITH ARGUMENTS");
		return 1;

	}
	int nbPartition = 0;
	int valreturnSSCRANF = sscanf(argv[2], "%d", &nbPartition);

	if (valreturnSSCRANF != 1 || nbPartition < 0) {
		fprintf(stderr, "\n-p argument,"
				" the number of partition enter :  %d  is NOT CORRECT \n",
				nbPartition);
		return 1;
	}
	int file_count = 0;
	valreturnSSCRANF = sscanf(argv[4], "%d", &file_count);

	if (valreturnSSCRANF != 1 || file_count < 0) {
		fprintf(stderr, "\n-mf argument,"
				" the number of maximum files enter :  %d  is NOT CORRECT \n",
				nbPartition);
		return 1;
	}

	disk_id *disk = malloc(sizeof(*disk));
	error er;
	char* nameFile;
	if (argc != 6) {
		nameFile = "disk.tfs";
		//printf("name NOT give , we search for disk.tfs\n");
	} else {
		nameFile = argv[argc - 1];
		//printf("name give : %s\n", nameFile);
	}


	/********************************************************/
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
	//printf("position : %d\n",positionFirstBlock);

	partition *p=malloc(sizeof(partition));
	p->disque=disk;
	p->firstPositionInTFS=positionFirstBlock;

	er=readBlockOfPartition(*p,*b,0);
	testerror(er);


	if(nombre32bitsToValue(b->valeur[0])==TTTFS_MAGIC_NUMBER){
		fprintf(stderr, "the partition already have "
				"the magic number so tfs_format already been use \n");
			return 1;
	}

	b->valeur[0] = valueToNombre32bits(TTTFS_MAGIC_NUMBER);
	b->valeur[1] = valueToNombre32bits(TFS_VOLUME_BLOCK_SIZE);

	int sizePartition=getSizePartition(nbPartition,*disk);
	int nbFreeBlock =sizePartition-1;

	//printf("size partition ask : %d\n",sizePartition);
	b->valeur[2] = valueToNombre32bits(sizePartition);

	/********************************************************/
	/** MAX FILE
	 *
	 */
	int nbOfBlockForFileTab = file_count/TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK;

	if(nbOfBlockForFileTab<1){ nbOfBlockForFileTab=1;}

	//printf("\n nb of block for file tab : %d\n",nbOfBlockForFileTab);

	//+1 is the root folder un-delatable
	if(nbFreeBlock-(nbOfBlockForFileTab+file_count+1)<0){
		fprintf(stderr, "\n-mf file_count argument,"
						" the number of maximum files enter :  %d  is NOT CORRECT \n"
						"because is bigger than what is possible even, if all files have the minimun size.\n"
						"The maxmimum is %d files in this partition\n",file_count,
						nbFreeBlock-(nbOfBlockForFileTab+1));
		return 1;
	}
	//printf("val de file count : %d \n", file_count);

	//-1 because of the root
	b->valeur[5] = valueToNombre32bits(file_count);

	/********************************************************/
	/**
	 * WRITE BLOCK OF DESCRIPTION
	 */

	er=writeBlockOfPartition(*p,*b,0);
	testerror(er);

	/********************************************************/
	/**
	 * TTTFS File Table
	 */

	file_count=file_count+1;// to add the ROOT un-delatable


	int i;

	for(i=file_count-1; i>-1 ; i--){
		printf("add file \n");
		er=add_OF_FLAG_FreeListe(*p,i,FLAG_FILE);
		testerror(er);
	}
	/********************************************************/
	/**
	 * TTTFS Data blocks & Free Blocks Chain
	*/

	int j;

	int limit = nbOfBlockForFileTab;
	for(j=sizePartition-1;j>limit;j--){
		printf("add block \n");
		er=add_OF_FLAG_FreeListe(*p,j,FLAG_BLOCK);
		testerror(er);
	}
	/********************************************************/
	/**
	 * CREATE THE ROOT
	 */
	createEmptyFolder(*p,0);

	/********************************************************/
	/**
	 * FREE MEMORY
	 */
	freeBlock(b);
	freeDisk(disk);
	free(p);



	return 0;

}

