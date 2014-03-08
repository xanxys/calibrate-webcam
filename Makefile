# Copyright (c) 2013 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#
# GNU Make based build file.  For details on GNU Make see:
# http://www.gnu.org/software/make/manual/make.html
#

#
# Get pepper directory for toolchain and includes.
#
# If NACL_SDK_ROOT is not set, then assume it can be found three directories up.
#
THIS_MAKEFILE := $(abspath $(lastword $(MAKEFILE_LIST)))
NACL_SDK_ROOT := /home/xyx/nacl_sdk/pepper_33

# Project Build flags
WARNINGS := -Wno-long-long -Wall -Wswitch-enum -pedantic -Werror
CXXFLAGS := -pthread -std=gnu++98 $(WARNINGS)

#
# Compute tool paths
#
GETOS := python $(NACL_SDK_ROOT)/tools/getos.py
OSHELPERS = python $(NACL_SDK_ROOT)/tools/oshelpers.py
OSNAME := $(shell $(GETOS))
RM := $(OSHELPERS) rm

PNACL_TC_PATH := $(abspath $(NACL_SDK_ROOT)/toolchain/$(OSNAME)_pnacl)
PNACL_CXX := $(PNACL_TC_PATH)/bin/pnacl-clang++
PNACL_FINALIZE := $(PNACL_TC_PATH)/bin/pnacl-finalize
CXXFLAGS := -std=gnu++11 -D__GXX_EXPERIMENTAL_CXX11__=1 -I$(NACL_SDK_ROOT)/include -I$(NACL_SDK_ROOT)/toolchain/linux_pnacl/usr/include
LDFLAGS := -L$(NACL_SDK_ROOT)/lib/pnacl/Release -lppapi_cpp -lppapi -L$(NACL_SDK_ROOT)/toolchain/usr/lib \
	-lopencv_calib3d -lopencv_flann -lopencv_highgui -lopencv_features2d -lopencv_imgproc  -lopencv_core -lpng  -lz  -L$(NACL_SDK_ROOT)/toolchain/linux_pnacl/usr/share/OpenCV/3rdparty/lib -llibjpeg


# Declare the ALL target first, to make the 'all' target the default build
all: calibrate.pexe

clean:
	$(RM) calibrate.pexe calibrate.bc

calibrate.bc: calibrate.cpp
	$(PNACL_CXX) -o $@ $< -O2 $(CXXFLAGS) $(LDFLAGS)

calibrate.pexe: calibrate.bc
	$(PNACL_FINALIZE) -o $@ $<
