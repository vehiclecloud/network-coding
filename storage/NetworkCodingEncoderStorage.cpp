/* Copyright (c) 2014 SRI International and GPC
 * Developed under DARPA contract N66001-11-C-4022.
 * Authors:
 *   Joshua Joy (JJ, jjoy)
 *   Mark-Oliver Stehr (MOS)
 *   Hasnain Lakhani (HL)
 */

#include "NetworkCodingEncoderStorage.h"
#include "Trace.h"

NetworkCodingEncoderStorage::NetworkCodingEncoderStorage() {
	this->encoderStorage = new encoder_t();
	this->dataobjectstorage = new dataobjectstorage_t();
	this->networkCodedBlockSendEventTime = new networkCodedBlockSendEventTime_t();
}

NetworkCodingEncoderStorage::~NetworkCodingEncoderStorage() {

}

codetorrentencoderref NetworkCodingEncoderStorage::getEncoder(string parentDataObjectId,string filePath,size_t blockSize) {
    codetorrentencoderref& encoderref = this->encoderStorage->operator [](parentDataObjectId);
    codetorrentencoder* encoder = encoderref.getObj();
    if (encoder == NULL) {
        printf("Creating new network coding encoder for file %s parentDataObjectId %s\n", filePath.c_str(),parentDataObjectId.c_str());

	// MOS - this code is now here to better deal with errors
	FILE *filePointer = fopen(filePath.c_str(), "rb");
	if(filePointer) printf("Opening file %s with file descriptor %d\n", filePath.c_str(), fileno(filePointer));
	if(!filePointer) return NULL; // MOS
	fseek(filePointer, 0, SEEK_END);
	long fileSize = ftell(filePointer);
	fclose(filePointer);
	//printf("fileSize=%d\n",fileSize);
	if(fileSize < 0) {
	  printf("Unable to determine size of file %s\n", filePath.c_str());
	  return NULL;
	}
        encoder = new codetorrentencoder(filePath.c_str(),blockSize,fileSize);
        this->encoderStorage->operator [](parentDataObjectId) = encoder;
    }
    return encoderref;
}

void NetworkCodingEncoderStorage::addDataObject(string originalDataObjectId,
        const DataObjectRef originalDataObject) {
    this->dataobjectstorage->operator [](originalDataObjectId) = originalDataObject;
}

const DataObjectRef NetworkCodingEncoderStorage::getDataObject(
        string originalDataObjectId) {
    const DataObjectRef found = this->dataobjectstorage->operator [](originalDataObjectId);
    return found;
}

const DataObjectRef NetworkCodingEncoderStorage::getDataObjectById(string id) {
    //printf("Looking for data object %s\n", id.c_str());
    const DataObjectRef found = this->getDataObject(id);
    if (found != NULL) {
        printf("Found data object %s\n", found->getIdStr());
    }
    else {
        printf("Unable to locate data object %s\n", id.c_str());
    }
    return found;
}

void NetworkCodingEncoderStorage::deleteFromStorageByDataObjectId(string parentDataObjectId) {
    printf("Removing data object %s from networkcoding encoder storage\n",
                parentDataObjectId.c_str());
    this->encoderStorage->erase(parentDataObjectId);

    printf("Removing data object %s from networkcoding encoder storage\n",
                parentDataObjectId.c_str());
    this->dataobjectstorage->erase(parentDataObjectId);
}

// CBMEN, HL, Begin
void NetworkCodingEncoderStorage::deleteTrackingInfoByDataObjectId(string parentDataObjectId) {
    printf("Removing block send for data object %s time from tracker\n",parentDataObjectId.c_str());
    this->networkCodedBlockSendEventTime->erase(parentDataObjectId);
}
// CBMEN, HL, End

void NetworkCodingEncoderStorage::trackSendEvent(string parentDataObjectId) {
    Timeval timeNow = Timeval::now();
    printf("Tracking parent data object %s - time=%s\n",
            parentDataObjectId.c_str(), timeNow.getAsString().c_str());
    this->networkCodedBlockSendEventTime->operator [](parentDataObjectId) = timeNow.getAsString();
}

void NetworkCodingEncoderStorage::ageOffBlocks(double maxAgeBlock) {
    Timeval timeNow = Timeval::now();
    List<string> deleteAfterIterating;
    size_t encoderStorageSize = this->encoderStorage->size();
    size_t dataobjectstorageSize = this->dataobjectstorage->size();

    printf("encoderStorageSize=%d dataobjectstorageSize=%d\n",encoderStorageSize,dataobjectstorageSize);

    for (Map<string,string>::iterator it = this->networkCodedBlockSendEventTime->begin();
            it!=this->networkCodedBlockSendEventTime->end();it++) {
        string parentDataObjectId = it->first;
        string timeSendEvent = it->second;
        Timeval timevalTimeSendEvent(timeSendEvent);
        if( timevalTimeSendEvent + maxAgeBlock < timeNow) {
            printf("Expiration reached - timesendevent=%s timenow=%s\n",timevalTimeSendEvent.getAsString().c_str(),timeNow.getAsString().c_str());
            this->deleteFromStorageByDataObjectId(parentDataObjectId);
            deleteAfterIterating.push_front(parentDataObjectId);
        }
    }

    for(List<string>::iterator it = deleteAfterIterating.begin();it!=deleteAfterIterating.end();it++) {
    	string parentDataObjectId = (*it);
    	printf("Removing block send for data object %s time from tracker\n",parentDataObjectId.c_str());
    	this->networkCodedBlockSendEventTime->erase(parentDataObjectId);
    }


}
