STRIP ?= strip

APPNAME           := client
HUE_BRIDGE_BINARY := client
TARGETS := $(HUE_BRIDGE_BINARY)

# Sources for demo
SOURCES := \
	LarfeClient.cpp \
	LarfeDevice.cpp \
	LarfeClientLog.cpp \
	client.cpp

# Must end with a trailing slash or must be empty.
TOOLSOBJDIR := obj/
TOOLOBJS := $(addprefix $(TOOLSOBJDIR),$(addsuffix .o,$(basename $(notdir $(SOURCES)))))	

##############
# build flags
##############

CC := g++
CPPFLAGS := -I../../../../extlibs/rapidjson/rapidjson/include -I../../../src/utils/Log/src/ -Wall -O2
LDFLAGS := -L../../../../out/linux/x86_64/release/homebrain/src/utils $(JSONC_LIB) -lpthread -lrt -lm -lutils_log -lutils_message -lutils_misc

##############
# make targets
##############

$(TARGETS): $(TOOLSOBJDIR) $(TOOLOBJS) 
	$(info Linking $@ ...)
	$(CC) $(CPPFLAGS) -o $@ $(TOOLOBJS) $(LDFLAGS) 
	if [ -z "$$NOSTRIP" ]; then $(STRIP) $@; fi


$(TOOLSOBJDIR):
	mkdir $(TOOLSOBJDIR)

$(TOOLSOBJDIR)%.o: %.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<


