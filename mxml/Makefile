include mxml/Android.mk

ifeq ($(INSTALL),)
INSTALL=./install
endif

.FORCE: $(OBJ_DIR) clean

OUT_FILES=mvalidate.c mxmldoc.c testmxml.c
SRC_ALL_FILES=$(filter-out $(OUT_FILES) ,$(LOCAL_SRC_FILES))

SRC_FILES=$(filter %.c, $(SRC_ALL_FILES))

OBJ_DIR=objs

LOCAL_OBJ_FILES=$(patsubst %.c,%.o, $(SRC_FILES))
$(info $(LOCAL_OBJ_FILES))
CFLAGS=-I$(LOCAL_MODULE) -Wall #-Werror

%.o : $(LOCAL_MODULE)/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

$(OBJ_DIR) $(LIBS): 
	mkdir -p $@

lib$(LOCAL_MODULE).a: $(LOCAL_OBJ_FILES)
	$(AR) rc $@ $^

clean:
	rm -rf $(LOCAL_OBJ_FILES) lib$(LOCAL_MODULE).a


all: lib$(LOCAL_MODULE).a
