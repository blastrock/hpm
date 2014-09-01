# Makefile for hpm
# $Id: Makefile,v 0.6.2.1 2001/12/13 23:59:58 izto Exp $

CFLAGS+=-D_GNU_SOURCE -DPIC -fPIC -D_REENTRANT
CXXFLAGS+=-D_GNU_SOURCE -DPIC -fPIC -D_REENTRANT -std=c++0x
PREFIX=/usr/local
DESTDIR=$(PREFIX)
UNAME_S=$(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LIBNAME=libhpmwatch.dylib
else
	LIBNAME=libhpmwatch.so
endif


$(LIBNAME): hpmwatch.o
ifeq ($(UNAME_S),Darwin)
	g++ -std=c++0x -dynamiclib -o libhpmwatch.dylib hpmwatch.o
else
	$(CXX) -std=c++0x -shared -o libhpmwatch.so hpmwatch.o -ldl -lc -fPIC -lpthread
endif

.PHONY: clean
clean:
	rm -f hpmwatch.o $(LIBNAME)

.PHONY: install
install: libhpmwatch.so
	cp $(LIBNAME) $(DESTDIR)/lib
	cp hpm hpmwatch $(DESTDIR)/bin

uninstall:
	rm $(DESTDIR)/lib/$(LIBNAME)
	rm -f $(DESTDIR)/bin/hpm $(DESTDIR)/bin/hpmwatch
