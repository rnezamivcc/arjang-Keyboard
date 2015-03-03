############
## if you want to link to another jni folder and use that instead of current folder use this:
##### Here we point to source folder jni in main trunk folder.
###include ../../jni/Android.mk


LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)
# Here we give our module name and source file(s)
LOCAL_MODULE    := wlpredcorrect
LOCAL_LIB_NAME  := lib$(LOCAL_MODULE).a

ifeq ($(APP_OPTIM), debug)
LOCAL_CFLAGS    := -g -DDEBUG -DLOGGING
LOCAL_CPPFLAGS := -Dcplusplus -DDEBUG -DLOGGING
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog 
else
LOCAL_CFLAGS    := -g 
LOCAL_CPPFLAGS := -Dcplusplus 
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib 
endif

LOCAL_SRC_FILES := \
                    predinterface.cpp \
                    utility.cpp \
                    compactstore.cpp  \
                    dictionary.cpp \
                    compactdict.cpp \
                    parsedline.cpp \
                    dictmanager.cpp \
                    dictadvance.cpp \
                    dictnext.cpp \
                    dictconfig.cpp \
                    wordpath.cpp \
                    searchResults.cpp \
                    compatibility.cpp \
                    wordpunct.cpp \
                    wordpathext.cpp \
                    userWordCache.cpp \
                    UserWordCacheOffline.cpp \
                    T-Graph.cpp \
                    phraseEngine.cpp \
                    nGramHistory.cpp \
                    nGramLearning.cpp \
                    dictPhrase.cpp \
                    autocorrect-jni.cpp \
                    autocorrect.cpp \
                    autocorrect/acmatch.cpp \
                    autocorrect/geometry.cpp \
                    autocorrect/keyposition.cpp \
                    swipe.cpp

include $(BUILD_SHARED_LIBRARY)
