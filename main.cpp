#include <memory>

#include "networkcoding/codetorrentencoder.h"
#include "networkcoding/codetorrentdecoder.h"
#include "networkcoding/CodeTorrentUtility.h"

int main() {
    printf("Running network coding example\n");
    //32KB adjust based on scenario
    size_t DEFAULT_BLOCK_SIZE = 32768;

    char* fpath="testfile";
    FILE *filePointer = fopen(fpath, "rb");
    if(!filePointer) {
        printf("Unable to open file");
        return -1;
    }
    fseek(filePointer, 0, SEEK_END);
    long fileSize = ftell(filePointer);
    fclose(filePointer);
    if(fileSize < 0) {
      printf("Unable to determine size of file %s\n", fpath);
      return -1;
    }

    std::shared_ptr<codetorrentencoder> encoderref = 
        std::make_shared<codetorrentencoder>(fpath,DEFAULT_BLOCK_SIZE,fileSize);
   
    char* outputFile="outputfile";
    std::shared_ptr<codetorrentdecoder> decoderref = 
        std::make_shared<codetorrentdecoder>(fileSize,outputFile,DEFAULT_BLOCK_SIZE); 

    std::shared_ptr<CodeTorrentUtility> codeTorrentUtility =
        std::make_shared<CodeTorrentUtility>();

    while(!decoderref->decode()) {
        //should free codedblock 
        CodedBlockPtr block=encoderref->encode();
        if(block == NULL) {
          printf("Encoding failure\n");
          return -1;
        }        
        printf("Encoded block\n");

        bool isInnovative=decoderref->store_block(0,block->coeffs,block->sums);
        printf("Stored block\n");
        codeTorrentUtility->freeCodedBlock(&block);
    }

    return 0;
}
