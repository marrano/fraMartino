#15/02/2011 Stefano Burchietti

VERSION = 0.1
CC = gcc
CFLAGS=-Wall
LFLAGS=
SRC_DIR=src/
BUILD_DIR=build/
FRAMARTINO_BIN = fraMartino
FRAMARTINO_SRC = fraMartino.c
$LIBS=

all:	fraMartino

fraMartino:
	${CC} $(CFLAGS) $(LFLAGS) -o ${BUILD_DIR}${FRAMARTINO_BIN} ${SRC_DIR}${FRAMARTINO_SRC} $(LIBS)

clean:       cleanbin
	rm -f ${BUILD_DIR}*.o  ${SRC_DIR}*~ 

cleanbin:
	rm -f ${BUILD_DIR}${FRAMARTINO_BIN}

tar:
	tar czvf fraMartino.tgz Makefile src/ README TODO

# DO NOT DELETE