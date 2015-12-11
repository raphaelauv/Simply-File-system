#include <sys/types.h>

#define MAXLIN 512           // Longueur max. d'une ligne.
#define MAXVEC (MAXLIN / 2)  // Nombre max. d'arguments.
#define MAXJOB 64            // Nombre max. de jobs.
#define ARGSEP " \t\n"       // Séparateurs d'arguments.
#define PROMPT "DisqueDur$ "      // Invite de commande.

extern int status;  // Code retour de la dernière commande.

/* _____________________________________________________ */
// Fonctions principales.  (main.c)
// ---
void display_prompt();
void read_command(char* argl);
int tokenize_command(char* argl, char** argv);
int execute_command(int argc, char** argv);

/* _____________________________________________________ */
// Fonctions exécutant des commandes.
// ---
int execute_command_exit(int argc, char** argv);
int execute_command_tfs_create(int argc, char** argv);
// ---

