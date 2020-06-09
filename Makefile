include $(THEOS)/makefiles/common.mk

ARCHS = arm64 arm64e
TOOL_NAME=key_dumper aes_ap
key_dumper_FILES = key_dumper.c
key_dumper_FRAMEWORKS = IOKit
key_dumper_CODESIGN_FLAGS = -Stfp0.plist

aes_ap_FILES = golb.c aes_ap.c
key_dumper_CODESIGN_FLAGS = -Stfp0.plist


include $(THEOS_MAKE_PATH)/tool.mk

