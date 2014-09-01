# Makefile for hpm
# $Id: Makefile,v 0.6.2.1 2001/12/13 23:59:58 izto Exp $

CFLAGS+=-D_GNU_SOURCE -DPIC -fPIC -D_REENTRANT
CXXFLAGS+=-D_GNU_SOURCE -DPIC -fPIC -D_REENTRANT -std=c++0x
PREFIX=/usr/local
DESTDIR=$(PREFIX)
UNAME_S=$(shell uname -s)

libhpmwatch.so: hpmwatch.o
ifeq ($(UNAME_S),Darwin)
	g++ -std=c++0x -dynamiclib -o libhpmwatch.dylib hpmwatch.o
else
	$(CXX) -std=c++0x -shared -o libhpmwatch.so hpmwatch.o -ldl -lc -fPIC -lpthread
endif

.PHONY: clean
clean:
ifeq ($(UNAME_S),Darwin)
	rm -f hpmwatch.o libhpmwatch.dylib
else
	rm -f hpmwatch.o libhpmwatch.so
endif

.PHONY: install
install: libhpmwatch.so
ifeq ($(UNAME_S),Darwin)
	cp libhpmwatch.dylib $(DESTDIR)/lib
else
	cp libhpmwatch.so $(DESTDIR)/lib
endif
	cp hpm hpmwatch $(DESTDIR)/bin

uninstall:
ifeq ($(UNAME_S),Darwin)
	rm $(DESTDIR)/lib/libhpmwatch.dylib
else
	rm $(DESTDIR)/lib/libhpmwatch.so
endif
	rm -f $(DESTDIR)/bin/hpm $(DESTDIR)/bin/hpmwatch
