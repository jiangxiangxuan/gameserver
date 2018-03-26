
#export LANG=zh_CN.gb2312
export LC_ALL="zh_CN.utf8"

CC=gcc
CXX=g++

TARGET=libkernal.so

PRO_DIR=$(shell cd ./; pwd)
PRO_BIN=$(PRO_DIR)/../../Lib
PRO_OBJ_DIR=$(PRO_DIR)/objs

PRO_EXTERNAL_INC=$(PRO_DIR)/../../External
PRO_SRC=$(PRO_DIR)

CFLAGS := -g -I$(PRO_SRC) -I$(PRO_DIR)/../../External -I$(PRO_DIR)/../../Engine -fPIC -shared  -std=c++11
LIBS := -lpthread -L$(PRO_DIR)/../../Lib   #-lluajit

DIRS := $(shell find $(PRO_SRC) -maxdepth 4 -type d)
EXTERNAL_DIRS := $(shell find $(PRO_EXTERNAL_INC) -maxdepth 4 -type d)

SRC := $(foreach dir,$(DIRS),$(wildcard $(dir)/*.cpp))
SRC += $(foreach dir,$(EXTERNAL_DIRS),$(wildcard $(dir)/*.cpp))

SRC_C := $(foreach dir,$(EXTERNAL_DIRS),$(wildcard $(dir)/*.c))

PRO_OBJS := $(patsubst %.cpp,%.o, $(notdir $(SRC) ))
PRO_OBJS := $(patsubst %.cpp,%.o, $(notdir $(PRO_OBJS) ))
PRO_OBJS := $(patsubst %.c,%.o, $(notdir $(PRO_OBJS) ))
PRO_OBJS := $(addprefix $(PRO_OBJ_DIR)/,$(PRO_OBJS))

PRO_OBJS_C := $(patsubst %.cpp,%.o, $(notdir $(SRC_C) ))
PRO_OBJS_C := $(patsubst %.cpp,%.o, $(notdir $(PRO_OBJS_C) ))
PRO_OBJS_C := $(patsubst %.c,%.o, $(notdir $(PRO_OBJS_C) ))
PRO_OBJS_C := $(addprefix $(PRO_OBJ_DIR)/,$(PRO_OBJS_C))

all: $(PRO_OBJS) #$(PRO_OBJS_C)
	mkdir -p $(PRO_BIN)
	$(CXX) $(CFLAGS) $^ -o $(PRO_BIN)/$(TARGET) $(LIBS)

define compile_cxx_file
    @for file in $(SRC); do \
    ( $(CXX) $(CFLAGS) -c $$file -o $(PRO_OBJ_DIR)/`expr "$$file" : '.*/\([^.]*\).*'`.o   $(LIBS)  ) \
    done;
endef

define compile_c_file
    @for file in $(SRC_C); do \
    ( $(CC) $(CFLAGS) -c $$file -o $(PRO_OBJ_DIR)/`expr "$$file" : '.*/\([^.]*\).*'`.o   $(LIBS)  ) \
    done;
endef

%.o:
	mkdir -p $(PRO_OBJ_DIR)
	$(call compile_cxx_file)
	#$(call compile_c_file)

clean:
	rm -rf $(PRO_BIN)/$(TARGET)
	rm -rf $(PRO_OBJ_DIR)
