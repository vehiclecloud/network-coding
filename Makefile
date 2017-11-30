CC=g++
CFLAGS=-c -Wall -std=c++11
LDFLAGS=

INCLUDES=-I/home/josh/projects/network-coding

SOURCES= main.cpp \
networkcoding/codetorrent.cpp \
networkcoding/codetorrentdecoder.cpp \
networkcoding/codetorrentencoder.cpp \
networkcoding/CodeTorrentUtility.cpp \
networkcoding/galois.cpp \
networkcoding/nc.cpp \
networkcoding/NetworkCodingFileUtility.cpp \
networkcoding/singleblockencoder.cpp

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=networkcodingtest

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(INCLUDES) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	@rm -rf $(OBJECTS) $(EXECUTABLE)
