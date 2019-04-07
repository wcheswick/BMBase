BIN=/usr/local/bin
LIB=${HOME}/lib

all::
	(cd src; make)

install::	${BIN}/bmscan ${BIN}/rpi_power \
	${BIN}/bmfetch ${BIN}/bmpoll ${BIN}/bmpush

${BIN}/bmscan::
	(cd src; make install)

${BIN}/rpi_power:	rpi_power
	sudo cp rpi_power ${BIN}/rpi_power

${BIN}/bmpoll:	bmpoll
	sudo cp bmpoll ${BIN}/bmpoll

${BIN}/bmpush:	bmpush
	sudo cp bmpush ${BIN}/bmpush

${BIN}/bmfetch:	bmfetch
	sudo cp bmfetch ${BIN}/bmfetch

clean::
	(cd src; make clean)

