CURDIR=$(shell pwd)

CROSS_COMPILE=aarch64-linux-android-
CPPFLAG = $(FLAG)
CPPFLAG += -I/home/lsc/RC_CCU_C++SDK/kk_sdk/lib/include 
LDFLAGS=-L/home/lsc/RC_CCU_C++SDK/kk_sdk/lib
INSTALL_BIN=/home/lsc/RC_CCU_C++SDK/kk_sdk/demo_bin

export INSTALL_BIN
export CPPFLAG
export LDFLAGS
export CROSS_COMPILE

$(warning  $(CPPFLAG))
################# bin
ifeq ("$(SUBDIR)", "")
SUBDIR=HJ_SDK_NO_ACE_Demo
endif


all clean install:
	@for i in $(SUBDIR); do cd $$i/build && make -j4  $(MAKECMDGOALS) || exit "$$?"; cd "$(CURDIR)"; done
        
.PHONY:all clean install

