#include "ll.h"

int main(int argc, char *argv[]) {

	char* nameFile;
	if (argc == 2) {
		nameFile = argv[1];
	}
	else{
		nameFile = "disk.tfs";
	}

	disk_id *disk = malloc(sizeof(*disk));
	error er;
	block *b;
	b=initBlock();
	er = start_disk(nameFile, disk);
	testerror(er);
	er = read_block(*disk, *b, 0);
	testerror(er);

	int * array;
	array=getInfo(b);

	if(array[0]==0){
		er.val=1;
		er.message="\nthe file selected is 0 size or not yet initialised with tfs_create\n";
		testerror(er);
	}
	printf("TAILLE DISQUE : %d\n",array[0]);

	int nbPartitions=array[1];
	printf("NOMBRE DE PARTITIONS : %d\n",nbPartitions);

	int i;
	for(i=1;i<array[1]+1;i++){
		printf("PARTITION : %d  | TAILLE : %d\n",i , array[i+1]);
	}

	printf("TAILLE OCCUPE AU TOTAL PAR PARTITIONS : %d\n",array[nbPartitions+2]);
	printf("ESPACE DISPONIBLE : %d\n",array[nbPartitions+3]);
	printf("NOMBRE MAX DE PARTITION ENCORE CREABLE : %d\n",array[nbPartitions+4]);

	er = stop_disk(*disk);
		testerror(er);
		freeBlock(b);
		freeDisk(disk);
	return 0;
}
