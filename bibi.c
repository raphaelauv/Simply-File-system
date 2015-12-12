#include "ll.h"


error read_physical_block(disk_id id,block b,uint32_t num){
	error* er;

	FILE *r;
	r = fopen(id.name, "wb");

	if (r == NULL) {
	    fprintf(stderr, "tfs_create , the name enter is creating a problem \n");
	    return *er;
	    			}
	return *er;

}
error write_physical_block(disk_id id,block b,uint32_t num){


}
int main(int argc, char *argv[]){
	printf("%s\n",argv[0]);
	char cwd[1024];
	   if (getcwd(cwd, sizeof(cwd)) != NULL)
	       fprintf(stdout, "Current working dir: %s\n", cwd);
	   else
	       perror("getcwd() error");
	   return 0;
	disk_id a;
	block b;
	uint32_t c=10;
	error er=read_physical_block(a,b,c);
	printf("hello");

}
