/* Copyright (c) 2014 SRI International and GPC
 * Developed under DARPA contract N66001-11-C-4022.
 * Authors:
 *   Joshua Joy (JJ, jjoy)
 *   Mark-Oliver Stehr (MOS)
 */

#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>


#include "NetworkCodingFileUtility.h"

NetworkCodingFileUtility::NetworkCodingFileUtility() {

}

NetworkCodingFileUtility::~NetworkCodingFileUtility() {

}

std::string time_stamp() {

    char *timestamp = (char *) malloc(sizeof(char) * 80);
    memset(timestamp,0,sizeof(timestamp));
    time_t ltime;
    ltime = time(NULL);
    struct tm *tm;
    tm = localtime(&ltime);
    struct timeval tv;
    gettimeofday(&tv,NULL);


    sprintf(timestamp, "%04d%02d%02d%02d%02d%02d%d", tm->tm_year + 1900, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min,
            tm->tm_sec,tv.tv_usec);
    std::string stringtimestamp = timestamp;
    free(timestamp);
    return stringtimestamp;
}

std::string NetworkCodingFileUtility::createNetworkCodedBlockFileName(std::string originalFileName) {
    //FIXME
    std::string storage = "/tmp";
    std::string extension = ".nc";
    std::string timestamp = time_stamp();

    printf("originalFileName=%s timestamp=%s \n", originalFileName.c_str(),timestamp.c_str());

    std::string networkCodedBlockFileName = storage + "/"+ originalFileName + "."+timestamp+extension;

    printf("networkCodedBlockFileName=%s\n",networkCodedBlockFileName.c_str());

    return networkCodedBlockFileName;
}

std::string NetworkCodingFileUtility::createDecodedBlockFileName(const char*originalFileName) {
    int fname_len = strlen(originalFileName);

    std::string timestamp = time_stamp();
    std::string ext = "."+timestamp+".dc";
    size_t ext_len = ext.length();

    printf("Creating block file name from networkCodedFileName=%s fnamelen=%d extlen=%d\n",originalFileName,fname_len,ext_len);

    char* decodedBlockFilePtr = (char*) malloc(sizeof(char *) * 256);
    char* const decodedBlockFile = decodedBlockFilePtr;
    memset(decodedBlockFilePtr, 0, sizeof(decodedBlockFilePtr));

    strcpy(decodedBlockFilePtr, originalFileName);
    decodedBlockFilePtr = decodedBlockFilePtr + fname_len;
    strcpy(decodedBlockFilePtr, ext.c_str());
    decodedBlockFilePtr = decodedBlockFilePtr + ext_len;
    //strcpy(decodedBlockFilePtr, 0);
    printf("Result: decodedFile=%s\n", decodedBlockFile);
    std::string decodedBlockFileName = decodedBlockFile;
    free(decodedBlockFile);
    return decodedBlockFileName;
}

std::string NetworkCodingFileUtility::createDecodedBlockFilePath(const char* decodedFileName,const char* storagePath) {
    int decodedFileNameLen = strlen(decodedFileName);
    int storagePathLen = strlen(storagePath);

    printf("Creating block file path from decodedFileName=%s storagePath=%s decodedFileNameLen=%d storagePathLen=%d\n",decodedFileName,storagePath,decodedFileNameLen,storagePathLen);

    char* decodedBlockFilePathPtr = (char*) malloc(sizeof(char *) * 256);
    char* const decodedBlockFilePath = decodedBlockFilePathPtr;
    memset(decodedBlockFilePathPtr, 0, sizeof(decodedBlockFilePathPtr));

    strcpy(decodedBlockFilePathPtr,storagePath);
    decodedBlockFilePathPtr = decodedBlockFilePathPtr + storagePathLen;
    strcpy(decodedBlockFilePathPtr,"/");
    decodedBlockFilePathPtr = decodedBlockFilePathPtr+1;
    strcpy(decodedBlockFilePathPtr,decodedFileName);
    printf("Result: decodedFilePath=%s\n",decodedBlockFilePath);
    std::string decodedBlockFilePathName = decodedBlockFilePath;
    free(decodedBlockFilePath);
    return decodedBlockFilePathName;
}
