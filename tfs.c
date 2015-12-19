#include "tfs.h"

error readBlockOfPartition(partition p, block b, uint32_t numberBlock) {
	error er;

	uint32_t positionOnTFS = p.firstPositionInTFS + numberBlock;
	er = read_block(*(p.disque), b, positionOnTFS);

	return er;

}

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

error putBlockInFreeBlockList(partition p, descriptionBlock * dB,uint32_t numberBlock) {

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

