
# ------------------------------------------------------------------------------
#
# If making on x86 assume cross compile for ARM target
#
# Environment:
# - PSDK_LINUX : Target Linux Processor SDK for automatic configuration. If not
#   set, must specify paths in overridable variables.
# - PSDK_RTOS : Target RTOS Processor SDK for automatic configuration. If not
#   set and required (for persistent examples), must specify paths in
#   overridable variables.
#
# Overrides:
# - TARGET_ROOTDIR : ARM Linux devkit/filesystem
# - HOST_ROOTDIR : x86_64 Linux devkit
# - TI_OCL_INSTALL : TI OpenCL installation prefix if not from PSDK or target
#   devkit/filesystem
# - TI_OCL_CGT_INSTALL : TI C6x CGT installation directory if not from PSDK or
#   host devkit
# - CXX : C++ cross-compiler if not from PSDK or host devkit
# - CLOCL : clocl if not from PSDK, host devkit, or TI_OCL_INSTALL
#
# ------------------------------------------------------------------------------
#
# If making on the ARM target, use native compilation
#
# Overrides:
# - TI_OCL_INSTALL : TI OpenCL installation prefix if not system package
# - TI_OCL_CGT_INSTALL : TI C6x CGT installation directory if not system package
#
# ------------------------------------------------------------------------------

ifeq ($(MAKECMDGOALS),clean)
    # config not required for clean

else
ifneq (,$(findstring 86, $(shell uname -m)))
    # --------------------------------------------------------------------------
    # host cross-compilation

    ifeq ($(PSDK_LINUX),)
        ifeq ($(TARGET_ROOTDIR),)
            $(error Environment variable PSDK_LINUX is not defined; set it to point at the target Linux PSDK for automatic configuration or set TARGET_ROOTDIR to the ARM Linux devkit/filesystem)
        endif
    endif

    export TARGET_ROOTDIR?=$(PSDK_LINUX)/linux-devkit/sysroots/armv7ahf-neon-linux-gnueabi
    export HOST_ROOTDIR?=$(PSDK_LINUX)/linux-devkit/sysroots/x86_64-arago-linux
    export TI_OCL_INSTALL?=$(TARGET_ROOTDIR)
    export TI_OCL_CGT_INSTALL?=$(HOST_ROOTDIR)/usr/share/ti/cgt-c6x

    # --------------------------------------------------------------------------
    # ARM GCC

    ifeq ($(origin CXX),default)
        CXX=
    endif
    ifeq ($(CXX),)
        ifneq ($(wildcard $(HOST_ROOTDIR)/usr/bin/arm-linux-gnueabihf-g++),)
            CXX=$(HOST_ROOTDIR)/usr/bin/arm-linux-gnueabihf-g++
        else
            CXX=arm-linux-gnueabihf-g++
        endif
    endif

    ifneq ($(shell which $(firstword $(CXX)) >/dev/null 2>/dev/null; echo $$?),0)
        $(error C++ cross-compiler $(CXX) not found. Set PSDK_LINUX to point to the target Linux PSDK for automatic configuration, set HOST_ROOTDIR to the x86_64 Linux devkit, or add the compiler bin directory to PATH.)
    endif

    # --------------------------------------------------------------------------
    # clocl

    ifeq ($(CLOCL),)
        ifneq ($(wildcard $(TI_OCL_INSTALL)/usr/share/ti/opencl/bin/x86/clocl),)
            CLOCL=$(TI_OCL_INSTALL)/usr/share/ti/opencl/bin/x86/clocl
        else
            ifneq ($(wildcard $(HOST_ROOTDIR)/usr/bin/clocl),)
                CLOCL=$(HOST_ROOTDIR)/usr/bin/clocl
            else
                CLOCL=clocl
            endif
        endif
    endif

    ifneq ($(shell which $(firstword $(CLOCL)) >/dev/null 2>/dev/null; echo $$?),0)
        $(error CLOCL $(CLOCL) not found. Set PSDK_LINUX to point to the target Linux PSDK for automatic configuration, set HOST_ROOTDIR to the x86_64 Linux devkit, or add the clocl bin directory to PATH.)
    endif

    # --------------------------------------------------------------------------
    # cl6x

    ifeq ($(CL6X),)
        ifneq ($(wildcard $(TI_OCL_CGT_INSTALL)/bin/cl6x),)
            CL6X_BIN=$(TI_OCL_CGT_INSTALL)/bin
            CL6X=$(TI_OCL_CGT_INSTALL)/bin/cl6x
        else
            ifneq ($(wildcard $(HOST_ROOTDIR)/usr/bin/cl6x),)
                CL6X_BIN=$(HOST_ROOTDIR)/usr/bin
                CL6X=$(HOST_ROOTDIR)/usr/bin/cl6x
            else
                CL6X=cl6x
            endif
        endif
    endif

    ifneq ($(shell which $(firstword $(CL6X)) >/dev/null 2>/dev/null; echo $$?),0)
        $(error $(CL6X) not found. Set PSDK_LINUX to point to the target Linux PSDK for automatic configuration, set HOST_ROOTDIR to the x86_64 Linux devkit, or set TI_OCL_CGT_INSTALL to a TI C6x CGT installation.)
    endif

    # --------------------------------------------------------------------------

    # gcc ARM cross compiler will not, by default, search the host's
    # /usr/include.  Explicitly specify here to find dependent vendor headers
    CXXFLAGS += -I$(TARGET_ROOTDIR)/usr/include -idirafter /usr/include

    # If cross-compiling, provide path to dependent ARM libraries on the
    # target filesystem
    LDFLAGS += -L$(TARGET_ROOTDIR)/lib -L$(TARGET_ROOTDIR)/usr/lib -Wl,-rpath-link,$(TARGET_ROOTDIR)/lib -Wl,-rpath-link,$(TARGET_ROOTDIR)/usr/lib

else
    # --------------------------------------------------------------------------
    # target native compilation

    CXX      = g++
    CXXFLAGS += -I$(TI_OCL_INSTALL)/usr/include
    LDFLAGS  += -L$(TI_OCL_INSTALL)/usr/lib
    LIBS     += -lbfd

    # --------------------------------------------------------------------------
    # clocl

    ifeq ($(CLOCL),)
        ifneq ($(wildcard $(TI_OCL_INSTALL)/usr/bin/clocl),)
            CLOCL=$(TI_OCL_INSTALL)/usr/bin/clocl
        else
            CLOCL=clocl
        endif
    endif

    ifneq ($(shell which $(firstword $(CLOCL)) >/dev/null 2>/dev/null; echo $$?),0)
        $(error CLOCL $(CLOCL) not found. Set TI_OCL_INSTALL to a valid TI OpenCL installation prefix.)
    endif

    # --------------------------------------------------------------------------
    # cl6x

    ifeq ($(CL6X),)
        ifneq ($(wildcard $(TI_OCL_CGT_INSTALL)/bin/cl6x),)
            CL6X_BIN=$(TI_OCL_CGT_INSTALL)/bin
            CL6X=$(TI_OCL_CGT_INSTALL)/bin/cl6x
        else
            CL6X=cl6x
        endif
    endif

    ifneq ($(shell which $(firstword $(CL6X)) >/dev/null 2>/dev/null; echo $$?),0)
        $(error $(CL6X) not found. Set TI_OCL_CGT_INSTALL to a TI C6x CGT installation.)
    endif
endif

# --------------------------------------------------------------------------
# RTOS components
# Used by persistent_* examples

ifeq ($(USE_RTOS),1)
    ifeq ($(PSDK_RTOS),)
        tmp_NEED_RTOS=
        ifeq ($(XDC_DIR),)
            tmp_NEED_RTOS += XDC_DIR
        endif

        ifneq ($(tmp_NEED_RTOS),)
            $(error Environment variable PSDK_RTOS is not defined; set PSDK_RTOS to point at the target RTOS PSDK for automatic configuration or set $(tmp_NEED_RTOS) to the corresponding package directories)
        endif
    endif

    XDC_DIR   ?= $(wildcard $(PSDK_RTOS)/xdc*)/packages
    BIOS_DIR  ?= $(TARGET_ROOTDIR)/usr/share/ti/ti-sysbios-tree/packages
    IPC_DIR   ?= $(TARGET_ROOTDIR)/usr/share/ti/ti-ipc-tree/packages

    DSP_INCLUDE += -I$(BIOS_DIR) -I$(XDC_DIR) -I$(IPC_DIR)
endif

endif

# --------------------------------------------------------------------------

# clocl executes "cl6x" directly; make sure it uses the selected CGT
ifneq ($(CL6X_BIN),)
    CLOCL:=env PATH=$(CL6X_BIN):$(PATH) $(CLOCL)
endif

ifneq ($(TI_OCL_CGT_INSTALL),)
    DSP_INCLUDE += -I$(TI_OCL_CGT_INSTALL)/include
endif

DSP_INCLUDE += -I$(TI_OCL_INSTALL)/usr/share/ti/opencl
DSP_INCLUDE += -I$(TARGET_ROOTDIR)/usr/share/ti/opencl
DSP_INCLUDE += -I$(TARGET_ROOTDIR)/usr/share/ti/cgt-c6x/include

CL6X += -mv6600 --abi=eabi $(DSP_INCLUDE)

LIBS  = -lOpenCL -locl_util

%.o: %.cpp
	@echo Compiling $<
	$(CXX) -c $(CXXFLAGS) $<

%.o: %.c
	@echo Compiling $<
	$(CXX) -c $(CXXFLAGS) $<

%.obj: %.c
	@echo Compiling $<
	$(CL6X) -c $(CL6XFLAGS) $<

%.out: %.cl
	@echo Compiling $<
	$(CLOCL) $(CLOCLFLAGS) $^

%.dsp_h: %.cl
	@echo Compiling $<
	$(CLOCL) -t $(CLOCLFLAGS) $^

$(EXE):

clean::
	@$(RM) -f $(EXE) *.o *.obj *.out *.asm *.if *.opt *.bc *.objc *.map *.bin *.dsp_h

test: clean $(EXE)
	@echo Running   $(EXE)
	@./$(EXE) >/dev/null || echo "FAILED!!!"
