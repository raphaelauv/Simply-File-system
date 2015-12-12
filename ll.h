#include <stdio.h>
#include <stdint.h>
#include <string.h>
typedef struct {
    int size;
    char* data2;
} error;


typedef struct {
    char* name;//emplacement
} disk_id;

typedef struct {
    int number;
} block;

error start_disk(char *name,disk_id *id); //qui permet de manipuler un disque en lui associant une identit� dynamique;
error read_block(disk_id id,block b,uint32_t num); //qui permet de lire un bloc sur le disque (lire annexe sur la raison de diff�rencier cette fonction et la fonction read_physical_block);
error write_block(disk_id id,block b,uint32_t num); //qui permet d��crire un bloc sur le disque (m�me remarque que la fonction pr�c�dente);
error sync_disk(disk_id id); //(voir annexe - en premi�re approximation cette fonction peut ne rien faire du tout;
error stop_disk(disk_id id); //qui permet de terminer une session de travail sur un disque.

