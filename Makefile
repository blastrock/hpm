# Makefile for hpm
# $Id: Makefile,v 0.6.2.1 2001/12/13 23:59:58 izto Exp $

CFLAGS+=-D_GNU_SOURCE -DPIC -fPIC -D_REENTRANT 
PREFIX=/usr/local
DESTDIR=$(PREFIX)

libhpmwatch.so: hpmwatch.o
	$(LD) -shared -o libhpmwatch.so hpmwatch.o -ldl -lc -fPIC

.PHONY: clean
clean:
	rm -f hpmwatch.o libhpmwatch.so

.PHONY: install
install: libhpmwatch.so
	cp libhpmwatch.so $(DESTDIR)/lib
	cp hpm hpmwatch $(DESTDIR)/bin

uninstall:
	rm -f $(DESTDIR)/lib/libhpmwatch.so
	rm -f $(DESTDIR)/bin/hpm $(DESTDIR)/bin/hpmwatch
