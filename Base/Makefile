BIN=/usr/local/bin
LIB=${HOME}/lib

all::
	(cd src; make)

install::	${BIN}/bmscan ${BIN}/rpi_power \
	${BIN}/BMfetch ${BIN}/BMpoll ${BIN}/BMpush

${BIN}/bmscan:
	(cd src; make install)

${BIN}/rpi_power:	rpi_power
	sudo cp rpi_power ${BIN}/rpi_power

${BIN}/BMpoll:	BMpoll
	sudo cp BMpoll ${BIN}/BMpoll

${BIN}/BMpush:	BMpush
	sudo cp BMpush ${BIN}/BMpush

${BIN}/BMfetch:	BMfetch
	sudo cp BMfetch ${BIN}/BMfetch

clean::
	(cd src; make clean)

