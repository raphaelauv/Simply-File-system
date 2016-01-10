#include "ll.h"

error createFile(char *name) {

	FILE *fichier;

	error er;

	fichier = fopen(name, "r+");
	if (fichier == NULL) {
		//printf("the file does not exit , we gone a try to create it\n");
		fichier = fopen(name, "a");
		if (fichier == NULL) {
			er.val = 1;
			return er;
		}
		//printf("the file %s have been created\n ", name);
		er.message = "the file have been created";
		er.val = 0;
	}
	fclose(fichier);
	er.message = "the file alreay exist";
	er.val = 0;
	return er;

}

int main(int argc, char *argv[]) {

	if (argc > 2 && strcmp(argv[1], "-s") == 0) {

		int size;
		int valReturn = sscanf(argv[2], "%d", &size);

		if (valReturn != 1 || size < 2) {
			fprintf(stderr, "tfs_create -s need 1 argument :"
					" the size enter :  %d  is not CORRECT \n", size);
			return 1;

		} else {
			error er;
			disk_id *disk = malloc(sizeof(*disk));
			if(disk==NULL){
				er.val=1;
				er.message="ERROR MALLOC DISK in TFS_CREATE";
				testerror(er);
			}

			char* nameFile;

			if (argc > 3) {  //the user give a name to the file
				nameFile = argv[3];
			} else {
				nameFile = "disk.tfs";
			}

			er = createFile(nameFile);
			testerror(er);
			er = start_disk(nameFile, disk);
			testerror(er);

			int sizeInOctal = size * 1024;

			/*
			 int i=0;
			 for(i; i<sizeInOctal;i++){
			 fputc(45,r);
			 }

			 ftruncate(fileno(r) sizeInOctal);
			 deplace le curseur a la position max-1
			 */
			fseek(disk->fichier, (sizeInOctal - 1), SEEK_CUR);
			fputc(0, disk->fichier); //ecris un octet en position max
			//Il faut reouvrir le fichier en mode binaire apres avoir fait un fputc
			fseek(disk->fichier, 0, SEEK_CUR);
			stop_disk(*disk);

			er = start_disk(nameFile, disk);
			testerror(er);
			block *b;
			b = initBlock();
			free(b->valeur[0]);
			b->valeur[0] = valueToNombre32bits(size);
			//b->valeur[1] = valueToNombre32bits(1);// there is 1 partition at first
			//b->valeur[2] = valueToNombre32bits(size-1);
			er=write_block(*disk, *b, 0);
			testerror(er);

			/*
			 read_block(*disk,*b,0);
			 printf("dans fonction afficahge de block apres read\n");
			 printBlock(b);
			 */
			er=stop_disk(*disk);
			testerror(er);
			freeBlock(b);
			freeDisk(disk);
			return 0;
		}
	} else {
		fprintf(stderr,
				"tfs_create need at minimum 1 argument :\n -s "
						"follow by the size ( a POSITIVE NUMBER)of "
						"the new tfs\n [name] is optional,  it is the name of the tfs\n");
		return 1;
	}
	return 0;
}

