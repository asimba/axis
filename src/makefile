include build.inc
include version.inc

GCC_MINGW32_I686_W64_PREFIX = i686-w64-mingw32
GCC_MINGW32_I586_PREFIX = i586-mingw32msvc
GCC_TEST:=$(shell g++ --version 2>&1| head -1 | grep -G ".*\..\.")
GCC_MINGW32_I686_W64_TEST:=$(shell $(GCC_MINGW32_I686_W64_PREFIX)-g++ --version 2>&1| head -1 | grep -G ".*\.")
GCC_MINGW32_I586_TEST:=$(shell $(GCC_MINGW32_I586_PREFIX)-g++ --version 2>&1| head -1 | grep -G ".*\.")
BUILD_NEW:=$(shell expr $(BUILD_NUMBER) + 1)

all:
	@if ! test -d ../obj ; then mkdir ../obj ; fi
	@echo BUILD_NUMBER=$(BUILD_NEW) > ./build.inc
	@rm -f ./version.h
	@echo "#define VERSION \""$(VERSION_STRING) \(build  $(BUILD_NEW) `date +'%Y-%m-%d'`\)"\"" > ./version.h
	@rm -f ./hashtbl.h
	@if test -n "$(GCC_TEST)" ;\
	then make -s -f makefile.mk CONFIG_BASE=config.inc.linux ;\
	fi
	@if test -n "$(GCC_MINGW32_I686_W64_TEST)" ;\
	then make -s -f makefile.mk TARGET=$(GCC_MINGW32_I686_W64_PREFIX) CONFIG_BASE=config.inc.win32 ;\
	elif test -n "$(GCC_MINGW32_I586_TEST)" ;\
	then make -s -f makefile.mk TARGET=$(GCC_MINGW32_I586_PREFIX) CONFIG_BASE=config.inc.win32 ;\
	fi

clean:
	make -s -f makefile.mk clean CONFIG_BASE=config.inc.win32
	make -s -f makefile.mk clean CONFIG_BASE=config.inc.linux

clean-obj:
	make -s -f makefile.mk clean-obj CONFIG_BASE=config.inc.linux

.NOEXPORT:
