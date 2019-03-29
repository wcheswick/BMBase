# get bluez and make it.  Then point these to the relevant directory
#
# bluez hould really install this stuff in the system, but it doesn't seem to.

#BLUEZ_SRC=${HOME}/src/bluez-5.43
#BLUEZ_INC=-I${BLUEZ_SRC}/lib
#BLUEZ_LIB=${BLUEZ_SRC}/lib/.libs

BIN=/usr/local/bin
LIB=${HOME}/lib

OBJ=bmscan.o dump.o broodminder.o

#CFLAGS+=-I${BLUEZ_SRC}
#LDFLAGS+=-L${BLUEZ_LIB}

CFLAGS+=-static
LDFLAGS+=-static

LIBS=-lbluetooth
#LIBS=${BLUEZ_LIB}/libbluetooth-internal.a 


all::	bmscan

bmscan:	${OBJ} ${LIBS}
	${CC} ${CFLAGS} -o $@ ${OBJ} ${LIBS}

bmscan.o:	bmscan.c broodminder.h dump.h
	${CC} ${CFLAGS} -c bmscan.c

dump.o:		dump.c dump.h
	${CC} ${CFLAGS} -c dump.c

broodminder.o:	broodminder.c broodminder.h
	${CC} ${CFLAGS} -c broodminder.c

install::	${BIN}/bmscan $(BIN)/poll_hives \
	${BIN}/aploop ${BIN}/rpi_network ${BIN}/rpi_light ${BIN}/rpi_power \
	${BIN}/BMpoll ${BIN}BMpush

${BIN}/rpi_power:	rpi_power
	sudo cp rpi_power ${BIN}/rpi_power

${BIN}/rpi_network:	rpi_network
	sudo cp rpi_network ${BIN}/rpi_network

${BIN}/rpi_light:	rpi_light
	sudo cp rpi_light ${BIN}/rpi_light

${BIN}/aploop:	aploop
	sudo cp aploop ${BIN}/aploop

${BIN}/bmscan:	bmscan
	sudo cp bmscan ${BIN}/bmscan
	sudo chmod 4555 ${BIN}/bmscan

${BIN}/poll_hives:	poll_hives
	sudo cp poll_hives ${BIN}/poll_hives

${BIN}/BMpoll:	BMpoll
	sudo cp BMpoll ${BIN}/BMpoll

${BIN}/BMpush:	BMpush
	sudo cp BMpus ${BIN}/BMpush

clean::
	rm -f *.o *core*

test::		all
	sudo ./bmscan || (sudo hciconfig hci0 down; sudo hciconfig hci0 up; \
		sudo ./bmscan)

