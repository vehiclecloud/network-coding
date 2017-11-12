/* Copyright (c) 2014 SRI International and GPC
 * Developed under DARPA contract N66001-11-C-4022.
 * Authors:
 *   Joshua Joy (JJ, jjoy)
 */

#ifndef NETWORKCODINGFILEUTILITY_H_
#define NETWORKCODINGFILEUTILITY_H_

#include <string>

class NetworkCodingFileUtility {
public:
    NetworkCodingFileUtility();
    virtual ~NetworkCodingFileUtility();

    std::string createNetworkCodedBlockFileName(std::string originalFileName);
    std::string createDecodedBlockFileName(const char*networkCodedFileName);
    std::string createDecodedBlockFilePath(const char* decodedFileName,const char* storagePath);
};

#endif /* NETWORKCODINGFILEUTILITY_H_ */
