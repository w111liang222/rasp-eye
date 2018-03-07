CC=gcc
AR=ar
ARFLAGS=rcs
OPTS=-Ofast

export CC AR ARFLAGS OPTS

all :
	make -C rasp_detection/darknet_modify

clean :
	make -C rasp_detection/darknet_modify clean

.PHONY : clean
