#include "ll.h"

/*
   Function check_for_endianness() returns 1, if architecture
   is little endian, 0 in case of big endian.
 */


error stop_disk(disk_id id){
	error er;

	int val;
	val=fclose(id.fichier);
	if(val==0){

		er.val=0;
		er.message="the file have been correctly close";
	}
	else{
		er.message="a problem happend during the closing of the file";
		er.val=1;
	}
	return er;

}
int check_for_endianness()
{
  unsigned int x = 1;
  char *c = (char*) &x;
  return (int)*c;
}

void printNombre32bits (nombre32bits *bytes){
	printf("%x %x %x %x\n", (unsigned char)bytes->val[0],
			                        (unsigned char)bytes->val[1],
			                        (unsigned char)bytes->val[2],
			                        (unsigned char)bytes->val[3]);

}
void printBlock(block * block){

	int i;
	for(i=0;i<256;i++){
		if(block->valeur[i]==NULL){
				//	printf("null\n");
					return;
				}
		printNombre32bits(block->valeur[i]);
	}

}

nombre32bits* intToNombre32bits (int n){

	nombre32bits * bytes;
	bytes=malloc(sizeof(*bytes));


	if( check_for_endianness()== 0) {

		bytes->val[0] = (n >> 24) & 0xFF;
		bytes->val[1] = (n >> 16) & 0xFF;
		bytes->val[2] = (n >> 8) & 0xFF;
		bytes->val[3] = n & 0xFF;

	}
	else{
		bytes->val[3] = (n >> 24) & 0xFF;
		bytes->val[2] = (n >> 16) & 0xFF;
		bytes->val[1] = (n >> 8) & 0xFF;
		bytes->val[0] = n & 0xFF;
	}

	return bytes;

}
block* initBlock(){
	block *block=malloc(sizeof(*block));
	int i;
	for(i=0;i<256;i++){
		block->valeur[i]=malloc(sizeof(nombre32bits));
		block->valeur[i]=intToNombre32bits(10);
	}
	return block;

}

error read_physical_block(disk_id id,block b,uint32_t num){

	error er;
	printf("position actuelle : %d\n",(int)ftell(id.fichier));
	rewind(id.fichier);
	printf("revenu a zero\n");
	fseek(id.fichier, num, SEEK_SET);
	printf("en position : %d\n",num);
	int i;
	for(i=0;i<256;i++){
		fread((b.valeur[i]),(sizeof(unsigned char)),4,id.fichier);
		printNombre32bits(b.valeur[i]);
		//printf("position actuelle : %d\n",(int)ftell(id.fichier));

	}
	/*block *bPoint=&b;
	printf("debut affichage \n");
	printBlock(bPoint);
	printf("fini\n");*/

	er.val=0;
	return er;

}
error write_physical_block(disk_id id,block b,uint32_t num){
	int i;
	for(i=0;i<256;i++){

		if(b.valeur[i]==NULL){
			//printf("null\n");
			b.valeur[i]=intToNombre32bits(-i);
		}

		printNombre32bits(b.valeur[i]);
		fwrite( b.valeur[i] , 1, 4 , id.fichier);
	}
	error er;
	er.val=0;
	return er;
}



error start_disk(char *name,disk_id *id){
	printf("dans start\n");
	error er;
	FILE *fichier;
	fichier=fopen(name,"r+");
	if(fichier==NULL){
		printf("the file does not exit , we gone a try to create it\n");
		fichier=fopen(name,"a");
		if(fichier==NULL){
			er.message="fail of creation of file";
			er.val=1;
			return er;
		}
		fclose(fichier);
		fichier=fopen(name,"r+");
		if(fichier==NULL){
			er.val=1;
			return er;
				}

		printf("the file %s have been created\n ",name);
	}
	id->fichier=fichier;
	er.val=0;
	return er;
}






int main(int argc, char *argv[]){

	printf("%s\n",argv[0]);
	char cwd[1024];
	   if (getcwd(cwd, sizeof(cwd)) != NULL)
	       fprintf(stdout, "Current working dir: %s\n", cwd);
	   else
	       perror("getcwd() error");
	//disk_id a;
	block *b=malloc(sizeof(*b));


	printNombre32bits(intToNombre32bits(2349));
	b->valeur[0]=intToNombre32bits(2389);
	b->valeur[1]=intToNombre32bits(876);

	char toto[10]="papapap";
	char* v=toto;
	printf("nom de fichier : %s\n",v);
	disk_id *disque=malloc(sizeof(*disque));

	start_disk(v,disque);

	//write_physical_block(*disque,*b,0);

	block*b2=initBlock();

	read_physical_block(*disque,*b2,0);
	return 0;
}
