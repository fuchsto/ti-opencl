#
#  Copyright (c) 2012-2017 Texas Instruments Incorporated - http://www.ti.com
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  *  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#  *  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#  *  Neither the name of Texas Instruments Incorporated nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
#  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

#
#  ======== make_rtos.inc ========
#
ifeq ($(OS), Windows_NT)
DEFAULT_PSDK_RTOS      ?= C:\ti\ti-processor-sdk-rtos-am57xx-evm-04.00.00.01-20170530
GCC_ARM_NONE_TOOLCHAIN ?= C:\ti\ccsv6\tools\compiler\gcc-arm-none-eabi-4_9-2015q3

TI_OCL_CGT_INSTALL     ?= C:\ti\ccsv6\tools\compiler\ti-cgt-c6000_8.2.0
DSS_CMD                 = $(CCS_INSTALL_DIR)\ccs_base\scripting\bin\dss.bat
NULLFILE                = NUL

else  # Linux
DEFAULT_PSDK_RTOS      ?= /cgnas/ti-processor-sdk-rtos-am57xx-evm-04.00.00.01-20170530
GCC_ARM_NONE_TOOLCHAIN ?= /cgnas/gcc-arm-none-eabi-4_9-2015q3

TI_OCL_CGT_INSTALL     ?= /cgnas/ti-cgt-c6000-8.2.0-release-linux
DSS_CMD                 = $(CCS_INSTALL_DIR)/ccs_base/scripting/bin/dss.sh
NULLFILE                = /dev/null
endif

TI_OCL_INSTALL         ?= $(wildcard $(DEFAULT_PSDK_RTOS)/opencl_rtos_*)/packages/ti/opencl

BIOS_INSTALL_DIR       ?= $(wildcard $(DEFAULT_PSDK_RTOS)/bios_*)
IPC_INSTALL_DIR        ?= $(wildcard $(DEFAULT_PSDK_RTOS)/ipc_*)
XDC_INSTALL_DIR        ?= $(wildcard $(DEFAULT_PSDK_RTOS)/xdctools_*)

PROFILE                ?= release
HOSTCONFIG             ?= $(OCL_INSTALL_DIR)/packages/ti/opencl/Host.cfg

export TI_OCL_INSTALL
export TI_OCL_CGT_INSTALL
export PATH:=$(TI_OCL_CGT_INSTALL)/bin:$(PATH)


CC                = $(GCC_ARM_NONE_TOOLCHAIN)/bin/arm-none-eabi-gcc
CXX               = $(GCC_ARM_NONE_TOOLCHAIN)/bin/arm-none-eabi-g++
LD                = $(GCC_ARM_NONE_TOOLCHAIN)/bin/arm-none-eabi-gcc
OCL_INSTALL_DIR   = $(TI_OCL_INSTALL)/../../..
ifeq ($(OS), Windows_NT)
CLOCL             = $(TI_OCL_INSTALL)/usr/share/ti/opencl/bin/pc/clocl
else
CLOCL             = $(TI_OCL_INSTALL)/usr/share/ti/opencl/bin/x86/clocl
endif
DSP_INCLUDE       = -I$(TI_OCL_CGT_INSTALL)/include
DSP_INCLUDE      += -I$(TI_OCL_INSTALL)/usr/share/ti/opencl
CL6X              = $(TI_OCL_CGT_INSTALL)/bin/cl6x -mv6600 --abi=eabi $(DSP_INCLUDE)
DSP1_OUT          = $(TI_OCL_INSTALL)/usr/share/ti/opencl/dsp0.out
DSP2_OUT          = $(TI_OCL_INSTALL)/usr/share/ti/opencl/dsp1.out

ifeq ($(MAKECMDGOALS), test)
    ifeq ($(CCS_INSTALL_DIR),)
        $(error Environment variable CCS_INSTALL_DIR must be defined. Set it to point at Code Composer Studio installation (e.g. /opt/ti/ccsv6))
    endif
    ifeq ($(CCS_TARGET_CONFIG),)
        $(error Environment variable CCS_TARGET_CONFIG must be defined. Set it to point at CCS target configuration file (e.g. \$HOME/ti/CCSTargetConfigurations/AM57.ccxml))
    endif
endif


# Use this goal to print your product variables.
.show:
	@echo "BIOS_INSTALL_DIR       = $(BIOS_INSTALL_DIR)"
	@echo "IPC_INSTALL_DIR        = $(IPC_INSTALL_DIR)"
	@echo "XDC_INSTALL_DIR        = $(XDC_INSTALL_DIR)"
	@echo "OCL_INSTALL_DIR        = $(OCL_INSTALL_DIR)"
	@echo "GCC_ARM_NONE_TOOLCHAIN = $(GCC_ARM_NONE_TOOLCHAIN)"
	@echo "TI_OCL_CGT_INSTALL     = $(TI_OCL_CGT_INSTALL)"
	@echo "TI_OCL_INSTALL         = $(TI_OCL_INSTALL)"


PROCLIST    = host

# Avoid repeated xdc configuring for examples using the default Host.cfg
ifeq ($(HOSTCONFIG), $(OCL_INSTALL_DIR)/packages/ti/opencl/Host.cfg)
    CONFIG  = ../common/$(PROFILE)/cfg
else
    CONFIG  = bin/$(PROFILE)/cfg
endif

objs = $(addprefix bin/$(PROFILE)/,$(patsubst %.cpp,%.oa15fg,$(srcs)))
libs = -Wl,-T,$(CONFIG)/linker.cmd -lrdimon -lgcc -lc -lm -lnosys -lstdc++ \
       -Wl,--start-group -Wl,--end-group

.PRECIOUS: %/compiler.opt %/linker.cmd

$(EXE).x: bin/$(PROFILE)/$(EXE).xa15fg
bin/$(PROFILE)/$(EXE).xa15fg: $(objs) $(CONFIG)/linker.cmd
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(LD) $(LDFLAGS) -Wl,-Map,"$(EXE).map" -o $@ $(objs) $(libs) $(LDLIBS)
	$(RM) *.map
#	$(RMDIR) $(CONFIG)

bin/$(PROFILE)/%.oa15fg: %.cpp $(CONFIG)/compiler.opt
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(CXX) $(CXX_FLAGS) $(CFLAGS) -o $@ $<

bin/$(PROFILE)/%.oa15fg: %.c $(CONFIG)/compiler.opt
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(CXX) $(CXX_FLAGS) $(CFLAGS) -o $@ $<

%.obj: %.c
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(CL6X) -c $(CL6X_FLAGS) $<

%.dsp_h: %.cl
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(CLOCL) --txt $(CLOCL_FLAGS) $^

%/compiler.opt: %/linker.cmd ;
$(CONFIG)/compiler.opt: $(CONFIG)/linker.cmd ;

$(CONFIG)/linker.cmd: $(HOSTCONFIG)
	@$(ECHO) "#"
	@$(ECHO) "# Making $@ ..."
	$(XDC_INSTALL_DIR)/xs --xdcpath="$(subst +,;,$(PKGPATH))" \
            xdc.tools.configuro -o $(CONFIG) \
            -t gnu.targets.arm.A15F \
            -c $(GCC_ARM_NONE_TOOLCHAIN) \
            -p ti.opencl.platforms.am57x_rtos:HOST \
            -r $(PROFILE) \
	    $(HOSTCONFIG)

help:
	@$(ECHO) "make                   # build executable"
	@$(ECHO) "make clean             # clean everything"

clean::
	$(RMDIR) bin *.out *.dsp_h *.obj autoRunLog.xml
	$(RM) *.map

realclean: clean
	$(RMDIR) $(CONFIG)

test: clean
	@$(ECHO) "#"
	@$(ECHO) "# Making  bin/$(PROFILE)/$(EXE).xa15fg ..."
	@$(MAKE) -f Makefile.rtos $(EXE).x > $(NULLFILE)
	@$(ECHO) "#"
	@$(ECHO) "# Running bin/$(PROFILE)/$(EXE).xa15fg ..."
	@$(DSS_CMD) ../load_am57_rtos.js $(CCS_TARGET_CONFIG) \
                   bin/$(PROFILE)/$(EXE).xa15fg $(DSP1_OUT) $(DSP2_OUT) 0

test_noclean:
	$(DSS_CMD) ../load_am57_rtos.js $(CCS_TARGET_CONFIG) \
                   bin/$(PROFILE)/$(EXE).xa15fg $(DSP1_OUT) $(DSP2_OUT) 1


PKGPATH := $(BIOS_INSTALL_DIR)/packages
PKGPATH := $(PKGPATH)+$(IPC_INSTALL_DIR)/packages
PKGPATH := $(PKGPATH)+$(OCL_INSTALL_DIR)/packages

#  ======== install validation ========
ifeq (install,$(MAKECMDGOALS))
ifeq (,$(EXEC_DIR))
$(error must specify EXEC_DIR)
endif
endif

#  ======== toolchain macros ========

CXX_FLAGS += -Dfar= -D__DYNAMIC_REENT__ -D_TI_RTOS -fpermissive -std=c++0x
CFLAGS   += -c -MD -MF $@.dep -MMD -MP -mcpu=cortex-a15 -mtune=cortex-a15 -mfloat-abi=hard -ffunction-sections -fdata-sections $(CCPROFILE_$(PROFILE)) @$(CONFIG)/compiler.opt -I"$(GCC_ARM_NONE_TOOLCHAIN)/arm-none-eabi/include" -I"$(GCC_ARM_NONE_TOOLCHAIN)/arm-none-eabi/include/newlib-nano" -I. -I$(TI_OCL_INSTALL)/usr/include

LDFLAGS += $(LDPROFILE_$(PROFILE)) -mfloat-abi=hard -nostartfiles --specs=nano.specs -Wl,-static -Wl,--gc-sections -Wl,-Map=$(@D)/$(@F).map
LDLIBS += -L$(BIOS_INSTALL_DIR)/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/lib/hard

CCPROFILE_debug = -g -ggdb -D_DEBUG_=1
CCPROFILE_release = -O3

LDPROFILE_debug = -g -ggdb
LDPROFILE_release =

#  ======== standard macros ========
ifneq (,$(wildcard $(XDC_INSTALL_DIR)/bin/echo.exe))
    # use these on Windows
    CP      = $(XDC_INSTALL_DIR)/bin/cp
    ECHO    = $(XDC_INSTALL_DIR)/bin/echo
    MKDIR   = $(XDC_INSTALL_DIR)/bin/mkdir -p
    RM      = $(XDC_INSTALL_DIR)/bin/rm -f
    RMDIR   = $(XDC_INSTALL_DIR)/bin/rm -rf
else
    # use these on Linux
    CP      = cp
    ECHO    = echo
    MKDIR   = mkdir -p
    RM      = rm -f
    RMDIR   = rm -rf
endif

#  ======== create output directories ========
ifneq (clean,$(MAKECMDGOALS))
ifneq (,$(PROFILE))
ifeq (,$(wildcard bin/$(PROFILE)))
    $(shell $(MKDIR) -p bin/$(PROFILE))
endif
endif
endif
