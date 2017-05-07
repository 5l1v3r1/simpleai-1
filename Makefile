-include Makefile.local

# Building
## Release or Debug
#
#  make BUILD_TYPE=Debug
#  make BUILD_TYPE=Release
#
# To build either in debug mode, or in release mode
#
# Debugging
## Run in debugger
#
#  make tests DEBUG=1 ARGS=--gtest_filter=World*
#
# to only build the tests targets, and run it via gdb afterwards.
# It will automatically start the execution and forward the arguments
# that are given via ARGS to the target
#
## Verbose output
#
#  make Q=
#
# This will print out information about the commands that are executed to build
# and run the target(s)

Q                 = @
OS               := $(shell uname)

VALGRIND         ?=
ifeq ($(VALGRIND),)
VALGRIND_CMD     ?=
else
VALGRIND_CMD     ?= valgrind
endif

DEBUG            ?=
ifeq ($(DEBUG),)
DEBUG_CMD        ?=
else
DEBUGGER         := $(shell (gdb --help >/dev/null 2>&1 && echo GDB) || (lldb --help >/dev/null 2>&1 && echo LLDB))
ifeq ($(DEBUGGER),GDB)
DEBUG_CMD        ?= gdb -ex run --args
else ifeq ($(DEBUGGER),LLDB)
DEBUG_CMD        ?= lldb -b -o run
else
DEBUG_CMD        ?=
endif
endif

CMAKE_OPTIONS    ?= -DSIMPLEAI_DEBUGGER=On

GPROF            ?=
BUILD_TYPE       ?= Debug
# override this in your Makefile.local to use a different directory
BUILDDIRPATH     ?= ./
#BUILDDIR         ?= $(BUILDDIRPATH)build-$(shell echo $(BUILD_TYPE) | tr '[:upper:]' '[:lower:]')
ifeq ($(GPROF),)
BUILDDIR         ?= $(BUILDDIRPATH)build/$(BUILD_TYPE)
else
BUILDDIR         ?= $(BUILDDIRPATH)build/$(BUILD_TYPE)/gprof
CMAKE_OPTIONS    += -DUSE_GPROF=True
endif

ifneq ($(THREADS),)
BUILDDIR         ?= $(BUILDDIRPATH)build/$(BUILD_TYPE)/threads
CMAKE_OPTIONS    += -DSANITIZER_THREADS=True
endif

#VOGL_OPTIONS     ?= --vogl_force_debug_context --vogl_exit_after_x_frames 2000
VOGL_OPTIONS     ?= --vogl_force_debug_context
VOGL             ?=
ifeq ($(VOGL),)
VOGL_CMD         ?=
else
VOGL_BIN         ?= vogl
VOGL_CMD         ?= $(VOGL_BIN) trace --vogl_tracepath $(BUILDDIR) --vogl_tracefile $@.trace.bin $(VOGL_OPTIONS)
ARGS_TMP         := $(ARGS)
ARGS              = "--args $(ARGS_TMP)"
endif

MAKE_PID     := $$PPID
JOB_FLAG     := $(filter -j%, $(subst -j ,-j,$(shell ps T | grep "^\s*$(MAKE_PID).*$(MAKE)")))
MAKE_OPTIONS := --no-print-directory -C $(BUILDDIR)

ifeq ($(OS),Darwin)
CMAKE_GENERATOR ?= "Xcode"
CMAKE_BINARY    ?= /Applications/CMake.app/Contents/bin/cmake
DARWIN          := 1
else ifeq ($(OS),Linux)
CMAKE_GENERATOR ?= "Eclipse CDT4 - Unix Makefiles"
CMAKE_BINARY    ?= cmake
LINUX           := 1
else
CMAKE_GENERATOR ?= "MSYS Makefiles"
CMAKE_BINARY    ?= cmake
WINDOWS         := 1
endif
INSTALL_DIR     ?= $(BUILDDIRPATH)$(OS)

all: build

run: simpleai-run

.PHONY: clangtidy
clangtidy:
	$(Q)mkdir -p $(BUILDDIR)/tidy
	$(Q)cd $(BUILDDIR)/tidy; $(CMAKE_BINARY) -DCMAKE_CXX_CLANG_TIDY:STRING="clang-tidy-4.0;-checks=readability-uniqueptr-delete-release,readability-non-const-parameter,readability-redundant-smartptr-get,performance-unnecessary-value-param,performance-unnecessary-copy-initialization,performance-inefficient-string-concatenation,performance-implicit-cast-in-loop,performance-for-range-copy,performance-faster-string-find,modernize-make-shared,clang-analyzer-security.*;-fix" $(CURDIR) $(CMAKE_OPTIONS) && cmake --build .

.PHONY: cmake
cmake:
	$(Q)mkdir -p $(BUILDDIR)
	$(Q)cd $(BUILDDIR); $(CMAKE_BINARY) -G$(CMAKE_GENERATOR) -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CURDIR) $(CMAKE_OPTIONS)

define COMPILE
$(if $(LINUX),\
	$(Q)$(MAKE) $(MAKE_OPTIONS) $(JOB_FLAG) $(1) \
$(else),\
	$(if $(DARWIN),\
		$(Q)cd $(BUILDDIR); xcodebuild build -target $(1) install -project tests.xcodeproj -configuration $(BUILD_TYPE) CODE_SIGN_IDENTITY=\"\" CODE_SIGNING_REQUIRED=NO | tee xcodebuild.log | xcpretty && exit ${PIPESTATUS[0]} \
	$(else),\
		$(Q)$(MAKE) $(MAKE_OPTIONS) $(JOB_FLAG) $(1) \
	)
)
endef

.PHONY: build
build: cmake
	$(call COMPILE, install)

clean:
	$(Q)rm -rf $(BUILDDIR)

doc: cmake
	$(call COMPILE, $@)

simpleai-run simpleai-debugger simpleai-tests: cmake
	$(call COMPILE, $@)
	$(Q)$(VALGRIND_CMD) $(DEBUG_CMD) $(VOGL_CMD) ./$@ $(ARGS)
