PHONY = all

TOPDIR := $(shell pwd)

PLATFORMS_DIR := $(TOPDIR)/homebrain/build/platforms

ifeq ($(J), )
	JJ := 2
else
	JJ := $(J)
endif

all: ubuntu

mstar648:
	@echo "Build $@ $(D)"
	DEBUG=0 JJ=$(JJ) $(PLATFORMS_DIR)/$@/build.sh $(TOPDIR) $@ $(D)
	@echo "Build End"

kk:
	@echo "Build $@ $(D)"
	DEBUG=0 JJ=$(JJ) $(PLATFORMS_DIR)/$@/build.sh $(TOPDIR) $@ $(D)
	@echo "Build End"

ubuntu:
	@echo "Build $@ $(D)"
	DEBUG=0 JJ=$(JJ) $(PLATFORMS_DIR)/$@/build.sh $(TOPDIR) $@ $(D)
	@echo "Build End"

cetron:
	@echo "Build $@ $(D)"
	DEBUG=0 JJ=$(JJ) $(PLATFORMS_DIR)/$@/build.sh $(TOPDIR) $@ $(D)
	@echo "Build End"

clean:
	@echo "Clean project"
	$(TOPDIR)/homebrain/build/script/clean_project.sh  $(TOPDIR)
	@echo "Clean End"
