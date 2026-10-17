PROJECT_ROOT := $(abspath ../../..)
SHORT_NAME := P2
FULL_NAME := Pandemonium 2
CONFIG ?= Release
CXX_FLAGS := -std=gnu++98 -fno-exceptions
APP_SOURCES := $(wildcard $(PROJECT_ROOT)/src/*.c) $(wildcard $(PROJECT_ROOT)/src/*.cpp)
APP_DEFINES := -DWND_TITLE='"Pandemonium 2 (xport)"' -DWND_WIDTH=960 -DWND_HEIGHT=720 -DFIELD_RATE=60 -DFIELD_RATE_NUMERATOR=53693175 -DFIELD_RATE_DENOMINATOR=897619

ifeq ($(CONFIG),Release)
APP_DEFINES += -DXPORT_DISABLE_FUNCTION_MARKERS
endif

include ../../../../xport/src/platform/web/Makefile

# Preserve nested game calls across browser resource and event waits
LINK_FLAGS += -sASYNCIFY_STACK_SIZE=65536 -fno-exceptions
