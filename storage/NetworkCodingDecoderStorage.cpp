/* Copyright (c) 2014 SRI International and GPC
 * Developed under DARPA contract N66001-11-C-4022.
 * Authors:
 *   Joshua Joy (JJ, jjoy)
 *   Mark-Oliver Stehr (MOS)
 *   Hasnain Lakhani (HL)
 */

#include "NetworkCodingDecoderStorage.h"

NetworkCodingDecoderStorage::NetworkCodingDecoderStorage(HaggleKernel* _haggleKernel,DataStore* _dataStore,
        NetworkCodingDataObjectUtility* _networkCodingDataObjectUtility,NetworkCodingConfiguration* _networkCodingConfiguration) {
	this->haggleKernel = _haggleKernel;
    this->dataStore = _dataStore;
    this->networkCodingDataObjectUtility = _networkCodingDataObjectUtility;
    this->networkCodingConfiguration = _networkCodingConfiguration;

    this->decoderStorage = new decoder_t();
    this->networkCodedBlocks = new networkCodedBlocks_t();
    this->networkCodedDecoderLastUsedTime = new networkCodedDecoderLastUsedTime_t();

}

NetworkCodingDecoderStorage::~NetworkCodingDecoderStorage() {

    /*
    for (decoder_t::iterator it = this->decoderStorage.begin();
            it != this->decoderStorage.end(); it++) {
        codetorrentdecoder* decoder = (*it).second.getObj();
    }
    */
    this->decoderStorage->clear();

}

codetorrentdecoderref NetworkCodingDecoderStorage::getDecoder(string originalDataObjectId,size_t _originalDataFileSize,string _decodedFilePath) {
    codetorrentdecoderref decoderref = this->decoderStorage->operator [](originalDataObjectId);
    if(!decoderref) {
        printf("Creating network coding decoder\n");
        decoderref = this->createAndAddDecoder(
                originalDataObjectId, _originalDataFileSize,
                _decodedFilePath.c_str(),
                this->networkCodingConfiguration->getBlockSize());

        // CBMEN, HL - Just do this the first time
        //raise event to convert any fragmentation blocks to network coding blocks
        int objectidlen = originalDataObjectId.size();
        char* dataobjectid = new char[objectidlen+1];
        memset(dataobjectid,0,objectidlen+1);
        strncpy(dataobjectid,originalDataObjectId.c_str(),objectidlen);
        Event* convertFragmentationToNetworkCodingEvent = new Event(EVENT_TYPE_DATAOBJECT_CONVERT_FRAGMENTATION_NETWORKCODING, dataobjectid, 0.01);
        this->haggleKernel->addEvent(convertFragmentationToNetworkCodingEvent);
    }
    this->trackDecoderLastUsedTime(originalDataObjectId);

    return decoderref;
}

// CBMEN, HL, Begin
bool NetworkCodingDecoderStorage::haveDecoder(string dataObjectId) {
    return ((this->decoderStorage->operator [](dataObjectId)) != NULL);
}
// CBMEN, HL, End

codetorrentdecoderref& NetworkCodingDecoderStorage::createAndAddDecoder(
        const string originalDataObjectId, const size_t originalDataFileSize,
        const char* decodedFilePath, size_t blockSize) {
    codetorrentdecoder* decoder = new codetorrentdecoder(originalDataFileSize,
            decodedFilePath, blockSize);
    this->decoderStorage->operator [](originalDataObjectId) = decoder;
    this->trackDecoderLastUsedTime(originalDataObjectId);
    return this->decoderStorage->operator [](originalDataObjectId);
}

bool NetworkCodingDecoderStorage::isDecodingCompleted(
        string originalDataObjectId) {
    return this->decodingCompleted[originalDataObjectId];
}

void NetworkCodingDecoderStorage::setDecodingCompleted(
        string originalDataObjectId) {
    this->decodingCompleted[originalDataObjectId] = true;
}

codedblocksidlistref NetworkCodingDecoderStorage::getNetworkCodedBlocks(
        string originalDataObjectId) {
    if (!this->networkCodedBlocks->operator [](originalDataObjectId)) {
        List<string>* ncblockList = new List<string>();
        this->networkCodedBlocks->operator [](originalDataObjectId) = ncblockList;
    }
    printf("Retrieving blocks for data object %s\n", originalDataObjectId.c_str());
    return this->networkCodedBlocks->operator [](originalDataObjectId);
}

void NetworkCodingDecoderStorage::addCodedBlockId(string originalDataObjectId,
        const string networkCodedDataObjectId) {
    codedblocksidlistref networkCodedBlocksList = this->getNetworkCodedBlocks(
            originalDataObjectId);
    size_t beforeSize = this->networkCodedBlocks->operator [](originalDataObjectId)->size();
    networkCodedBlocksList->push_front(networkCodedDataObjectId);
    this->networkCodedBlocks->operator [](originalDataObjectId) = networkCodedBlocksList;
    size_t afterSize = this->networkCodedBlocks->operator [](originalDataObjectId)->size();
    printf("Added block for parent data object %s - beforesize=%d aftersize=%d\n",
		originalDataObjectId.c_str(), beforeSize, afterSize);
}

void NetworkCodingDecoderStorage::trackNetworkCodedBlock(
        const string originalDataObjectId,
        const string networkCodedDataObjectId) {
    printf("Tracking block %s for parent data object %s\n",
		networkCodedDataObjectId.c_str(), originalDataObjectId.c_str());

    this->addCodedBlockId(originalDataObjectId, networkCodedDataObjectId);
}

void NetworkCodingDecoderStorage::disposeNetworkCodedBlocks(
        const string originalDataObjectId) {
    printf("Disposing blocks of parent data object %s\n",
            originalDataObjectId.c_str());

    codedblocksidlistref networkCodedBlocksList = this->getNetworkCodedBlocks(
            originalDataObjectId);

    size_t networkCodedBlocksSize = this->networkCodedBlocks->size();
    size_t reconstructedNetworkCodingDataObjectTrackerSize = this->reconstructedNetworkCodingDataObjectTracker.size();
    size_t decodingCompletedSize = this->decodingCompleted.size();
    printf("networkCodedBlocksSize=%d reconstructedNetworkCodingDataObjectTrackerSize=%d decodingCompletedSize=%d\n",
    		networkCodedBlocksSize,reconstructedNetworkCodingDataObjectTrackerSize,decodingCompletedSize);

    for (List<string>::iterator it = networkCodedBlocksList->begin();
            it != networkCodedBlocksList->end(); it++) {
        string networkCodedDataObjectId = (*it);
        DataObjectIdRef networkedCodedBlockId =
                this->networkCodingDataObjectUtility->convertDataObjectIdStringToDataObjectIdType(
                        networkCodedDataObjectId);
        this->disposeDataObject(networkedCodedBlockId.c_str());
    }

    printf("Deleting network coded blocks from tracker\n");
    this->networkCodedBlocks->erase(originalDataObjectId);
}

void NetworkCodingDecoderStorage::disposeDataObject(
        const unsigned char* dataObjectId) {
    string dataObjectIdHex = this->networkCodingDataObjectUtility->convertDataObjectIdToHex(dataObjectId);
    printf("Requesting to delete data object %s\n",
            dataObjectIdHex.c_str());
    this->dataStore->deleteDataObject(dataObjectId, false);
}

void NetworkCodingDecoderStorage::disposeNetworkCodedBlock(
        DataObjectRef networkCodedBlock) {
    printf("Disposing block %s\n", networkCodedBlock->getIdStr());
    string networkCodedDataObjectId = networkCodedBlock->getIdStr();
    DataObjectIdRef networkedCodedBlockId =
            this->networkCodingDataObjectUtility->convertDataObjectIdStringToDataObjectIdType(
                    networkCodedDataObjectId);
    const unsigned char* networkCodedBlockIdPtr = networkedCodedBlockId.c_str();
    string dataObjectIdHex = this->networkCodingDataObjectUtility->convertDataObjectIdToHex(networkCodedBlockIdPtr);
    //printf("Disposing block %s\n", dataObjectIdHex.c_str());
    this->disposeDataObject(networkCodedBlockIdPtr);
}

void NetworkCodingDecoderStorage::trackReconstructedDataObject(const string dataObjectId) {
    Timeval t = Timeval::now();
    string timeNow = t.getAsString();
    printf("Tracking reconstructed data object %s - timeReconstructed=%s\n", dataObjectId.c_str(), timeNow.c_str());
    reconstructedNetworkCodingDataObjectTracker[dataObjectId] = timeNow;
}

double NetworkCodingDecoderStorage::getResendReconstructedDelay(const string dataObjectId,
        double doubleResendReconstructedDelay) {
    Timeval timeValTimeNow = Timeval::now();
    Timeval timeValResendReconstructedDelay(doubleResendReconstructedDelay);

    string stringReconstructedTime = reconstructedNetworkCodingDataObjectTracker[dataObjectId];
    printf("Computing resend delay for data object %s - timeReconstructed=%s\n",
            dataObjectId.c_str(), stringReconstructedTime.c_str());
    Timeval timeReconstructed(timeValTimeNow);
    if (stringReconstructedTime.size() > 0 && stringReconstructedTime.c_str() != NULL) {
        Timeval valToSet(stringReconstructedTime);
        timeReconstructed = valToSet;

        timeReconstructed += timeValResendReconstructedDelay;
        printf("timeReconstructed+delay=%s\n", timeReconstructed.getAsString().c_str());
    } else {
        printf("timeReconstructed=%s\n", timeReconstructed.getAsString().c_str());
    }

    double calculatedResendDelay = 0.0;

    if (timeReconstructed <= timeValTimeNow) {
        calculatedResendDelay = -100.0;
    } else {
        printf("Using timeReconstructed=%s timeValTimeNow=%s\n",
                timeReconstructed.getAsString().c_str(), timeValTimeNow.getAsString().c_str());
        calculatedResendDelay = (timeReconstructed - timeValTimeNow).getTimeAsSecondsDouble();
    }

    printf("Result: calculatedResendDelay=%f timenow=%s doubleResendReconstructedDelay=%f\n",
            calculatedResendDelay, timeValTimeNow.getAsString().c_str(), doubleResendReconstructedDelay);
    return calculatedResendDelay;
}

void NetworkCodingDecoderStorage::trackDecoderLastUsedTime(string dataObjectId) {
    Timeval timeNow = Timeval::now();
    printf("Tracking last used decoder time for data object %s - time=%s\n",
            dataObjectId.c_str(), timeNow.getAsString().c_str());
    this->networkCodedDecoderLastUsedTime->operator [](dataObjectId) = timeNow.getAsString();
}

void NetworkCodingDecoderStorage::ageOffDecoder(double maxAgeDecoder) {
    Timeval timeNow = Timeval::now();

    size_t decoderStorageSize = this->decoderStorage->size();
    size_t networkCodedDecoderLastUsedTimeSize = this->networkCodedDecoderLastUsedTime->size();
    printf("decoderStorageSize=%d networkCodedDecoderLastUsedTimeSize=%d\n",decoderStorageSize,networkCodedDecoderLastUsedTimeSize);

    for (Map<string,string>::iterator it = this->networkCodedDecoderLastUsedTime->begin();
            it!=this->networkCodedDecoderLastUsedTime->end();it++) {
        string dataObjectId = it->first;
        string timeSendEvent = it->second;
        Timeval timevalTimeSendEvent(timeSendEvent);
        if( timevalTimeSendEvent + maxAgeDecoder < timeNow) {
            printf("Expiration reached - timesendevent=%s timenow=%s\n",timevalTimeSendEvent.getAsString().c_str(),timeNow.getAsString().c_str());
            this->deleteDecoderFromStorageByDataObjectId(dataObjectId);
            this->disposeNetworkCodedBlocks(dataObjectId);
        }
    }
}

void NetworkCodingDecoderStorage::deleteDecoderFromStorageByDataObjectId(string parentDataObjectId) {
    printf("Removing decoder for data object %s from networkcoding decoder storage\n",
            parentDataObjectId.c_str());
    this->decoderStorage->erase(parentDataObjectId);

    printf("Clean up last used time tracking for data object %s from networkcoding decoder storage\n",
            parentDataObjectId.c_str());
    this->networkCodedDecoderLastUsedTime->erase(parentDataObjectId);
}
