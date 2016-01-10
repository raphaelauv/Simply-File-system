#include "tfs.h"

int main(int argc, char *argv[]) {

	error er;
	int succes;

	if (argc < 2) {
		er.val = 1;
		er.message = "error with arguments";
		testerror(er);
	}

	int i;
	int modeRECURSIVE = 0;
	int modeVERBOSE = 0;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-r") == 0) {
			if (modeRECURSIVE) {
				er.val = 1;
				er.message = "only 1 mode -m !!";
				testerror(er);
			}
			modeRECURSIVE = 1;
		} else if (strcmp(argv[i], "-v") == 0) {
			if (modeVERBOSE) {
				er.val = 1;
				er.message = "only 1 mode -p !!";
				testerror(er);
			}

			modeVERBOSE = 1;
		} else {
			succes = tfs_rmdir(argv[i]);
			if (succes == 0) {
				//TODO put the printf in comment
				printf("succes !!\n");
			} else if (succes == -1) {
				return -1;
			}
		}

	}

	return 0;
}
