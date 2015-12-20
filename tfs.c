#include "tfs.h"

//return the file number nbFile of the FileTab
file* getFile_Of_FileTab(partition p,uint32_t nbFile){
	uint32_t blockNumber;
	uint32_t positionInBlock;
	if(nbFile<TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK){
		blockNumber=0;
		positionInBlock=nbFile;
	}else{
		uint32_t blockNumber=nbFile% TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK;
		positionInBlock = nbFile - (blockNumber*TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK);
	}

	error er;

	block * b;
	b = initBlock();
	readBlockOfPartition(p,*b,blockNumber);
	testerror(er);

	file * file=malloc(sizeof(file));

	file->tfs_size=nombre32bitsToValue(b->valeur[positionInBlock]);
	file->tfs_type=nombre32bitsToValue(b->valeur[positionInBlock+1]);
	file->tfs_subtype=nombre32bitsToValue(b->valeur[positionInBlock+2]);

	//3 to 12
	int i;
	for(i=0;i<10;i++){
		file->tfs_direct[i]=nombre32bitsToValue(b->valeur[positionInBlock+3+i]);
	}

	file->tfs_indirect1=nombre32bitsToValue(b->valeur[positionInBlock+13]);
	file->tfs_indirect2=nombre32bitsToValue(b->valeur[positionInBlock+14]);
	file->tfs_next_free=nombre32bitsToValue(b->valeur[positionInBlock+15]);

	freeBlock(b);
	return file;
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
descriptionBlock* initDescriptionBlock(partition p) {

	descriptionBlock *dB = malloc(sizeof(descriptionBlock));
	return dB;

}

error putBlockInFreeBlockList(partition p,descriptionBlock * dB,uint32_t numberBlock) {

	error er;
	uint32_t old = dB->volumeFistFreeBlock;

	block * b;
	b = initBlock();
	if (old != 0) {

		// get the actual First free BLock of the list who is in the description
		er = readBlockOfPartition(p, *b, numberBlock);
		testerror(er);

		//put at the end of the new first block the old first free block
		b->valeur[TFS_VOLUME_BLOCK_SIZE - 1] = valueToNombre32bits(old);

	}
	else{
		//there is no other free Block in the list so the new free block tfs_next-free go on imself
		b->valeur[TFS_VOLUME_BLOCK_SIZE - 1] = valueToNombre32bits(numberBlock);
	}

	er = writeBlockOfPartition(p, *b, numberBlock);
	testerror(er);

	//put in the descriptionBlock the new first free block
	dB->volumeFistFreeBlock = numberBlock;
	testerror(er);

	freeBlock(b);
	er.val=0;
	er.message="block add at first with succes";

	return er;
}

error removeBlockOfFreeBlockListe(partition p,descriptionBlock * dB,uint32_t numberBlock){
	error er;

	uint32_t tmp = dB->volumeFistFreeBlock;


	if (tmp == 0) {
		er.val=1;
		er.message="list of free block is empty";
		return er;
	}
	if(numberBlock==tmp){
		//deletion of the first element of the list , the list is now empty
		dB->volumeFistFreeBlock=0;
		er.val=0;
		return er;

	}
	block * b;
	b = initBlock();
	er = readBlockOfPartition(p, *b, tmp);
	testerror(er);

	uint32_t tmp2=nombre32bitsToValue(b->valeur[TFS_VOLUME_BLOCK_SIZE - 1]);

	while(tmp2!=numberBlock){
		er = readBlockOfPartition(p, *b, tmp2);
		testerror(er);
		tmp=tmp2;

		tmp2=nombre32bitsToValue(b->valeur[TFS_VOLUME_BLOCK_SIZE - 1]);
		if(tmp==tmp2){
			er.message="block not found in the list of free block";
			er.val=1;
			return er;
		}
	}

	er = readBlockOfPartition(p, *b, tmp2);
	testerror(er);
	uint32_t tmp3=nombre32bitsToValue(b->valeur[TFS_VOLUME_BLOCK_SIZE - 1]);


	er = readBlockOfPartition(p, *b, tmp);
	testerror(er);


	// deletion of the last element of the list
	if(tmp3==tmp2){
		b->valeur[TFS_VOLUME_BLOCK_SIZE - 1] = valueToNombre32bits(tmp);
		writeBlockOfPartition(p,*b,tmp);
	}

	// the block n is at the middle of 2 blocks , n-1 is connect to n+1
	else{
		b->valeur[TFS_VOLUME_BLOCK_SIZE - 1] = valueToNombre32bits(tmp3);
		writeBlockOfPartition(p,*b,tmp);

	}
	freeBlock(b);
	er.val=0;
	er.message="block delete with succes";
	return er;
}

descriptionBlock* getDescriptionBlock(partition p) {

	descriptionBlock * dB = malloc(sizeof(descriptionBlock));
	block * b;
	b = initBlock();
	error er;
	er = readBlockOfPartition(p, *b, p.firstPositionInTFS);
	testerror(er);
	dB->magic = nombre32bitsToValue(b->valeur[0]);
	dB->volumeBlockSize = nombre32bitsToValue(b->valeur[1]);
	dB->volumeFreeBlockNb = nombre32bitsToValue(b->valeur[2]);
	dB->volumeFistFreeBlock = nombre32bitsToValue(b->valeur[3]);
	dB->volumeMaxFile = nombre32bitsToValue(b->valeur[4]);
	dB->volumeFreeFileNb = nombre32bitsToValue(b->valeur[5]);
	dB->volumeFirstFreeFile = nombre32bitsToValue(b->valeur[6]);

	free(b);
	return dB;
}

