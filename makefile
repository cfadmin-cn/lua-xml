.PHONY : build rebuild clean

default :
	@echo "======================================="
	@echo "Please use 'make build' command to build it.."
	@echo "Please use 'make rebuild' command to build it.."
	@echo "Please use 'make clean' command to clean all."
	@echo "======================================="

CC = cc

INCLUDES += -I../../src -I../../../src -I/usr/include/libxml2 -I/usr/local/include/libxml2
LIBS = -L../ -L../../ -L../../../ -L/usr/local/lib

CFLAGS = -O3 -Wall -shared -fPIC
DLL = -lcore -llua -lxml2

build:
	@$(CC) -o lxml.so lxml.c $(INCLUDES) $(LIBS) $(CFLAGS) $(DLL)
	@mv *.so ../
