PD_SRC_PATH ?= /Applications/Pd-0.47-1-64bit.app/Contents/Resources/src
LIBNAME = dualshock.pd_darwin
SRC = dualshock.cpp
CC = clang
PREFIX ?= /opt/local

CXXFLAGS = -I$(PD_SRC_PATH) -I$(PREFIX)/include/hidapi -stdlib=libc++ -std=c++14 -undefined dynamic_lookup -Wall -Werror
LDFLAGS = -L$(PREFIX)/lib -lhidapi

default: dualshock.pd_darwin

dualshock.pd_darwin: $(SRC)
	$(CC) $(CXXFLAGS) $(LDFLAGS) -shared -o $(LIBNAME) $(SRC)

clean:
	@rm -rf $(LIBNAME)
