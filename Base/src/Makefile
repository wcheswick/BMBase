BIN=/usr/local/bin
LIB=${HOME}/lib

OBJ=bmscan.o dump.o broodminder.o

CFLAGS+=-static
LDFLAGS+=-static

LIBS=-lbluetooth

all::	bmscan

bmscan:	${OBJ} ${LIBS}
	${CC} ${CFLAGS} -o $@ ${OBJ} ${LIBS}

bmscan.o:	bmscan.c broodminder.h dump.h
	${CC} ${CFLAGS} -c bmscan.c

dump.o:		dump.c dump.h
	${CC} ${CFLAGS} -c dump.c

broodminder.o:	broodminder.c broodminder.h
	${CC} ${CFLAGS} -c broodminder.c

install::	${BIN}/bmscan

${BIN}/bmscan:	bmscan
	sudo cp bmscan ${BIN}/bmscan
	sudo chmod 4555 ${BIN}/bmscan

clean::
	rm -f *.o *core*
