LOCAL_PATH := $(call my-dir)
 
#include $(CLEAR_VARS)
 
## Here we give our module name and source file(s)
#LOCAL_MODULE    := predinterface
#LOCAL_SRC_FILES := predinterface.cpp utility.cpp compactstore.cpp  dictionary.cpp compactdict.cpp	dictmanager.cpp dictadvance.cpp dictnext.cpp dictconfig.cpp wordpath.cpp wordlist.cpp compatibility.cpp wordpunct.cpp wordpathext.cpp testdictmanager.cpp userWordCache.cpp
#LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog 
##LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib 
#LOCAL_CPPFLAGS := -Dcplusplus -DDEBUG -DLOGGING
##LOCAL_CPPFLAGS := -Dcplusplus 
#LOCAL_CFLAGS    := -g
 
#include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := autocorrect
LOCAL_SRC_FILES := WordLogicDLL2.cpp autocorrect.cpp autocorrect/accentfilter.cpp autocorrect/acgraph.cpp autocorrect/acmatch.cpp autocorrect/geometry.cpp autocorrect/hash.cpp autocorrect/keyposition.cpp autocorrect/personaldict.cpp autocorrect/personalword.cpp
#LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog 
LOCAL_CFLAGS := -O3

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE	:= libpredinterface
LOCAL_SRC_FILES	:= prebuilt/libpredinterface.so

include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE	:= libautocorrect-jni
LOCAL_SRC_FILES	:= prebuilt/libautocorrect-jni.so

include $(PREBUILT_SHARED_LIBRARY)