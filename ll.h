#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


#define TFS_VOLUME_BLOCK_SIZE 1024

typedef struct {
    int val;
    char* message;
} error;

typedef struct {
	FILE * fichier;
	int flags;
} disk_id;

typedef struct {
	uint32_t firstPositionInTFS;
	disk_id *disque;
} partition;


typedef struct{
	unsigned char val[4];
}nombre32bits;


typedef struct {
	nombre32bits* valeur[TFS_VOLUME_BLOCK_SIZE/4];
} block;



error start_disk(char *name,disk_id *id); //qui permet de manipuler un disque en lui associant une identit� dynamique;
error read_block(disk_id id,block b,uint32_t num); //qui permet de lire un bloc sur le disque (lire annexe sur la raison de diff�rencier cette fonction et la fonction read_physical_block);
error write_block(disk_id id,block b,uint32_t num); //qui permet d��crire un bloc sur le disque (m�me remarque que la fonction pr�c�dente);
error sync_disk(disk_id id); //(voir annexe - en premi�re approximation cette fonction peut ne rien faire du tout;
error stop_disk(disk_id id); //qui permet de terminer une session de travail sur un disque.

block* initBlock();
void freeBlock(block* block);
void freeDisk(disk_id*disk);
nombre32bits* valueToNombre32bits (uint32_t n);
uint32_t nombre32bitsToValue(nombre32bits *bytes);
void printNombre32bits (nombre32bits *bytes);
void testerror(error er);
int* getInfo(disk_id disk);
int firstblockPositionOfPartition(int nbPartition, disk_id disk);

void printBlock(block * block);
int getSizePartition(int n,disk_id disk);
