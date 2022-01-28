CC = gcc
CFLAG = -Wall
TARGET = main

OUTFILE = debug

OUTBIN = bin

OBJFILE = obj

ifneq ($(OUTFILE), $(wildcard $(OUTFILE)))
$(shell mkdir -p $(OUTFILE) $(OUTFILE)/$(OUTBIN) $(OUTFILE)/$(OBJFILE))
$(shell echo 'OBJS=*.o\nODIR=obj\n$$(ROOT_DIR)/$$(BIN_DIR)/$$(BIN):$$(ODIR)/$$(OBJS)\n\t$$(CC) -g -o $$@ $$^ $$(CFLAGS) $$(LDFLAGS)'>$(OUTFILE)/Makefile)
endif

ifneq ($(OUTFILE)/$(OUTBIN), $(wildcard $(OUTFILE)/$(OUTBIN)))
$(shell mkdir -p $(OUTFILE)/$(OUTBIN))
endif

ifneq ($(OUTFILE)/$(OBJFILE), $(wildcard $(OUTFILE)/$(OBJFILE)))
$(shell mkdir -p $(OUTFILE)/$(OBJFILE))
endif

SUBDIRS=$(shell ls -l | grep ^d | awk '{if($$9 != "debug") print $$9}')

SUBDIRS:=$(patsubst includes,,$(SUBDIRS))


ROOT_DIR=$(shell pwd)

BIN = myapp

OBJS_DIR = debug/obj

BIN_DIR = debug/bin

#CUR_SOURCE = ${wildcard *.c SUBDIRS/*.c}
CUR_SOURCE =${wildcard *.c  $(foreach dir,$(SUBDIRS),$(dir)/*c)}

CUR_OBJS = ${patsubst %.c,%.o,$(CUR_SOURCE)}



#get all include path
CFLAGS  += $(foreach dir, $(SUBDIRS), -I$(ROOT_DIR)/$(dir))
CFLAGS  += -I$(ROOT_DIR)/includes

export CC BIN OBJS_DIR BIN_DIR ROOT_DIR CFLAGS

all:$(clean) $(SUBDIRS) $(CUR_OBJS) DEBUG

$(SUBDIRS):ECHO
	make -C $@
DEBUG:ECHO
	make -C debug
ECHO:
	echo $(SUBDIRS)

$(CUR_OBJS):%.o:%.c
	$(CC) -c $^ -g -o $(ROOT_DIR)/$(OBJS_DIR)/$@ $(CFLAGS)
clean:
	rm $(OBJS_DIR)/*.o
	$(RM) $(BIN_DIR)/*

	$(RM) myapp
ck:
	export
	echo ${wildcard *.c test/*.c}
	@echo "SUBDIRS="$(SUBDIRS) "| ROOT_DIR=" $(ROOT_DIR)" | CUR_SOURCE=" $(CUR_SOURCE) " | CUR_OBJS= " $(CUR_OBJS) "| CFLAGS= " $(CFLAGS)
cp:
	echo $(ROOT_DIR)
	cp $(ROOT_DIR)/$(BIN_DIR)/myapp ./
