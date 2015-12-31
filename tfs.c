#include "tfs.h"

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

	file * file;
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

void printFile(file* file) {

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

	b->valeur[positionInBlock] = valueToNombre32bits(file->tfs_size);
	b->valeur[positionInBlock + 1] = valueToNombre32bits(file->tfs_type);
	b->valeur[positionInBlock + 2] = valueToNombre32bits(file->tfs_subtype);

	//3 to 12 include
	int i;
	for (i = 0; i < DIRECT_TAB; i++) {
		b->valeur[positionInBlock + 3 + i] = valueToNombre32bits(
				file->tfs_direct[i]);
	}

	b->valeur[positionInBlock + 3 + DIRECT_TAB] = valueToNombre32bits(
			file->tfs_indirect1);
	b->valeur[positionInBlock + 4 + DIRECT_TAB] = valueToNombre32bits(
			file->tfs_indirect2);
	b->valeur[positionInBlock + 5 + DIRECT_TAB] = valueToNombre32bits(
			file->tfs_next_free);

	er = writeBlockOfPartition(p, *b, blockNumber);

	return er;
}

/********************************************************/
/**
 * DELETE RECURSIVLY FILES IN BLOCK IF IT IS A FOLDER
 * ADD FREE FILE TO FREE FILE LIST
 * ADD FREE BLOCK TO FREE BLOCK LIST
 * DELETE WITH SECURITY IF FLAG SECURE IS FLAG DELETE SECURE
 */
error cleanBlock(partition p, uint32_t nbBlock , int FLAG,int FLAG_SECURE) {

	error er;

	block * b;
	b = initBlock();

	if(FLAG==FLAG_ENTRY_FOLDER){
		er=readBlockOfPartition(p,*b,nbBlock);
		int i;
		for(i=0;i<TFS_VOLUME_BLOCK_SIZE;i=i+SIZE_ENTRY_IN_FOLDER){

			if(b->valeur[i]!=0){
				deleteEntry(p,b->valeur[i],FLAG_SECURE,DELETE_FOLDER);
				er=add_OF_FLAG_FreeListe(p,b->valeur[i],FLAG_FILE);
				testerror(er);
			}
		}
	}

	if(FLAG_SECURE==FLAG_DELETE_SECURE){

		er = writeBlockOfPartition(p, *b, nbBlock);
		testerror(er);
	}
	er=add_OF_FLAG_FreeListe(p,nbBlock,FLAG_BLOCK);
	testerror(er);
	freeBlock(b);
}

void delete_File_Direct(partition p, uint32_t nbBlock, int FLAG, int FLAG_SECURE) {
	error er;

	er = cleanBlock(p, nbBlock, FLAG,FLAG_SECURE);
	testerror(er);
	er = add_OF_FLAG_FreeListe(p, nbBlock, FLAG_BLOCK);
	testerror(er);
}

void delete_File_Indirect1(partition p, uint32_t nbBlock,int FLAG, int FLAG_SECURE) {
	error er;
	block *b;
	b = initBlock();
	er = readBlockOfPartition(p, *b, nbBlock);
	testerror(er);

	int i;
	// -1 at TFS_VOLUME_BLOCK_SIZE because the last emplacement of a block is next_free
	uint32_t tmp;
	for (i = 0; i < TFS_VOLUME_BLOCK_SIZE - 1; i++) {
		tmp = nombre32bitsToValue(b->valeur[i]);
		if (tmp != 0) {
			delete_File_Direct(p, tmp, FLAG , FLAG_SECURE);
		}
	}
	freeBlock(b);

}
void delete_File_Indirect2(partition p, uint32_t nbBlock, int FLAG, int FLAG_SECURE) {
	error er;
	block *b;
	b = initBlock();
	er = readBlockOfPartition(p, *b, nbBlock);
	testerror(er);

	int i;
	// -1 at TFS_VOLUME_BLOCK_SIZE because the last emplacement of a block is next_free
	uint32_t tmp;
	for (i = 0; i < TFS_VOLUME_BLOCK_SIZE - 1; i++) {
		tmp = nombre32bitsToValue(b->valeur[i]);
		if (tmp != 0) {
			delete_File_Indirect1(p, tmp, FLAG, FLAG_SECURE);
		}
	}

	freeBlock(b);
}

error deleteEntry(partition p, uint32_t nbFile, int FLAG_SECURE , int FOLDER_OPTION) {

	int FLAG;
	file * file;
	error er;
	file = getFile_Of_FileTab(p, nbFile);
	testerror(er);
	//file->tfs_size = 0;
	if(file->tfs_type == 0){
		FLAG=FLAG_ENTRY_FILE;
	}
	else if(file->tfs_type == 1 && FOLDER_OPTION==DELETE_FOLDER){
		FLAG=FLAG_ENTRY_FOLDER;
	}

	else{
		er.val=1;
		er.message="Impossible to delete a folder without option FOLDER";
	}
	if( (FLAG==FLAG_ENTRY_FILE && file->tfs_type!=0)
			|| (FLAG ==FLAG_ENTRY_FOLDER && file->tfs_type!=1)){

		er.val=1;
		er.message="error in FLAG selected in deleteEntry";
		return er;
	}
	//file->tfs_subtype = 0;

	int i;
	for (i = 0; i < DIRECT_TAB; i++) {
		if (file->tfs_direct[i] != 0) {
			delete_File_Direct(p, file->tfs_direct[i],FLAG, FLAG_SECURE);
		}
		//file->tfs_direct[i] = 0;
	}
	if (file->tfs_indirect1 != 0) {
		delete_File_Indirect1(p, file->tfs_indirect1,FLAG, FLAG_SECURE);
		//file->tfs_indirect1 = 0;
	}

	if (file->tfs_indirect2 != 0) {
		delete_File_Indirect2(p, file->tfs_indirect2,FLAG, FLAG_SECURE);
		//file->tfs_indirect2 = 0;
	}

	//file->tfs_next_free = 0;

	er=add_OF_FLAG_FreeListe(p,nbFile,FLAG_FILE);// add the new free file entry in the free file list
	testerror(er);
	free(file);
	er.val = 0;
	er.message = "Delete SUCCES of file";
	return er;

}

//read the numberBlock block of the p partition
error readBlockOfPartition(partition p, block b, uint32_t numberBlock) {
	error er;

	uint32_t positionOnTFS = p.firstPositionInTFS + numberBlock;
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

	descriptionBlock *dB = malloc(sizeof(descriptionBlock));
	return dB;

}

file* initFile() {

	file * file = malloc(sizeof(*file));
	if (file == NULL) {
	//TODO
			error er;
			er.val=1;
			er.message="error malloc file in INITFILE ";
			testerror(er);
		}
	file->nbFile=0;
	int i;
	for (i = 0; i < 10; i++) {
		file->tfs_direct[i] = 0;
	}
	file->tfs_indirect1 = 0;
	file->tfs_indirect2 = 0;
	file->tfs_size = 0;
	file->tfs_subtype = 0;
	file->tfs_type = 0;
	file->tfs_next_free = 0;


	return file;

}

error add_OF_FLAG_FreeListe(partition p, uint32_t numberOfValueToAdd, int FLAG) {

	error er;

	descriptionBlock * dB;
	dB = initDescriptionBlock();
	er = getDescriptionBlock(p, dB);

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
		file = initFile();
		old = dB->volumeFirstFreeFile;
	} else {
		er.val = 1;
		er.message = "BAD FLAG USE IN putBlockInFreeBlockList";
		return er;
	}

	if (old != 0) {
		if (FLAG == FLAG_BLOCK) {
			//put at the end of the new first block the old first free block
			b->valeur[TFS_VOLUME_NUMBER_VALUE_BY_BLOCK - 1] =
					valueToNombre32bits(old);

		} else if (FLAG == FLAG_FILE) {

			file = getFile_Of_FileTab(p, numberOfValueToAdd);

			file->tfs_next_free = old;

		}

	} else {
		//there is no free Block or free file in the list so the new free block/file tfs_next-free go on imself
		if (FLAG == FLAG_BLOCK) {

			b->valeur[TFS_VOLUME_NUMBER_VALUE_BY_BLOCK - 1] =
					valueToNombre32bits(numberOfValueToAdd);
		} else if (FLAG == FLAG_FILE) {

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

uint32_t remove_OF_FLAG_FreeListe(partition p, int FLAG) {
	error er;

	descriptionBlock * dB;
	dB = initDescriptionBlock();
	er = getDescriptionBlock(p, dB);
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

		b = initBlock();

	} else if (FLAG == FLAG_FILE) {

		if (dB->volumeFreeFileNb < 1) {
			return -1;
		}
		tmp = dB->volumeFirstFreeFile;

		file = initFile();
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

error getDescriptionBlock(partition p, descriptionBlock* dB) {

	error er;
	if (dB == NULL) {
		er.val = 1;
		er.message = "argument of getDescriptionBlock is NULL";
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
	er.message = "getDescriptionBlock SUCCES";
	return er;
}

error writeDescriptionBlock(partition p, descriptionBlock* dB) {
	error er;

	block * b;
	b = initBlock();

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
int isFolder(file* file) {
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
 * return the number of the new file in  File Table for the new empty folder , or new empty file
 */
int createEmptyEntry(partition p, uint32_t parentFolder,int FLAG) {

	error er;
	testerror(er);
	uint32_t numberOfFile = remove_OF_FLAG_FreeListe(p, FLAG_FILE);

	if(numberOfFile==-1){

		return -1;

	}

	file * file;
	file = initFile();
	file = getFile_Of_FileTab(p, numberOfFile);

	uint32_t numberOfBlock = remove_OF_FLAG_FreeListe(p, FLAG_BLOCK);

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

		b->valeur[0] = valueToNombre32bits(numberOfFile);
		//TODO add ascii '.'
		b->valeur[1] = fourCharToNombre32bits(ASCII_FOR_POINT, 0, 0, 0);

		b->valeur[8] = valueToNombre32bits(parentFolder);

		b->valeur[9] = fourCharToNombre32bits(ASCII_FOR_POINT, ASCII_FOR_POINT,0, 0);

		er = writeBlockOfPartition(p, *b, numberOfBlock);
		testerror(er);

		file->tfs_size = 64;
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
 * insert in the first empty entrance the new Entry in block
 */
int insertInBlock(block * b, uint32_t nbFile,char* nameEntry,int sizeNameEntry,int numberOfNewFile) {

	int succes=0;
	if (b == NULL) {
		error er;
		er.val=1;
		er.message="error with arg block in insertFolderInBlock";
		testerror(er);
	}

	int i;
	int j;
	for (i = 0; i < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; i=i+SIZE_ENTRY_IN_FOLDER) {
		if (b->valeur[i] == 0) {

			if(nbFile==0 && i<64){
				//DO NOT ERASE THE ROOT
			}
			else{
				b->valeur[i]=valueToNombre32bits(numberOfNewFile);
				for (j = 0; j < sizeNameEntry; j=j+4) {
					b->valeur[i+1+j] = fourCharToNombre32bits(nameEntry[j],nameEntry[j+1],nameEntry[j+2],nameEntry[j+3]);
					//TODO
				}
				break;
				succes=1;
			}
		}
	}
	return succes;
}

/**
 *
 */
int insertInFolder(partition p, file* file,char* nameEntry,int sizeNameEntry,int numberOfNewFile) {

	/**
	 * DIRECT
	 */

	error er;


	block* b;
	b = initBlock();

	int i;
	for (i = 0; i < 10; i++) {
		if (file->tfs_direct[i] == 0) {

		}else{
			er = readBlockOfPartition(p, *b, file->tfs_indirect1);
			testerror(er);
			int result=insertInBlock(b,file->nbFile,nameEntry,sizeNameEntry,numberOfNewFile);
			if(result){
				return result;
			}
		}
	}
	/**
	 * INDIRECT1
	 */

	block* b1;
	b1 = initBlock();
	er = readBlockOfPartition(p, *b1, file->tfs_indirect1);
	testerror(er);

	int j;
	for (j = 0; j < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; j++) {
		if (b1->valeur[j] == 0) {
			return j + DIRECT_TAB;
		}
	}

	freeBlock(b1);

	/**
	 * INDIRECT2
	 */

	block* b2;
	b2 = initBlock();
	er = readBlockOfPartition(p, *b2, file->tfs_indirect2);

	int h;
	int k;
	for (h = 0; h < TFS_VOLUME_NUMBER_VALUE_BY_BLOCK; h++) {

		// inside the block who countains other blocks
		if (b2->valeur[h] == 0) {
			uint32_t numberOfBlock = remove_OF_FLAG_FreeListe(p, FLAG_BLOCK);

			block * b2;
			b2 = initBlock();

			testerror(er);

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
}

error addEntry(partition p, file* file, char* nameEntry,int FLAG) {

	error er;
	int i;
	int ASCII_END_FOUND = 0;
	int sizeNameEntry=0;
	for (i = 0; i < SIZE_MAX_NAME_ENTRY; i++) {
		if (nameEntry[i] == 0) {
			sizeNameEntry=i;
			ASCII_END_FOUND = 1;
			break;
		}
	}
	if(sizeNameEntry<1){
		er.val = 1;
		er.message = "the name of the folder is smaller than 1 characters :\n";
		return er;
	}

	if (!ASCII_END_FOUND) {
		er.val = 1;
		er.message = "the name of the folder is bigger than 27 characters :\n %s",nameEntry;
		return er;
	}
	if (!isFolder(file)) {

		er.val = 1;
		er.message = "the file %d is not a folder so you cant add something inside",file->nbFile;
		return er;
	}

	int numberOfNewFile;

	if(FLAG==FLAG_ENTRY_FOLDER){
		numberOfNewFile= createEmptyEntry(p, file->nbFile,FLAG_ENTRY_FOLDER);
	}else if(FLAG==FLAG_ENTRY_FILE){
		numberOfNewFile= createEmptyEntry(p, file->nbFile,FLAG_ENTRY_FILE);
	}else{
		er.val=1;
		er.message="error in the FLAG of createEmptyEntry ";
		testerror(er);
	}

	if(numberOfNewFile==-1){
		er.val=1;
		er.message="NO MORE FREE PLACE IN THE DISK FOR A NEW FOLDER";
		return er;
	}
	int result;
	result=insertInFolder(p,file,nameEntry,sizeNameEntry,numberOfNewFile);
	if(result==1){
		er.val=1;
		er.message="CONTAINS LIMIT OF A FOLDER , you cannot add anithing to the folder %d",file->nbFile;
		return er;
	}

	er.val=0;
	er.message="FOLDER CREATE";
	return er;
}
