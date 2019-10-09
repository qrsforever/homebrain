include $(SWCFGPATH_DAILEO)/config.mk
include $(SWCFGPATH_DAILEO)/default_config.mk

BUILDTYPE = bin

############################################################ final target name
BINNAME  = demo_client

############################################################ source files.
SRC      += main.cpp
SRC      += iotSock.cpp
SRC      += iot485.cpp

VPATH = ./

############################################################ depnedent header files.
INCS     += -I./
INCS     += -I./include
INCS     += -I$(INCLUDE_PATH)/mdroid/frameworks/native/include
INCS     += -I$(INCLUDE_PATH)/mdroid/system/core/include
INCS     += -I$(INCLUDE_PATH)/mdroid/bionic/libc/include


############################################################ dependent libraries
LIBS     += -lpthread
LIBS     += -lmdroid
LIBS     += -L$(PHOTOSPHERE_ROOT)/target/$(PROJ_MODE).$(CHIP)/mslib
LIBS     += -L$(shell pwd)/
LIBS     += -Wl,--rpath-link,$(PHOTOSPHERE_ROOT)/target/$(PROJ_MODE).$(CHIP)/mslib

include ../services.mk
