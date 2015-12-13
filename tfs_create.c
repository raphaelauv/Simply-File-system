#include "ll.h"


int main(int argc, char *argv[]){

    if (argc == 2 && strcmp(argv[1], "-s") == 0) {

    		fprintf(stderr, "tfs_create with the option -s need 1 argument :"
    				" the size , who is the number of bloc of the new tfs\n");
    		return 1;
    	}
    	else if (argc > 2 && strcmp(argv[1], "-s") == 0) {
    		int DefaultName;
    		int size;
    		int valretour = sscanf(argv[2], "%d", &size);
    		if (valretour != 1 || size < 2) {
    			fprintf(stderr, "tfs_create -s need 1 argument :"
    					" the size enter :  %d  is not CORRECT \n", size);
    			return 1;
    		}
    		else {
    			disk_id *disk=malloc(sizeof(*disk));
    			error er;
    			if (argc > 3) {  //the user give a name to the file
    				er=start_disk(argv[3],disk);
    				DefaultName=0;
    			} else { // default name
    				er=start_disk("disk.tfs",disk);
    				DefaultName=1;
    			}
			if (er.val != 0) {
				fprintf(stderr, "%s", er.message);
				return er.val;
			}
    			int sizeInOctal = size * 1024;

    			/*
    			 int i=0;
    			 for(i; i<sizeInOctal;i++){
    			 fputc(45,r);
    			 }
    			 */
    			//ftruncate(fileno(r) sizeInOctal);
    			//deplace le curseur a la position max-1



    			fseek(disk->fichier, (sizeInOctal - 1), SEEK_CUR);
    			fputc(0, disk->fichier); //ecris un octet en position max
    			//Il faur reouvrir le fichier en mode binaire apres avoir fait un fputc
    			fseek(disk->fichier, 0, SEEK_CUR);
    			stop_disk(*disk);

    			if(DefaultName==1){
    				er=start_disk("disk.tfs",disk);
    			}
    			else{
    				er=start_disk(argv[3],disk);
    			}
    			block *b;
    			b=initBlock();
    			b->valeur[0]=intToNombre32bits(size);
    			write_block(*disk,*b,0);

    			/*
    			read_block(*disk,*b,0);
    			printf("dans fonction afficahge de block apres read\n");
    			printBlock(b);
    			*/
    			stop_disk(*disk);
    			freeDisk(disk);
    			return 0;
    		}
    	} else {
    			fprintf(stderr,"tfs_create need at minimum 1 argument :\n -s "
    				"follow by the size ( a POSITIVE NUMBER)of "
    				"the new tfs\n [name] is optional,  it is the name of the tfs\n");
    		return 1;
    	}
    	return 0;
}

