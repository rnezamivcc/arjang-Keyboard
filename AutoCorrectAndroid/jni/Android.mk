LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := autocorrect-jni
LOCAL_SRC_FILES := autocorrect-jni.cpp
LOCAL_SRC_FILES += autocorrect/wordhash.cpp autocorrect/vertex.cpp autocorrect/acMatch.cpp

#LOCAL_CPPFLAGS	:= -Dcplusplus -DDEBUG -DLOGGING
#LOCAL_CFLAGS 	:= -g

LOCAL_LDLIBS	:= -llog

include $(BUILD_SHARED_LIBRARY)