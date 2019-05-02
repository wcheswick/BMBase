HOMEBIN=${HOME}/bin

# for a FreeBSD make:

PROGS=bmplotg bmupdate bmplotall bmcheckforupdate bmplot bmplotap

all::

.for i in ${PROGS}
install::	${HOMEBIN}/$i

${HOMEBIN}/$i:	$i
	cp $> $@
.endfor

test::
	./bmupdate green
	./bmplot green

