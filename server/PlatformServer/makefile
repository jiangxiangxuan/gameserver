
#export LANG=zh_CN.gb2312
export LC_ALL="zh_CN.utf8"

CC=gcc
CXX=g++

TARGET=platformserver

PRO_DIR=$(shell cd ./; pwd)
PRO_BIN=$(PRO_DIR)/../bin/platform
PRO_OBJ_DIR=$(PRO_DIR)/objs

PRO_SRC=$(PRO_DIR)

CFLAGS := -g -I$(PRO_SRC) -I$(PRO_DIR)/../ -I$(PRO_DIR)/../Engine -I$(PRO_DIR)/../External -I$(PRO_DIR)/../Protocol -std=c++11 #-fPIC -shared  #-std=c++11
LIBS := -lpthread -L$(PRO_DIR)/../Lib -ldl -lm -lkernal -lidbc #-llua

DIRS := $(shell find $(PRO_SRC) -maxdepth 4 -type d)
PROTOCOLDIRS := $(shell find $(PRO_SRC)/../Protocol -maxdepth 4 -type d)

SRC := $(foreach dir,$(DIRS),$(wildcard $(dir)/*.cpp))
SRC += $(foreach dir,$(PROTOCOLDIRS),$(wildcard $(dir)/*.cpp))

PRO_OBJS := $(patsubst %.cpp,%.o, $(notdir $(SRC) ))
PRO_OBJS := $(patsubst %.cpp,%.o, $(notdir $(PRO_OBJS) ))
PRO_OBJS := $(patsubst %.c,%.o, $(notdir $(PRO_OBJS) ))
PRO_OBJS := $(addprefix $(PRO_OBJ_DIR)/,$(PRO_OBJS))

all: $(PRO_OBJS)
	mkdir -p $(PRO_BIN)
	$(CXX) $(CFLAGS) $^ -o $(PRO_BIN)/$(TARGET) $(LIBS)

define compile_cxx_file
    @for file in $(SRC); do \
    ( $(CXX) $(CFLAGS) -c $$file -o $(PRO_OBJ_DIR)/`expr "$$file" : '.*/\([^.]*\).*'`.o   $(LIBS)  ) \
    done;
endef

%.o:
	mkdir -p $(PRO_OBJ_DIR)
	$(call compile_cxx_file)

clean:
	rm -rf $(PRO_BIN)/$(TARGET)
	rm -rf $(PRO_OBJ_DIR)
