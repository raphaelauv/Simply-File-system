#include "ll.h"

void testerror(error er) {
	if (er.val != 0) {
		fprintf(stderr, "\n%s\n", er.message);
		exit(er.val);

	}
}

/// return the first block of a partition on the TFS , -1 if its impossible
int firstblockPositionOfPartition(int nbPartition, disk_id disk) {


	int * array;
	array = getInfo(disk);
	int nbRealPartitions = array[1];

	if (nbPartition >= nbRealPartitions) {
		return -1;
	}
	int i;
	int tailleBefore=1;//the first block of disk
	for (i = 1; i < nbPartition; i++) {
		tailleBefore=tailleBefore+array[i + 1];
	}

	return tailleBefore;

}

/// Return the size of the n partition
int getSizePartition(int n,disk_id disk){

	error er;
	block *b;
	b = initBlock();
	er = read_block(disk, *b, 0);
	testerror(er);

	if(n<0 || n> nombre32bitsToValue(b->valeur[1]) ){
		error er;
		er.val=1;
		er.message="Error in the n argument of getSizePartition";
		testerror(er);
	}

	int sizePartition=nombre32bitsToValue(b->valeur[n+2]);

	freeBlock(b);

	return sizePartition;
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
	int *array=malloc(5 + nbParitionActual);
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
		er.message = "a problem happend during the closing of the file";
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

void printNombre32bits(nombre32bits *bytes) {
	printf("%x %x %x %x\n", (unsigned char) bytes->val[0],
			(unsigned char) bytes->val[1], (unsigned char) bytes->val[2],
			(unsigned char) bytes->val[3]);

}
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




nombre32bits* fourCharToNombre32bits(int a, int b, int c, int d) {

	nombre32bits * bytes;
	bytes = malloc(sizeof(*bytes));
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
	nombre32bits * bytes;
	bytes = malloc(sizeof(*bytes));

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
	block *block = malloc(sizeof(*block));
	int i;
	for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; i++) {
		block->valeur[i] = malloc(sizeof(nombre32bits));
		block->valeur[i] = valueToNombre32bits(0);
	}
	return block;

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

error static read_physical_block(disk_id id, block b, uint32_t num) {

	error er;
	/*
	printf("position actuelle : %d\n", (int) ftell(id.fichier));
	rewind(id.fichier);
	printf("revenu a zero\n");
	*/
	num=num*TFS_VOLUME_BLOCK_SIZE;
	fseek(id.fichier, num, SEEK_SET);
	//printf("en position : %d\n", num);
	int i;
	int ecris;
	for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; i++) {
		ecris = fread((b.valeur[i]), 1, TFS_VOLUME_DIVISION_OCTAL, id.fichier);
		//printNombre32bits(b.valeur[i]);
		if (ecris != TFS_VOLUME_DIVISION_OCTAL) {
			er.val = 1;
			er.message = "error during write_physical_block in file";
		}
	}
	er.val = 0;
	return er;

}
error static write_physical_block(disk_id id, block b, uint32_t num) {
	int i;
	int ecris;
	num=num*TFS_VOLUME_BLOCK_SIZE;
	fseek(id.fichier, num, SEEK_SET);
	error er;

	for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; i++) {

		if (b.valeur[i] == NULL) {
			printf("null\n");
			b.valeur[i] = valueToNombre32bits(0);
		}
		//printNombre32bits(b.valeur[i]);
		ecris = fwrite(b.valeur[i], 1, TFS_VOLUME_DIVISION_OCTAL, id.fichier);
		if (ecris != TFS_VOLUME_DIVISION_OCTAL) {
			er.val = 1;
			er.message = "error during write_physical_block in file";
		}
	}
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
/*
 int main(int argc, char *argv[]) {

 printf("%s\n", argv[0]);
 char cwd[1024];
 if (getcwd(cwd, sizeof(cwd)) != NULL)
 fprintf(stdout, "Current working dir: %s\n", cwd);
 else
 perror("getcwd() error");
 //disk_id a;
 block *b = malloc(sizeof(*b));

 printNombre32bits(intToNombre32bits(2349));
 b->valeur[0] = intToNombre32bits(2389);
 b->valeur[1] = intToNombre32bits(876);

 char toto[10] = "papapap";
 char* v = toto;
 printf("nom de fichier : %s\n", v);
 disk_id *disque = malloc(sizeof(*disque));

 start_disk(v, disque);

 //write_physical_block(*disque,*b,0);

 block*b2 = initBlock();

 read_physical_block(*disque, *b2, 0);

 free(b2);

 uint32_t a = 4294967295;

 printf("valeur demande : %lu\n", (unsigned long) a);
 nombre32bits*coco = valueToNombre32bits(a);
 printNombre32bits(coco);
 uint32_t toto;
 toto = nombre32bitsToValue(coco);
 printf("toto : %lu\n", (unsigned long) toto);

 return 0;
 }

 */
