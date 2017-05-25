PD_SRC_PATH ?= /Applications/Pd-0.47-1-64bit.app/Contents/Resources/src
LIBNAME = dualshock.pd_darwin
SRC = dualshock.cpp
CC = clang
PREFIX ?= /opt/local

CFLAGS = -fno-rtti -Os -fno-exceptions -undefined dynamic_lookup
CXXFLAGS = -I$(PD_SRC_PATH) -I$(PREFIX)/include/hidapi $(CFLAGS) -stdlib=libc++ -std=c++14 -Wall -Werror
LDFLAGS = -L$(PREFIX)/lib -lhidapi

.phony: default remote moog

default: dualshock.pd_darwin

dualshock.pd_darwin: $(SRC) dualshock.h
	$(CC) $(CXXFLAGS) $(LDFLAGS) -shared -o $(LIBNAME) $(SRC)

remote: remote.pd_darwin

remote.pd_darwin: vendor/remote.c
	$(CC) -I$(PD_SRC_PATH) $(CFLAGS) -stdlib=libc++ -shared -o remote.pd_darwin vendor/remote.c

moog: moog~.pd_darwin

moog~.pd_darwin: vendor/moog~.c
	$(CC) -I$(PD_SRC_PATH) $(CFLAGS) -stdlib=libc++ -shared -o moog~.pd_darwin vendor/moog~.c

clean:
	@rm -rf $(LIBNAME)
