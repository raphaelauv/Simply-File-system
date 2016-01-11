#include "tfs.h"
/**
 * STDOUT -> succes and the path of folder create
 */
int main(int argc, char *argv[]) {

	error er;
	int succes;

	if (argc < 2) {
		er.val = 1;
		er.message = "error with arguments";
		testerror(er);
	}

	int i;
	mode_t mode = NULL;
	int modeSEE = 0;
	int modeVERBOSE = 0;
	int numberOfFolder = 0;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-m") == 0) {
			if (modeSEE) {
				er.val = 1;
				er.message = "only 1 mode -m !!";
				testerror(er);
			}
			//mode = argv[i + 1];
			modeSEE = 1;
		} else if (strcmp(argv[i], "-p") == 0) {
			if (modeVERBOSE) {
				er.val = 1;
				er.message = "only 1 mode -p !!";
				testerror(er);
			}

			modeVERBOSE = 1;
		} else {

			numberOfFolder++;
			succes = tfs_mkdir(argv[i], mode);
			if (succes == 0) {
				fprintf(stdout,"succes -> %s\n",argv[i]);
			} else if (succes == -1) {
				return -1;
			}
		}

	}

	return 0;
}
