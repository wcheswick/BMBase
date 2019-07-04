DATASRVHOST=data
BIN=${HOME}/bin

installlocal::	${BIN}/importbmbledata

${BIN}/importbmbledata:	importbmbledata
	cp importbmbledata ${BIN}/importbmbledata


installrmt::
	scp bmesplogrotate ${DATASRVHOST}:bin/bmesplogrotate
	scp bmespsrv ${DATASRVHOST}:bin/bmespsrv
	ssh ${DATASRVHOST} "sudo /etc/rc.d/inetd restart"
