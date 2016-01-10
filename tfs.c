#include "tfs.h"



error static readDescriptionBlock(partition p, descriptionBlock* dB);
error static writeDescriptionBlock(partition p,descriptionBlock* dB);


int static insertInBlock(block * b, uint32_t nbFileParent,char* nameEntry,int numberOfNewFile);
int static insertInFolder(partition p, file* fileParent,char* nameEntry,int numberOfNewFile);
error static addEntry(partition p,uint32_t parentFolder , char* nameEntry,int FLAG);
error static deleteEntry(partition p,uint32_t nbFileParent ,uint32_t nbFile, int FLAG_SECURE , int FOLDER_OPTION);
void static delete_File_Indirect2(partition p, uint32_t nbBlock, int FLAG, int FLAG_SECURE);
void static delete_File_Indirect1(partition p, uint32_t nbBlock,int FLAG, int FLAG_SECURE);
void static delete_File_Direct(partition p, uint32_t nbBlock, int FLAG, int FLAG_SECURE);
void static cleanBlock(partition p, uint32_t nbBlock , int FLAG,int FLAG_SECURE);


//determine the blocknumber of the partition where is located the nbfile file
uint32_t getblockNumber_Of_File(uint32_t nbFile) {

	if (nbFile < 0) {
		error er;
		er.val = 1;
		er.message = "in getblockNumber_Of_File the value of nbFile is <1 ";
		testerror(er);
	}

	if (nbFile < TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK + 1) {
		return 1; //1 because at 0 it is the description block
	} else {
		return (nbFile % TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK) + 1;
	}
}

//determine the position of the nbfile fil in the blocknumber of the partition
uint32_t positionInBlock_Of_File(uint32_t nbFile, uint32_t blockNumber) {
	if (blockNumber < 0) {
		error er;
		er.val = 1;
		er.message =
				"in positionInBlock_Of_File the value of blockNumber is negative or 0";
		testerror(er);
	}
	return nbFile * TTTFS_NUMBER_OF_INT_IN_KEY_OF_FILE_TABLE;
	/*else {
	 return nbFile*TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK
	 - ((blockNumber - 1) * TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK);
	 }*/

}

//return the file n of the FileTab
file* getFile_Of_FileTab(partition p, uint32_t nbFile) {

	file * file=NULL;
	file = initFile();

	uint32_t blockNumber = getblockNumber_Of_File(nbFile);
	uint32_t positionInBlock = positionInBlock_Of_File(nbFile, blockNumber);

	error er;
	block * b;
	b = initBlock();
	er = readBlockOfPartition(p, *b, blockNumber);
	testerror(er);

	file->tfs_size = nombre32bitsToValue(b->valeur[positionInBlock]);
	file->tfs_type = nombre32bitsToValue(b->valeur[positionInBlock + 1]);
	file->tfs_subtype = nombre32bitsToValue(b->valeur[positionInBlock + 2]);

	//3 to 12 include
	int i;
	for (i = 0; i < DIRECT_TAB; i++) {
		file->tfs_direct[i] = nombre32bitsToValue(
				b->valeur[positionInBlock + 3 + i]);
	}

	file->tfs_indirect1 = nombre32bitsToValue(
			b->valeur[positionInBlock + 3 + DIRECT_TAB]);
	file->tfs_indirect2 = nombre32bitsToValue(
			b->valeur[positionInBlock + 4 + DIRECT_TAB]);
	file->tfs_next_free = nombre32bitsToValue(
			b->valeur[positionInBlock + 5 + DIRECT_TAB]);

	file->nbFile=nbFile;
	freeBlock(b);

	return file;
}

void static printFile(file* file) {

	printf("-----FILE DESCRIPTION--\n");

	printf("FILE NUMBER : %d\n", file->nbFile);

	printf("tfs size : %d\n", file->tfs_size);
	printf("tfs type : %d\n", file->tfs_type);
	printf("tfs subtype : %d\n", file->tfs_subtype);
	printf("tfs direct : \n");
	int i;
	for (i = 0; i < DIRECT_TAB; i++) {
		printf("tfs direct[%d] : %d\n", i, file->tfs_direct[i]);
	}

	printf("tfs indirect1 : %d\n", file->tfs_indirect1);
	printf("tfs indirect2 : %d\n", file->tfs_indirect2);
	printf("tfs next_free : %d\n", file->tfs_next_free);
	printf("----------------------\n");

}

//write the info of *file of the nbFile in the partition p
error writeFile_Of_FileTab(partition p, file* file) {
	error er;

	uint32_t blockNumber = getblockNumber_Of_File(file->nbFile);
	uint32_t positionInBlock = positionInBlock_Of_File(file->nbFile, blockNumber);

	block * b;
	b = initBlock();
	er = readBlockOfPartition(p, *b, blockNumber);
	testerror(er);

	free(b->valeur[positionInBlock]);
	b->valeur[positionInBlock] = valueToNombre32bits(file->tfs_size);

	free(b->valeur[positionInBlock+1]);
	b->valeur[positionInBlock + 1] = valueToNombre32bits(file->tfs_type);

	free(b->valeur[positionInBlock+2]);
	b->valeur[positionInBlock + 2] = valueToNombre32bits(file->tfs_subtype);

	//3 to 12 include
	int i;
	for (i = 0; i < DIRECT_TAB; i++) {

		free(b->valeur[positionInBlock+ 3 + i]);
		b->valeur[positionInBlock + 3 + i] = valueToNombre32bits(
				file->tfs_direct[i]);
	}

	free(b->valeur[positionInBlock + 3 + DIRECT_TAB]);
	b->valeur[positionInBlock + 3 + DIRECT_TAB] = valueToNombre32bits(
			file->tfs_indirect1);

	free(b->valeur[positionInBlock + 4 + DIRECT_TAB]);
	b->valeur[positionInBlock + 4 + DIRECT_TAB] = valueToNombre32bits(
			file->tfs_indirect2);

	free(b->valeur[positionInBlock + 5 + DIRECT_TAB]);
	b->valeur[positionInBlock + 5 + DIRECT_TAB] = valueToNombre32bits(
			file->tfs_next_free);

	er = writeBlockOfPartition(p, *b, blockNumber);
	freeBlock(b);
	return er;
}

/********************************************************/
/**
 * DELETE RECURSIVLY FILES IN BLOCK IF IT IS A FOLDER
 * ADD FREE FILE TO FREE FILE LIST
 * ADD FREE BLOCK TO FREE BLOCK LIST
 * DELETE WITH SECURITY IF FLAG SECURE IS FLAG DELETE SECURE
 */
void static cleanBlock(partition p, uint32_t nbBlock, int FLAG_TYPE,
		int FLAG_SECURE) {

	error er;

	block * b;
	b = initBlock();

	if (FLAG_TYPE == FLAG_ENTRY_FOLDER) {
		er = readBlockOfPartition(p, *b, nbBlock);
		int i;
		int nbFile;
		int positioninblock;
		for (i = 0; i < TTTFS_VOLUME_BLOCK_SIZE;i = i + (SIZE_ENTRY_IN_FOLDER)) {
			positioninblock = i / TFS_VOLUME_DIVISION_OCTAL;
			//printf("dans clean val i : %d\n",i);

			if (i < SIZE_ENTRY_IN_FOLDER * 2) {

			} else {
				nbFile = nombre32bitsToValue(b->valeur[positioninblock]);
				if (nbFile != 0) {
					deleteEntry(p, nbBlock, nbFile, FLAG_SECURE,
							FLAG_DELETE_FOLDER);
					er = add_OF_FLAG_FreeListe(p, nbFile, FLAG_FILE);
					testerror(er);
				}
			}
		}
	}

	freeBlock(b);
	if (FLAG_SECURE == FLAG_DELETE_SECURE) {

		block * b2;
		b2 = initBlock();
		er = writeBlockOfPartition(p, *b2, nbBlock);
		testerror(er);
		freeBlock(b2);
	}
	er = add_OF_FLAG_FreeListe(p, nbBlock, FLAG_BLOCK);
	testerror(er);
}

void static delete_File_Direct(partition p, uint32_t nbBlock, int FLAG_TYPE, int FLAG_SECURE) {
	error er;
	cleanBlock(p, nbBlock, FLAG_TYPE,FLAG_SECURE);
	er = add_OF_FLAG_FreeListe(p, nbBlock, FLAG_BLOCK);
	testerror(er);
}

void static delete_File_Indirect1(partition p, uint32_t nbBlock,int FLAG_TYPE, int FLAG_SECURE) {
	error er;
	block *b;
	b = initBlock();
	er = readBlockOfPartition(p, *b, nbBlock);
	testerror(er);

	int i;
	uint32_t tmp;
	for (i = 0; i < TTTFS_VOLUME_BLOCK_SIZE ; i++) {
		tmp = nombre32bitsToValue(b->valeur[i]);
		if (tmp != 0) {
			delete_File_Direct(p, tmp, FLAG_TYPE , FLAG_SECURE);
		}
	}
	freeBlock(b);

}
void static delete_File_Indirect2(partition p, uint32_t nbBlock, int FLAG_TYPE, int FLAG_SECURE) {
	error er;
	block *b;
	b = initBlock();
	er = readBlockOfPartition(p, *b, nbBlock);
	testerror(er);

	int i;
	uint32_t tmp;
	for (i = 0; i < TTTFS_VOLUME_BLOCK_SIZE ; i++) {
		tmp = nombre32bitsToValue(b->valeur[i]);
		if (tmp != 0) {
			delete_File_Indirect1(p, tmp, FLAG_TYPE, FLAG_SECURE);
		}
	}

	freeBlock(b);
}

/**
 * Delete nbfile from the disk and FileTab and the link inside the parent folder
 */
error static deleteEntry(partition p,uint32_t nbFileParent ,uint32_t nbFile, int FLAG_SECURE , int FLAG_FOLDER_OPTION) {

	error er;

	int FLAG_TYPE;
	file * fileParent=NULL;
	file * fileFolder=NULL;

	fileParent = getFile_Of_FileTab(p, nbFileParent);
	fileFolder = getFile_Of_FileTab(p, nbFile);

	int succes = searchEntryInFolder(p, nbFileParent, nbFile, NULL,FLAG_SEARCH_DELETE);
	if (succes != -2) {
		er.val = -1;
		er.message = "error delete";
		return er;
	}
	//file->tfs_size = 0;
	if(fileFolder->tfs_type == 0){
		FLAG_TYPE=FLAG_ENTRY_FILE;
	}
	else if(fileFolder->tfs_type == 1 && FLAG_FOLDER_OPTION==FLAG_DELETE_FOLDER){
		FLAG_TYPE=FLAG_ENTRY_FOLDER;
	}

	else{
		er.val=1;
		er.message="Impossible to delete a folder without option FOLDER";
		testerror(er);
	}
	//file->tfs_subtype = 0;

	int i;
	for (i = 0; i < DIRECT_TAB; i++) {
		if (fileFolder->tfs_direct[i] != 0) {

			delete_File_Direct(p, fileFolder->tfs_direct[i],FLAG_TYPE, FLAG_DELETE_SECURE);
		}
		//file->tfs_direct[i] = 0;
	}
	if (fileFolder->tfs_indirect1 != 0) {
		delete_File_Indirect1(p, fileFolder->tfs_indirect1,FLAG_TYPE, FLAG_DELETE_SECURE);
		//file->tfs_indirect1 = 0;
	}

	if (fileFolder->tfs_indirect2 != 0) {
		delete_File_Indirect2(p, fileFolder->tfs_indirect2,FLAG_TYPE, FLAG_DELETE_SECURE);
		//file->tfs_indirect2 = 0;
	}

	//file->tfs_next_free = 0;

	er=add_OF_FLAG_FreeListe(p,nbFile,FLAG_FILE);// add the new free file entry in the free file list
	testerror(er);

	fileParent->tfs_size=(fileParent->tfs_size)-(SIZE_ENTRY_IN_FOLDER);
	er=writeFile_Of_FileTab(p,fileParent);

	free(fileFolder);
	free(fileParent);
	er.val = 0;
	er.message = "Delete SUCCES of file";
	return er;

}
//read the numberBlock block of the p partition
error readBlockOfPartition(partition p, block b, uint32_t numberBlock) {
	error er;

	uint32_t positionOnTFS = (p.firstPositionInTFS) + numberBlock;
	//printf("number block a lire : %d\n",positionOnTFS);

	er = read_block(*(p.disque), b, positionOnTFS);

	return er;

}

//write the numberBlock block of the p partition
error writeBlockOfPartition(partition p, block b, uint32_t numberBlock) {
	error er;

	uint32_t positionOnTFS = p.firstPositionInTFS + numberBlock;
	er = write_block(*(p.disque), b, positionOnTFS);

	return er;

}
descriptionBlock* initDescriptionBlock() {

	descriptionBlock * dB = NULL;
	dB= malloc(sizeof (descriptionBlock ));
	if(dB==NULL){
		error er;
		er.val=1;
		er.message="ERROR MALLOC initDescriptionBlock";
		testerror(er);
	}
	return dB;

}

file* initFile() {

	file * f = NULL;
	f = malloc(sizeof(file));
	if (f == NULL) {
			error er;
			er.val=1;
			er.message="error malloc file in INITFILE ";
			testerror(er);
		}
	f->nbFile=0;
	int i;
	for (i = 0; i < DIRECT_TAB; i++) {
		f->tfs_direct[i] = 0;
	}
	f->tfs_indirect1 = 0;
	f->tfs_indirect2 = 0;
	f->tfs_size = 0;
	f->tfs_subtype = 0;
	f->tfs_type = 0;
	f->tfs_next_free = 0;
	return f;

}

error add_OF_FLAG_FreeListe(partition p, uint32_t numberOfValueToAdd, int FLAG) {

	error er;

	descriptionBlock * dB;
	dB = initDescriptionBlock();
	er = readDescriptionBlock(p, dB);

	uint32_t old;

	block * b;
	file * file;

	if (FLAG == FLAG_BLOCK) {
		b = initBlock();
		old = dB->volumeFirstFreeBlock;
		// get the new first block
		er = readBlockOfPartition(p, *b, numberOfValueToAdd);
		testerror(er);

	} else if (FLAG == FLAG_FILE) {
		old = dB->volumeFirstFreeFile;
	} else {
		er.val = 1;
		er.message = "BAD FLAG USE IN putBlockInFreeBlockList";
		return er;
	}

	if (old != 0) {
		if (FLAG == FLAG_BLOCK) {
			//put at the end of the new first block the old first free block

			free(b->valeur[TFS_VOLUME_NUMBER_VALUE_BY_BLOCK - 1]);
			b->valeur[TFS_VOLUME_NUMBER_VALUE_BY_BLOCK - 1] =
					valueToNombre32bits(old);

		} else if (FLAG == FLAG_FILE) {

			file = getFile_Of_FileTab(p, numberOfValueToAdd);

			file->tfs_next_free = old;

		}

	} else {
		//there is no free Block or free file in the list so the new free block/file tfs_next-free go on imself
		if (FLAG == FLAG_BLOCK) {

			free(b->valeur[TFS_VOLUME_NUMBER_VALUE_BY_BLOCK - 1]);

			b->valeur[TFS_VOLUME_NUMBER_VALUE_BY_BLOCK - 1] =
					valueToNombre32bits(numberOfValueToAdd);
		} else if (FLAG == FLAG_FILE) {
			file = initFile();
			file->tfs_next_free = numberOfValueToAdd;
		}
	}

	/********************************************************/
	/**
	 * write the new block or new file of the filetab
	 */
	if (FLAG == FLAG_BLOCK) {

		er = writeBlockOfPartition(p, *b, numberOfValueToAdd);
		testerror(er);

		//put in the descriptionBlock the new first free block
		dB->volumeFirstFreeBlock = numberOfValueToAdd;
		dB->volumeFreeBlockNb = (dB->volumeFreeBlockNb) + 1;

		freeBlock(b);

	} else if (FLAG == FLAG_FILE) {

		er = writeFile_Of_FileTab(p,file);
		testerror(er);

		//put in the descriptionBlock the new first free file
		dB->volumeFirstFreeFile = numberOfValueToAdd;
		dB->volumeFreeFileNb = (dB->volumeFreeFileNb) + 1;

		free(file);

	}
	/********************************************************/
	/**
	 * //write the new descriptionBlock
	 */

	er = writeDescriptionBlock(p, dB);
	testerror(er);
	free(dB);
	er.val = 0;
	er.message = "new block or file add at first with succes";
	return er;
}


/**
 * Return the number of the removed Block or File in free List
 * return -1 if free list is empty
 */
uint32_t remove_OF_FLAG_FreeListe(partition p, int FLAG) {
	error er;

	descriptionBlock * dB;
	dB = initDescriptionBlock();
	er = readDescriptionBlock(p, dB);
	testerror(er);

	uint32_t tmp;
	uint32_t tmp2;

	file * file;
	block * b;
	if (FLAG == FLAG_BLOCK) {
		if (dB->volumeFreeBlockNb < 1) {
			return -1;
		}

		tmp = dB->volumeFirstFreeBlock;
		b = initBlock();
		er = readBlockOfPartition(p, *b, tmp);
		testerror(er);
		tmp2 = nombre32bitsToValue(
				b->valeur[TFS_VOLUME_NUMBER_VALUE_BY_BLOCK - 1]);

		//b = initBlock();

	} else if (FLAG == FLAG_FILE) {

		if (dB->volumeFreeFileNb < 1) {
			return -1;
		}
		tmp = dB->volumeFirstFreeFile;

		//file = initFile();
		file = getFile_Of_FileTab(p, tmp);
		testerror(er);
		tmp2 = file->tfs_next_free;
	}

	else {
		er.val = 1;
		er.message = "BAD FLAG USE IN putBlockInFreeBlockList";
		testerror(er);
	}
	if (FLAG == FLAG_BLOCK) {

		dB->volumeFirstFreeBlock = tmp2;
		dB->volumeFreeBlockNb = (dB->volumeFreeBlockNb) - 1;

		freeBlock(b);

	} else if (FLAG == FLAG_FILE) {

		dB->volumeFirstFreeFile = tmp2;
		dB->volumeFreeFileNb = (dB->volumeFreeFileNb) - 1;
		free(file);
	}


	er = writeDescriptionBlock(p, dB);
	testerror(er);
	free(dB);

	return tmp;
}

error static readDescriptionBlock(partition p, descriptionBlock* dB) {

	error er;
	if (dB == NULL) {
		er.val = 1;
		er.message = "argument of readDescriptionBlock is NULL";
		return er;
	}

	block * b;
	b = initBlock();

	er = readBlockOfPartition(p, *b, 0);
	if (er.val != 0) {
		return er;
	}
	dB->magic = nombre32bitsToValue(b->valeur[0]);
	dB->volumeBlockSize = nombre32bitsToValue(b->valeur[1]);
	dB->volumeBlockCount = nombre32bitsToValue(b->valeur[2]);
	dB->volumeFreeBlockNb = nombre32bitsToValue(b->valeur[3]);
	dB->volumeFirstFreeBlock = nombre32bitsToValue(b->valeur[4]);
	dB->volumeMaxFile = nombre32bitsToValue(b->valeur[5]);
	dB->volumeFreeFileNb = nombre32bitsToValue(b->valeur[6]);
	dB->volumeFirstFreeFile = nombre32bitsToValue(b->valeur[7]);

	freeBlock(b);
	er.val = 0;
	er.message = "readDescriptionBlock SUCCES";
	return er;
}

error static writeDescriptionBlock(partition p, descriptionBlock* dB) {
	error er;

	block * b;
	b = initBlock();
	int i;
	for(i=0 ; i<8 ; i++){
		free(b->valeur[i]);
	}

	b->valeur[0] = valueToNombre32bits(dB->magic);
	b->valeur[1] = valueToNombre32bits(dB->volumeBlockSize);
	b->valeur[2] = valueToNombre32bits(dB->volumeBlockCount);
	b->valeur[3] = valueToNombre32bits(dB->volumeFreeBlockNb);
	b->valeur[4] = valueToNombre32bits(dB->volumeFirstFreeBlock);
	b->valeur[5] = valueToNombre32bits(dB->volumeMaxFile);
	b->valeur[6] = valueToNombre32bits(dB->volumeFreeFileNb);
	b->valeur[7] = valueToNombre32bits(dB->volumeFirstFreeFile);

	er = writeBlockOfPartition(p, *b, 0);

	freeBlock(b);

	return er;
}

/**
 * return 1 if file is a folder
 */
int static isFolder(file* file) {
	if (file == NULL) {
		return -1;
	}
	if (file->tfs_type == 1) {
		return 1;
	} else {
		return 0;
	}
}

/**
 * return the number of the new file in  File Table of
 *
 * the new empty folder or the new empty file
 *
 * in all case the new entry have a Block in the directBlock
 *
 * return -1 if no more free file or free block available
 */
int createEmptyEntry(partition p, uint32_t parentFolder,int FLAG) {


	error er;
	uint32_t numberOfFile = remove_OF_FLAG_FreeListe(p, FLAG_FILE);
	if(numberOfFile==-1){
		return -1;
	}


	uint32_t numberOfBlock = remove_OF_FLAG_FreeListe(p, FLAG_BLOCK);

	file * file=NULL;

	file = getFile_Of_FileTab(p, numberOfFile);
	if(numberOfBlock==-1){
		//no more free block !!
		er=add_OF_FLAG_FreeListe(p,numberOfFile,FLAG_FILE);
		testerror(er);
		free(file);
		return -1;
	}
	//printf("numberblock : %d\n",numberOfBlock);
	//printf("numberfile : %d\n",numberOfFile);

	if (FLAG == FLAG_ENTRY_FOLDER) {
		block * b;
		b = initBlock();

		free(b->valeur[0]);
		b->valeur[0] = valueToNombre32bits(numberOfFile);
		//TODO add ascii '.'

		free(b->valeur[1]);
		b->valeur[1] = fourCharToNombre32bits(ASCII_FOR_POINT, 0, 0, 0);

		free(b->valeur[8]);
		b->valeur[8] = valueToNombre32bits(parentFolder);

		free(b->valeur[9]);
		b->valeur[9] = fourCharToNombre32bits(ASCII_FOR_POINT, ASCII_FOR_POINT,0, 0);

		er = writeBlockOfPartition(p, *b, numberOfBlock);
		testerror(er);

		file->tfs_size = (SIZE_ENTRY_IN_FOLDER)*2;
		file->tfs_type = 1;

		freeBlock(b);
	}
	else if(FLAG==FLAG_ENTRY_FILE){
		file->tfs_size = 0;
		file->tfs_type = 0;
	}

	else{
		er.val=1;
		er.message="error in the FLAG of createEmptyEntry ";
		testerror(er);
	}
	file->tfs_subtype = 0;
	file->tfs_direct[0] = numberOfBlock;
	file->tfs_next_free = 0;

	er = writeFile_Of_FileTab(p, file);
	testerror(er);

	free(file);
	return numberOfFile;
}

/**
 * insert in the first empty entrance of the block the new Entry
 * return -1 if no more place
 */
int static insertInBlock(block * b, uint32_t nbFileParent,char* nameEntry,int numberOfNewFile) {

	int succes=-1;
	if (b == NULL) {
		error er;
		er.val=1;
		er.message="error with arg block in insertFolderInBlock";
		testerror(er);
	}

	int i;
	int j;
	int positioninblock;
	for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; i=i+SIZE_ENTRY_IN_FOLDER) {

		positioninblock=i/TFS_VOLUME_DIVISION_OCTAL;// because block size  256 and the loop for limit is 1024

		if (nombre32bitsToValue(b->valeur[positioninblock]) == 0) {
			//printf("valeur de l'entree : %d\n",nombre32bitsToValue(b->valeur[positioninblock]));

			if(i<(SIZE_ENTRY_IN_FOLDER)*2){
				//DO NOT TOUCH THE . and .. of the root
				//we could start i at 64
			}
			else{
				//put the value of file in file Tab
				free(b->valeur[positioninblock]);

				//printf("valeur de position ou ecrire : %d",i);
				b->valeur[positioninblock]=valueToNombre32bits(numberOfNewFile);


				//Put the name of file
				int compteur=0; // to forwad of 1 every 4 of the for
				int sizeNameEntry=testNameEntry(nameEntry);


				/********************************************************/
				/**
				 *  WRITE THE NAME OF FOLDER
				 */
				for (j = 0; j < sizeNameEntry; j=j+TFS_VOLUME_DIVISION_OCTAL) {

					if(j+1>sizeNameEntry){
						b->valeur[positioninblock+1+compteur] = fourCharToNombre32bits(nameEntry[j],0,0,0);
					}else if(j+2>sizeNameEntry){
						b->valeur[positioninblock+1+compteur] = fourCharToNombre32bits(nameEntry[j],nameEntry[j+1],0,0);
					}else if(j+3>sizeNameEntry){
						b->valeur[positioninblock+1+compteur] = fourCharToNombre32bits(nameEntry[j],nameEntry[j+1],nameEntry[j+2],0);
					}else{
						b->valeur[positioninblock+1+compteur] = fourCharToNombre32bits(nameEntry[j],nameEntry[j+1],nameEntry[j+2],nameEntry[j+3]);
					}
					compteur++;
				}

				succes=1;
				break;

			}
		}
	}
	return succes;
}

/**
 *Try to insert a new entry in the FOLDER , start by indirect and after indirect1 ...
 *return 0 if succes
 *return -1 if problem
 */
int static insertInFolder(partition p, file* fileParent,char* nameEntry,int numberOfNewFile) {

	/********************************************************/
	/**
	 * DIRECT
	 */

	error er;

	block* b;
	b = initBlock();

	int i;
	for (i = 0; i < DIRECT_TAB; i++) {

		//we need a new block to add the new entry
		if (fileParent->tfs_direct[i] == 0) {
			uint32_t numberOfBlock = remove_OF_FLAG_FreeListe(p, FLAG_BLOCK);
			if (numberOfBlock == -1) {
				return -1;
			}
			fileParent->tfs_direct[i]=numberOfBlock;
			er=writeFile_Of_FileTab(p,fileParent);
			testerror(er);
		}

		er = readBlockOfPartition(p, *b, fileParent->tfs_direct[i]);
		testerror(er);
		//printf("avant insert block :numero block : %d\n",i);

		int result = insertInBlock(b, fileParent->nbFile, nameEntry,numberOfNewFile);
		if (result!=-1) {

			er = writeBlockOfPartition(p, *b, fileParent->tfs_direct[i]);
			testerror(er);
			//the entry have been had in the folder , we can leave
			return 0;
		}
	}
	/********************************************************/
	/**
	 * INDIRECT1
	 */

	block* b1;
	b1 = initBlock();
	er = readBlockOfPartition(p, *b1, fileParent->tfs_indirect1);
	testerror(er);

	int j;
	for (j = 0; j < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; j++) {
		if (nombre32bitsToValue(b1->valeur[j]) == 0) {
			return j + DIRECT_TAB;
		}
	}

	freeBlock(b1);

	/********************************************************/
	/**
	 * INDIRECT2
	 */

	block* b2;
	b2 = initBlock();
	er = readBlockOfPartition(p, *b2, fileParent->tfs_indirect2);

	int h;
	int k;
	for (h = 0; h < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; h++) {

		// inside the block who countains other blocks
		if (nombre32bitsToValue(b2->valeur[h]) == 0) {
			uint32_t numberOfBlock = remove_OF_FLAG_FreeListe(p, FLAG_BLOCK);

			block * b2;
			b2 = initBlock();

			testerror(er);

			free(b2->valeur[h]);
			b2->valeur[h] = valueToNombre32bits(numberOfBlock);

			freeBlock(b2);
			//TODO
			return DIRECT_MAX_SIZE + h;
		}
		for (k = 0; k < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; k++) {

			return j + DIRECT_TAB;
		}

		freeBlock(b2);
	}

	return -1;
}

/**
 * Add nameEntry file or folder ( depends of FLAG )  in parentFolder
 */
error static addEntry(partition p,uint32_t parentFolder , char* nameEntry,int FLAG) {
	error er;
	if(searchEntryInFolder(p,parentFolder,-1,nameEntry,FLAG_SEARCH_ONLY)!=-1){
		er.val=1;
		er.message="The folder or file already exist !!";
		return er;
	}

	file* fileParent=NULL;
	fileParent=getFile_Of_FileTab(p,parentFolder);

	if (!isFolder(fileParent)) {

		er.val = 1;
		er.message = "the entrance number %d is not a folder so you cant add something inside",fileParent->nbFile;
		return er;
	}

	//printf("sizeNewEntry : %d\n",sizeNameEntry);

	int numberOfNewEntry;

	if(FLAG==FLAG_ENTRY_FOLDER){

		numberOfNewEntry= createEmptyEntry(p,parentFolder,FLAG_ENTRY_FOLDER);
	}else if(FLAG==FLAG_ENTRY_FILE){
		numberOfNewEntry= createEmptyEntry(p,parentFolder,FLAG_ENTRY_FILE);
	}else{
		er.val=1;
		er.message="error in the FLAG of createEmptyEntry ";
		testerror(er);
	}

	//printf("valeur numberOfNewEntry : %d \n",numberOfNewEntry);

	if(numberOfNewEntry==-1){
		er.val=1;
		er.message="NO MORE FREE PLACE IN THE DISK FOR A NEW FOLDER";
		return er;
	}


	int result;
	result=insertInFolder(p,fileParent,nameEntry,numberOfNewEntry);
	if(result==1){
		er.val=1;
		er.message="CONTAINS LIMIT OF A FOLDER , you cannot add anithing to the folder %d",fileParent->nbFile;
		return er;
	}
	fileParent->tfs_size=(fileParent->tfs_size)+(SIZE_ENTRY_IN_FOLDER);
	er=writeFile_Of_FileTab(p,fileParent);
	testerror(er);
	free(fileParent);
	er.val=0;
	er.message="FOLDER CREATE";
	return er;
}

/**
 * return the nbFile of nameEntry if is in the block | FLAG SEARCH
 * return -2 if delete  with succes | FLAG DELETE
 * return -1 if nbFile is not inside this block
 */
uint32_t static searchEntryInBlock(block * b,  uint32_t nbBlock,uint32_t nbFile,char* nameEntry , int FLAG_SEARCH) {

	uint32_t nbFileOfNameEntry;
	int i;
	int j;
	int positionInBlock;

	char nameInside[SIZE_MAX_NAME_ENTRY]="";
	char *nameInsidePoint = nameInside;

	int compteur;
	int bollenFind=0;
	for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK;i = i + (SIZE_ENTRY_IN_FOLDER)) {
		positionInBlock = i /TFS_VOLUME_DIVISION_OCTAL;// because block size  256 and the for limit is 1024


		nbFileOfNameEntry=nombre32bitsToValue(b->valeur[positionInBlock]);

		if (nbFileOfNameEntry==0){

		}
		else if (nbFile==-1) {
			//only by nameEntry
			compteur=0;// to forwad of 1 every 4 of the for
			for (j = 0; j < SIZE_MAX_NAME_ENTRY; j = j + TFS_VOLUME_DIVISION_OCTAL) {

				//printf("val tmpPoint[0] : %c \n",tmpPoint[0]);
				nameInside[j]=b->valeur[positionInBlock + 1 + compteur]->val[0];
				nameInside[j+1]=b->valeur[positionInBlock + 1 + compteur]->val[1];
				nameInside[j+2]=b->valeur[positionInBlock + 1 + compteur]->val[2];
				nameInside[j+3]=b->valeur[positionInBlock + 1 + compteur]->val[3];

				compteur++;
			}
			if(strcmp(nameEntry, nameInsidePoint) == 0){
				bollenFind=1;
			}
		}
		//TODO optimise if ...

		if (nbFileOfNameEntry == nbFile || bollenFind) {
			//we found it
			if (FLAG_SEARCH == FLAG_SEARCH_ONLY) {
				return nbFileOfNameEntry;
			} else if (FLAG_SEARCH == FLAG_SEARCH_DELETE) {

				int i;
				for (i = 0; i < 8; i++) {
					free(b->valeur[positionInBlock + i]);
					b->valeur[positionInBlock + i] = valueToNombre32bits(0);
				}
				return -2;
			}
		}
	}
	return -1;
}
/**
 *
 * SEARCH IN parentFolder IF THE nbFIle OR THE nameEntry EXIST
 * PRIORITY TO nbFile
 * SEARCH nameEntry if nbFile is -1
 *
 * 	return the number File in filetab of the folder (name) | FLAG_SEARCH_SIMPLE
 * 	return -2 if delete <ith succes  | FLAG_SEARCH_DELETE
 *	return -1 if do not exist
 */
uint32_t static searchEntryInFolder(partition p, uint32_t parentFolder,uint32_t nbFile ,char *nameEntry, int FLAG_SEARCH) {

	file * fileParent=NULL;
	fileParent=getFile_Of_FileTab(p,parentFolder);

	uint32_t nbBlock;
	/**
	 * DIRECT
	 */
	error er;
	block* b;
	b = initBlock();
	int result;

	int i;
	for (i = 0; i < DIRECT_TAB; i++) {
		if (fileParent->tfs_direct[i] != 0) {
			nbBlock=fileParent->tfs_direct[i];
			er = readBlockOfPartition(p, *b, nbBlock);
			testerror(er);

			result = searchEntryInBlock(b,nbBlock,nbFile ,nameEntry,FLAG_SEARCH);
			if (result == -2) {
				//in mode delete
				//write the deletion of searchEntryInBlock
				er=writeBlockOfPartition(p,*b,nbBlock);
				testerror(er);
			}
			if(result!=-1){
				return result;
			}
		}
	}
	return -1;
}

/**
 * Init disk and partition thank to tabOfPath
 */
int static initDisk_Partition(disk_id *disk, partition *p, char **tabOfPath, int *size) {

	error er;

	er = start_disk(tabOfPath[0],disk);
	testerror(er);

	//1 step
	tabOfPath = iterateInPath(tabOfPath,size);

	p->disque = disk;

	int valPartition;

	valPartition = strtol(tabOfPath[0], NULL, 10);
	//1 step
	tabOfPath = iterateInPath(tabOfPath, size);
	int positionFirstBlock = firstblockPositionOfPartition(valPartition, *disk);

	if (positionFirstBlock == -1) {
		perror("The partiton selected do not exist");
		return -1;
	}
	p->firstPositionInTFS = positionFirstBlock;

	return 0;
}

/**
 * Create a folder thank to the path
 * return 0 if succes
 * -1 if error
 */
int tfs_mkdir(const char *path, mode_t mode){
	return tfs_RM_MK_DIR(path,mode,FLAG_MKDIR);
}

/**
 * DELETE an entry thank to the path
 * return 0 if succes
 * -1 if error
 */
int tfs_rmdir(const char *path) {
	return tfs_RM_MK_DIR(path, 0, FLAG_RMDIR);
}

/**
 * Do RM of MKDIR of path
 * FLAG tell which one to do
 * return -1 if fail
 * return 0 if succes
 */
int tfs_RM_MK_DIR(const char *path, mode_t mode , int FLAG){

	error er;
	int testResult=0;
	partition *p=malloc(sizeof(partition));
	disk_id *disk = malloc(sizeof(*disk));
	int val=0;
	int * size=&val;

	char **tabOfPath;
	char* tab[MAX_SIZE_OF_FOLDER_IN_PATH];  // Tableau d'arguments.
	tabOfPath = tab;
	/********************************************************/
	testStartOfPath(path);
	char copyPath[MAX_SIZE_PATH];
	copyConstPathWithoutStart(path, copyPath);
	*size = tokenize_Path(copyPath, tabOfPath);

	// HOST
	if (strcmp(tabOfPath[0], "HOST") == 0) {
		perror("for HOST use the unix command");
		return -1;
	}
	/********************************************************/
	er = start_disk(tabOfPath[0], disk);
	testerror(er);
	tabOfPath = iterateInPath(tabOfPath, size);//1 step

	p->disque = disk;
	int valPartition;
	valPartition = strtol(tabOfPath[0], NULL, 10);
	tabOfPath = iterateInPath(tabOfPath, size);//1 step
	int positionFirstBlock = firstblockPositionOfPartition(valPartition, *disk);
	if (positionFirstBlock == -1) {
		perror("The partiton selected do not exist");
		return -1;
	}
	p->firstPositionInTFS = positionFirstBlock;

/*
	testResult=initPath_Size(path,size,tabOfPath);
	if(testResult==-1){
			return -1;
		}
	testResult=initDisk_Partition(disk,p,tabOfPath,size);
	if(testResult==-1){
		return -1;
	}

*/
    /********************************************************/
    /**
     *  Search for the Parent folder in Disk
     *  where we want to add or delete the Folder
     */

    //printTokenPath(size,tabOfPath);

    uint32_t nbFileParent=0;//the number of the folder who gone a contain the new folder or where is the entry
    // start at 0 -> the ROOT
    while(*size>1){
    	nbFileParent=searchEntryInFolder(*p,nbFileParent,-1,tabOfPath[0],FLAG_SEARCH_ONLY);
    	if(nbFileParent==-1){
    		perror("ERROR IN THE PATH , OR ONE OF THE FOLDER IN THE PATH DO NOT EXIST");
    		return -1;
    	}
    	//1 step
    	tabOfPath=iterateInPath(tabOfPath,size);
    }

	/********************************************************/
	/**
	 *  CREATE the folder OR DELETE the entry
	 */
    if(FLAG==FLAG_MKDIR){
    	er=addEntry(*p,nbFileParent,tabOfPath[0],FLAG_ENTRY_FOLDER);
    }else if(FLAG==FLAG_RMDIR){

    	uint32_t nbFile=searchEntryInFolder(*p,nbFileParent,-1,tabOfPath[0],FLAG_SEARCH_ONLY);
    	if (nbFile== -1) {
    		perror("ERROR IN THE PATH , OR ONE OF THE FOLDER IN THE PATH DO NOT EXIST");
			return -1;
    	}
    	er=deleteEntry(*p,nbFileParent,nbFile,FLAG_DELETE_SIMPLE,FLAG_DELETE_FOLDER);
    }else if(FLAG==FLAG_RENAME){

    }
    testerror(er);


	freeDisk(disk);
	free(p);
	return 0;
}

/**
 *	Rename old with new
 *	return 0 if succes
 *	return -1 if fail
 */
int tfs_rename(const char *old, const char *new){

}

/**
 * try to open a file in TTFS
 * return the number in file descriptor
 * return -1 if fail
 *
 */
int tfs_open(const char *name, int oflag, ...) {

	MY_FILE* fp;
	for (fp = tabOfFiles + 3; fp < tabOfFiles + MY_FOPEN_MAX; ++fp) {
	        if (fp->flags == 0)
	            break;
	  }

}


int tfs_close(int fildes){

	MY_FILE* fp=&(tabOfFiles[fildes]);
	//flushbuffer(MY_EOF,fp);
	freeDisk(fp->p.disque);


}
