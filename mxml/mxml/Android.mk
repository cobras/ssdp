LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES :=$(LOCAL_PATH)/
LOCAL_MODULE    := mxml
LOCAL_SRC_FILES := \
	config.h \
	mvalidate.c \
	mxml-attr.c \
	mxmldoc.c \
	mxml-entity.c \
	mxml-file.c \
	mxml-get.c \
	mxml.h \
	mxml-index.c \
	mxml-node.c \
	mxml-private.c \
	mxml-private.h \
	mxml-search.c \
	mxml-set.c \
	mxml-string.c \
	testmxml.c \

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
