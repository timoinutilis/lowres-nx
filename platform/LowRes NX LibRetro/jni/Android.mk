LOCAL_PATH := $(call my-dir)

SRCDIR := $(LOCAL_PATH)/../../../

INCFLAGS       :=   -I $(SRCDIR)/core \
                    -I $(SRCDIR)/core/machine \
                    -I $(SRCDIR)/core/accessories \
                    -I $(SRCDIR)/core/datamanager \
                    -I $(SRCDIR)/core/interpreter \
                    -I $(SRCDIR)/core/libraries \
                    -I $(SRCDIR)/core/overlay

COREFLAGS := -fomit-frame-pointer -ffast-math -D__LIBRETRO__ $(INCFLAGS)
COREFLAGS += -DGB_INTERNAL -DDISABLE_DEBUGGER -DPLATFORM_ANDROID

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
  COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

SRCFILES     :=  $(wildcard $(SRCDIR)/core/*.c)\
                  $(wildcard $(SRCDIR)/core/*/*.c)\
                  $(wildcard $(SRCDIR)/libretro/*.c)

include $(CLEAR_VARS)
LOCAL_MODULE       := retro
LOCAL_SRC_FILES    := $(SRCFILES)
LOCAL_CPPFLAGS     := -std=c++17 $(COREFLAGS)
LOCAL_CFLAGS       := $(COREFLAGS)
LOCAL_LDFLAGS      := -Wl,-version-script=../link.T
LOCAL_CPP_FEATURES := exceptions rtti
include $(BUILD_SHARED_LIBRARY)