PD_SRC_PATH ?= /Applications/Pd-0.47-1-64bit.app/Contents/Resources/src
LIBNAME ?= hid.pd_darwin
SRC = hid.cpp
CC = clang

CXXFLAGS = -I$(PD_SRC_PATH) -stdlib=libc++ -std=c++14 -undefined dynamic_lookup -Wall -Werror
LDFLAGS =

default: hid.pd_darwin

hid.pd_darwin: $(SRC)
	$(CC) $(CXXFLAGS) $(LDFLAGS) -shared -o $(LIBNAME) $(SRC)

clean:
	@rm -rf $(LIBNAME)
