#include "tfs.h"

uint32_t getblockNumber_Of_File(uint32_t nbFile){

	if(nbFile<1){
			error er;
			er.val=1;
			er.message="in getblockNumber_Of_File the value of nbFile is <1 ";
			testerror(er);
		}

	if(nbFile<TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK+1){
			return 1;//1 because at 0 it is the description block
		}else{
			return (nbFile% TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK)+1;
		}
}

uint32_t positionInBlock_Of_File(uint32_t nbFile ,uint32_t blockNumber){
	if(blockNumber<1){
		error er;
		er.val=1;
		er.message="in positionInBlock_Of_File the value of blockNumber is negative or 0";
		testerror(er);
	}
	if(blockNumber==1){
		return nbFile;
	}
	else{
		return nbFile - ((blockNumber-1)*TTTFS_NUMBER_OF_FILE_IN_ONE_BLOCK);
	}

}

//write the info of *file of the nbFile in the partition p
error writeFile_Of_FileTab(partition p,uint32_t nbFile,file* file){
	error er;

	uint32_t blockNumber=getblockNumber_Of_File(nbFile);
	uint32_t positionInBlock=positionInBlock_Of_File (nbFile , blockNumber);

	block * b;
	b = initBlock();
	er=readBlockOfPartition(p,*b,blockNumber);
	testerror(er);

	b->valeur[positionInBlock]=valueToNombre32bits(file->tfs_size);
	b->valeur[positionInBlock+1]=valueToNombre32bits(file->tfs_type);
	b->valeur[positionInBlock+2]=valueToNombre32bits(file->tfs_subtype);

	//3 to 12 include
	int i;
	for(i=0;i<10;i++){
		b->valeur[positionInBlock+3+i]=valueToNombre32bits(file->tfs_direct[i]);
	}

	b->valeur[positionInBlock+13]=valueToNombre32bits(file->tfs_indirect1);
	b->valeur[positionInBlock+14]=valueToNombre32bits(file->tfs_indirect2);
	b->valeur[positionInBlock+15]=valueToNombre32bits(file->tfs_next_free);

	er=writeBlockOfPartition(p,*b,blockNumber);

	return er;
}


//return the file n of the FileTab
error getFile_Of_FileTab(partition p,uint32_t nbFile,file* file){
	uint32_t blockNumber=getblockNumber_Of_File(nbFile);
	uint32_t positionInBlock=positionInBlock_Of_File (nbFile , blockNumber);

	error er;
	block * b;
	b = initBlock();
	er=readBlockOfPartition(p,*b,blockNumber);
	if(er.val!=0){
		return er;
	}

	file->tfs_size=nombre32bitsToValue(b->valeur[positionInBlock]);
	file->tfs_type=nombre32bitsToValue(b->valeur[positionInBlock+1]);
	file->tfs_subtype=nombre32bitsToValue(b->valeur[positionInBlock+2]);

	//3 to 12 include
	int i;
	for(i=0;i<10;i++){
		file->tfs_direct[i]=nombre32bitsToValue(b->valeur[positionInBlock+3+i]);
	}

	file->tfs_indirect1=nombre32bitsToValue(b->valeur[positionInBlock+13]);
	file->tfs_indirect2=nombre32bitsToValue(b->valeur[positionInBlock+14]);
	file->tfs_next_free=nombre32bitsToValue(b->valeur[positionInBlock+15]);

	freeBlock(b);
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

	file * file=malloc(sizeof(file));

	if(file==NULL){
//TODO
	}

	return file;

}

error putThingInFreeList(partition p, descriptionBlock * dB,uint32_t numberOfValueToAdd, int FLAG) {

	error er;

	uint32_t old;

	block * b;
	file * file;
	if (FLAG == FLAG_BLOCK) {

		old = dB->volumeFirstFreeBlock;

		b = initBlock();
		// get the new first block
		er = readBlockOfPartition(p, *b, numberOfValueToAdd);
		testerror(er);

	} else if (FLAG == FLAG_FILE) {

		file=initFile();
		old = dB->volumeFirstFreeFile;
	} else {
		er.val = 1;
		er.message = "BAD FLAG USU IN putBlockInFreeBlockList";
		return er;
	}


	if (old != 0) {
		if (FLAG == FLAG_BLOCK) {
			//put at the end of the new first block the old first free block
			b->valeur[TFS_VOLUME_BLOCK_SIZE - 1] = valueToNombre32bits(old);

		} else if (FLAG == FLAG_FILE) {


			getFile_Of_FileTab(p,numberOfValueToAdd,file);

			file->tfs_next_free=old;

		}

	} else {
		//there is no free Block or free file in the list so the new free block/file tfs_next-free go on imself
		if (FLAG == FLAG_BLOCK) {

			b->valeur[TFS_VOLUME_BLOCK_SIZE - 1] = valueToNombre32bits(numberOfValueToAdd);
		} else if (FLAG == FLAG_FILE) {

			file->tfs_next_free=numberOfValueToAdd;
		}
	}

	//write the new block or new file of the filetab
	if (FLAG == FLAG_BLOCK) {

		er = writeBlockOfPartition(p, *b, numberOfValueToAdd);
		testerror(er);

		//put in the descriptionBlock the new first free block
		dB->volumeFirstFreeBlock = numberOfValueToAdd;


	} else if (FLAG == FLAG_FILE) {

		er=writeFile_Of_FileTab(p,numberOfValueToAdd,file);
		testerror(er);

		dB->volumeFirstFreeFile=numberOfValueToAdd;
	}

	//write the new descriptionBlock
	er=writeDescriptionBlock(p,dB);
	testerror(er);

	freeBlock(b);
	free(file);
	er.val = 0;
	er.message = "block add at first with succes";

	return er;
}


//TODO
error removeThingOfFreeListe(partition p,descriptionBlock * dB,uint32_t numberOfValueToAdd , int FLAG){
	error er;

	uint32_t tmp = dB->volumeFirstFreeBlock;


	if (tmp == 0) {
		er.val=1;
		er.message="list of free block is empty";
		return er;
	}
	if(numberOfValueToAdd==tmp){
		//deletion of the first element of the list , the list is now empty
		dB->volumeFirstFreeBlock=0;
		er.val=0;
		return er;

	}
	block * b;
	b = initBlock();
	er = readBlockOfPartition(p, *b, tmp);
	testerror(er);

	uint32_t tmp2=nombre32bitsToValue(b->valeur[TFS_VOLUME_BLOCK_SIZE - 1]);

	while(tmp2!=numberOfValueToAdd){
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

error getDescriptionBlock(partition p, descriptionBlock* dB) {

	block * b;
	b = initBlock();
	error er;
	er = readBlockOfPartition(p, *b,0);
	if(er.val!=0){
		return er;
	}
	dB->magic = nombre32bitsToValue(b->valeur[0]);
	dB->volumeBlockSize = nombre32bitsToValue(b->valeur[1]);
	dB->volumeFreeBlockNb = nombre32bitsToValue(b->valeur[2]);
	dB->volumeFirstFreeBlock = nombre32bitsToValue(b->valeur[3]);
	dB->volumeMaxFile = nombre32bitsToValue(b->valeur[4]);
	dB->volumeFreeFileNb = nombre32bitsToValue(b->valeur[5]);
	dB->volumeFirstFreeFile = nombre32bitsToValue(b->valeur[6]);

	freeBlock(b);
	er.val=0;
	er.message="getDescriptionBlock SUCCES";
	return er;
}

error writeDescriptionBlock(partition p,descriptionBlock* dB){
	error er ;

	block * b;
	b = initBlock();

	b->valeur[0]= valueToNombre32bits(dB->magic);
	b->valeur[1]= valueToNombre32bits(dB->volumeBlockSize);
	b->valeur[2]= valueToNombre32bits(dB->volumeFreeBlockNb);
	b->valeur[3]= valueToNombre32bits(dB->volumeFirstFreeBlock);
	b->valeur[4]= valueToNombre32bits(dB->volumeMaxFile);
	b->valeur[5]= valueToNombre32bits(dB->volumeFreeFileNb);
	b->valeur[6]= valueToNombre32bits(dB->volumeFirstFreeFile);


	er=writeBlockOfPartition(p,*b,0);

	freeBlock(b);

	return er;
}
