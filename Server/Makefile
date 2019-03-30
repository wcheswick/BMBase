HOMEBIN=${HOME}/bin

# for a FreeBSD make:

PROGS=bmplot bmplotg bmupdate

all::

.for i in ${PROGS}
install::	${HOMEBIN}/$i

${HOMEBIN}/$i:	$i
	cp $> $@
.endfor

test::
	./bmupdate green
	./bmplot green

