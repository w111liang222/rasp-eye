CC=gcc
AR=ar
ARFLAGS=rcs
OPTS=-Ofast

export CC AR ARFLAGS OPTS

ifeq (${AA}, )
AA=1
endif

all :ECHO
	make -C rasp_detection/darknet_modify

ECHO :
	echo ${AA}

clean :
	make -C rasp_detection/darknet_modify clean

.PHONY : clean
