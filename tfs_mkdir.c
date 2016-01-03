#include "tfs.h";

int main(int argc, char *argv[]) {

	error er;
	char * name;

	if (argc < 2) {
		er.val = 1;
		er.message = "error with arguments";
		testerror(er);
	}
	int i;
	mode_t mode;
	int modeSEE = 0;
	int modeVERBOSE = 0;
	int numberOfFolder;
	for (i = 2; i < argc; i++) {
		if (strcmp(argv[i], "-m") == 0) {
			if (modeSEE) {
				er.val = 1;
				er.message = "only 1 mode -m !!";
				testerror(er);
			}
			mode = argv[i + 1];
			modeSEE = 1;
		} else if (strcmp(argv[i], "-p") == 0) {
			if (modeVERBOSE) {
				er.val = 1;
				er.message = "only 1 mode -p !!";
				testerror(er);
			}

			modeVERBOSE= 1;
		}
		else{
			numberOfFolder++;

			//tfs_mkdir(argv[i],mode,modeVERBOSE);
		}

	}

}
