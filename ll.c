#include "ll.h"

void testerror(error er) {
	if (er.val != 0) {
		fprintf(stderr, "\n%s\n", er.message);
		exit(er.val);
	}
}

/**
 * return the first block of a partition on the TFS , -1 if its impossible
 */
int firstblockPositionOfPartition(int nbPartition, disk_id disk) {
	int * array;
	array = getInfo(disk);
	int nbRealPartitions = array[1];

	//printf("nb real patition : %d\n",nbRealPartitions);
	if (nbPartition >= nbRealPartitions) {
		return -1;
	}
	int i;

	int tailleBefore=1;//the first block of disk for partition 0

	for (i = 0; i < nbPartition; i++) {
		tailleBefore+=array[i+2];
	}

	free(array);
	//printf("premier block de la partition %d : %d\n",nbPartition,tailleBefore);
	return tailleBefore;
}

/**
 *  Return the size of the n partition
 */
int getSizePartition(int n,disk_id disk){

	int * array;


	array = getInfo(disk);

	int nbRealPartitions = array[1];

	if (n >= nbRealPartitions) {
		return -1;
	}
	int tailleBefore = array[n + 2];

	free(array);
	return tailleBefore;

}

/// return a tab with the information of the first block of the TFS
/*
 the size of array is array[2]+5
 size of drive
 nb of partitions
 size of the i partition
 addition of the size of all partitions
 size availlable on drive
 nb of maximum partition possible
 */
int* getInfo(disk_id disk) {
	error er;
	block *b;

	b = initBlock();

	er = read_block(disk, *b, 0);

	testerror(er);


	int nbParitionActual = nombre32bitsToValue(b->valeur[1]);
	int *array=NULL;
	array=malloc(5*sizeof(int) + nbParitionActual*sizeof(int));
	if(array==NULL){
		er.val=1;
		er.message="MALLOC ERROR getInfo";
		testerror(er);
	}
	array[0] = nombre32bitsToValue(b->valeur[0]); //size of drive
	array[1] = nbParitionActual; //nb of partitions

	int sizeOccupedByPartitions = 0;
	int tailleTmp;
	int i;



	for (i = 0; i < nbParitionActual; i++) {
		tailleTmp = nombre32bitsToValue(b->valeur[2 + i]);
		array[2 + i] = tailleTmp; //size of the i partition
		sizeOccupedByPartitions = sizeOccupedByPartitions + tailleTmp;
	}
	array[2 + nbParitionActual] = sizeOccupedByPartitions; //addition of the size of all partitions
	array[3 + nbParitionActual] = array[0] - 1 - sizeOccupedByPartitions; //size availlable on drive
	int max = 253 - nbParitionActual;


	//nb of maximum partition possible
	if (max > array[3 + nbParitionActual]) {
		array[4 + nbParitionActual] = array[3 + nbParitionActual];
	} else {
		array[4 + nbParitionActual] = max - nbParitionActual;
	}

	freeBlock(b);


	return array;
}

error stop_disk(disk_id id) {
	error er;

	int val;
	val = fclose(id.fichier);
	if (val == 0) {
		er.val = 0;
		er.message = "the file have been correctly close";
	} else {
		er.message = "a problem happened during the closing of the file";
		er.val = 1;
	}
	return er;

}

/*
 Function check_for_endianness() returns 1, if architecture
 is little endian, 0 in case of big endian.
 */

int check_for_endianness() {
	unsigned int x = 1;
	char *c = (char*) &x;
	return (int) *c;
}

/**
 * Print stdout nombre32bits
 */
void printNombre32bits(nombre32bits *bytes) {
	printf("%x %x %x %x\n", (unsigned char) bytes->val[0],
			(unsigned char) bytes->val[1], (unsigned char) bytes->val[2],
			(unsigned char) bytes->val[3]);

}

/**
 * print stdout block
 */
void printBlock(block * block) {

	int i;
	for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; i++) {
		if (block->valeur[i] == NULL) {
			//	printf("null\n");
			return;
		}
		printNombre32bits(block->valeur[i]);
	}

}

int charToInt(char a){
	return (int)a;
}

/**
 * DEPRECED
 */
char* nombre32bitsToFourChar(nombre32bits *bytes) {
	char nb[TFS_VOLUME_DIVISION_OCTAL];
	char * nbPoint =nb;
	if (check_for_endianness() == 0) {

		nb[0] = (bytes->val[0]) << 24;
		nb[1] = (bytes->val[1]) << 16;
		nb[2] = (bytes->val[2]) << 8;
		nb[3] = bytes->val[3];

	} else {

		nb[0] = (bytes->val[3]) << 24;
		nb[1] = (bytes->val[2]) << 16;
		nb[2] = (bytes->val[1]) << 8;
		nb[3] = bytes->val[0];

	}
	printf("val nbPoint[0] : %d \n",nb[0]);
	return nbPoint;
}

nombre32bits* fourCharToNombre32bits(int a, int b, int c, int d) {

	nombre32bits * bytes= NULL;
	bytes= malloc(sizeof(nombre32bits));
	if(bytes==NULL){
		error er;
		er.val=1;
		er.message="error in malloc fourCharToNombre32bits";
		testerror(er);
	}

	/*
	if (check_for_endianness() == 0) {
*/
		bytes->val[0] = a;
		bytes->val[1] = b;
		bytes->val[2] = c;
		bytes->val[3] = d;
/*
	} else {
		printf("dans endianes char\n");
		bytes->val[3] = a;
		bytes->val[2] = b;
		bytes->val[1] = c;
		bytes->val[0] = d;
	}*/
	return bytes;
}

uint32_t nombre32bitsToValue(nombre32bits *bytes) {
	//maximum value : FFFFFFFF -> 4294967295;

	uint32_t n;
	uint32_t number1;
	uint32_t number2;
	uint32_t number3;
	uint32_t number4;
	if (check_for_endianness() == 0) {

		number1 = (bytes->val[0]) << 24;
		number2 = (bytes->val[1]) << 16;
		number3 = (bytes->val[2]) << 8;
		number4 = bytes->val[3];

	} else {

		number1 = (bytes->val[3]) << 24;
		number2 = (bytes->val[2]) << 16;
		number3 = (bytes->val[1]) << 8;
		number4 = bytes->val[0];

	}
	n = number1 + number2 + number3 + number4;
	/*
	 printf(" val 1 : %lu , val 2 : %lu , val 3 : %lu , val 4 : %lu \n"
	 "l'addition ; %lu\n",(unsigned long)number1,(unsigned long)number2,
	 (unsigned long)number3,(unsigned long)number4,(unsigned long)n);*/
	return n;
}

nombre32bits* valueToNombre32bits(uint32_t n) {

	//maximum value : FFFFFFFF -> 4294967295;
	nombre32bits * bytes = NULL;
	bytes = malloc(sizeof(nombre32bits));
	if(bytes==NULL){
		error er;
		er.val=1;
		er.message="erreur malloc valueToNombre32bits";
		testerror(er);
	}

	if (check_for_endianness() == 0) {

		bytes->val[0] = (n >> 24) & 0xFF;
		bytes->val[1] = (n >> 16) & 0xFF;
		bytes->val[2] = (n >> 8) & 0xFF;
		bytes->val[3] = n & 0xFF;

	} else {
		bytes->val[3] = (n >> 24) & 0xFF;
		bytes->val[2] = (n >> 16) & 0xFF;
		bytes->val[1] = (n >> 8) & 0xFF;
		bytes->val[0] = n & 0xFF;
	}

	return bytes;

}
block* initBlock() {
	error er;
	block *b= NULL;
	b= malloc(sizeof(block));
	if(b==NULL){
		er.val=1;
		er.message="Error malloc initblock";
		testerror(er);
	}

	int i;

	for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; i++) {
		b->valeur[i] = valueToNombre32bits(0);
	}

	return b;

}

void freeBlock(block* block) {
	int i;
	for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; i++) {
		free(block->valeur[i]);
	}
	free(block);
}

void freeDisk(disk_id*disk) {
	free(disk);
}


/**
 * TO convert the block struct to an array for write_physical_block
 */
void static convertBlockToArray(block b, unsigned char array[]) {
	int i;
	int j;
	for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; i++) {
		for (j = 0; j < TFS_VOLUME_DIVISION_OCTAL; j++) {
			//printf("val k %d : %d \n ",(i*4) + j ,b.valeur[i]->val[j]);
			array[(i*4) + j] = b.valeur[i]->val[j];
		}
	}
}


/**
 * TO convert the array of read_physical_block to a block struct
 */
void static convertArrayToBlock(unsigned char array[], block b) {
	int i;
	int j;
	for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; i++) {
		for (j = 0; j < TFS_VOLUME_DIVISION_OCTAL; j++) {
			b.valeur[i]->val[j] = array[(i*4) + j];
			//printf("en  %d  vaut %d\n",(i*4)+j , array[(i*4) + j]);

		}
		//printf("valeur %d du block : %d\n",i,nombre32bitsToValue(b.valeur[i]));
	}
}

error static read_physical_block(disk_id id, block b, uint32_t num) {

	error er;
	int lus;

	num=num*TFS_VOLUME_BLOCK_SIZE;
	fseek(id.fichier, num, SEEK_SET);
	//printf("en position : %d\n", num);
	//int i;


	unsigned char array[TFS_VOLUME_BLOCK_SIZE];

	//for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; i++) {
		//lus = fread((b.valeur[i]), 1, TFS_VOLUME_DIVISION_OCTAL, id.fichier);

		lus = fread(array, 1, TFS_VOLUME_BLOCK_SIZE, id.fichier);
		//printNombre32bits(b.valeur[i]);
		//printf("val read : %d\n",lus);
		if (lus != TFS_VOLUME_BLOCK_SIZE) {
			er.val = 1;
			er.message = "error during write_physical_block in file";
		}

	convertArrayToBlock(array,b);
	//}
	er.val = 0;
	return er;

}
error static write_physical_block(disk_id id, block b, uint32_t num) {
	int i;
	int ecris;
	num=num*TFS_VOLUME_BLOCK_SIZE;
	fseek(id.fichier, num, SEEK_SET);
	error er;
	unsigned char array[TFS_VOLUME_BLOCK_SIZE];
	convertBlockToArray(b,array);

	//for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; i++) {

		//ecris = fwrite(b.valeur[i], 1, TFS_VOLUME_DIVISION_OCTAL, id.fichier);
		ecris = fwrite(array, 1, TFS_VOLUME_BLOCK_SIZE, id.fichier);
		if (ecris != TFS_VOLUME_BLOCK_SIZE) {
			er.val = 1;
			er.message = "error during write_physical_block in file";
		}

	//}
	fflush(id.fichier);
	er.val = 0;
	return er;
}
error read_block(disk_id id, block b, uint32_t num) {
	error er;

	er = read_physical_block(id, b, num);
	return er;
}
error write_block(disk_id id, block b, uint32_t num) {
	error er;
	er = write_physical_block(id, b, num);
	return er;
}

error start_disk(char *name, disk_id *id) {
	error er;
	FILE *fichier;
	fichier = fopen(name, "r+");
	if (fichier == NULL) {
		er.val = 1;
		er.message = "Error at the opening of the file specified,\n"
				" if it do not exist \n you should execute tfs_create";
		return er;
	}
	id->fichier = fichier;
	er.val = 0;
	return er;
}

error sync_disk(disk_id id) {
	error er;
	return er;
}
