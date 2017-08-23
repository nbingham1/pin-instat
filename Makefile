PIN_ROOT = /opt/pin-3.2-81205-gcc-linux
DEFINES = -DBIGARRAY_MULTIPLIER=1 -D__PIN__=1 -DPIN_CRT=1 -DTARGET_IA32E -DHOST_IA32E -DTARGET_LINUX
WARNINGS = -Wall -Werror -Wno-unknown-pragmas 
FLAGS = -fno-stack-protector -fno-exceptions -funwind-tables -fasynchronous-unwind-tables -fno-rtti -fPIC -fabi-version=2 -O3 -fomit-frame-pointer -fno-strict-aliasing -std=c++11
INCLUDE_PATHS = -I$(PIN_ROOT)/source/include/pin -I$(PIN_ROOT)/source/include/pin/gen -isystem $(PIN_ROOT)/extras/stlport/include -isystem $(PIN_ROOT)/extras/libstdc++/include -isystem $(PIN_ROOT)/extras/crt/include -isystem $(PIN_ROOT)/extras/crt/include/arch-x86_64 -isystem $(PIN_ROOT)/extras/crt/include/kernel/uapi -isystem $(PIN_ROOT)/extras/crt/include/kernel/uapi/asm-x86 -I$(PIN_ROOT)/extras/components/include -I$(PIN_ROOT)/extras/xed-intel64/include/xed -I$(PIN_ROOT)/source/tools/InstLib

LIBRARY_PATHS = -L$(PIN_ROOT)/intel64/runtime/pincrt -L$(PIN_ROOT)/intel64/lib -L$(PIN_ROOT)/intel64/lib-ext -L$(PIN_ROOT)/extras/xed-intel64/lib
LIBRARIES = -lpin -lxed $(PIN_ROOT)/intel64/runtime/pincrt/crtendS.o -lpin3dwarf -ldl-dynamic -nostdlib -lstlport-dynamic -lm-dynamic -lc-dynamic -lunwind-dynamic
LINK_FLAGS = -shared -Wl,--hash-style=sysv $(PIN_ROOT)/intel64/runtime/pincrt/crtbeginS.o -Wl,-Bsymbolic -Wl,--version-script=$(PIN_ROOT)/source/include/pin/pintool.ver -fabi-version=2

SOURCES = $(shell find src -name "*.cpp")
OBJECTS = $(SOURCES:src/%.cpp=obj/%.o)
TARGET = instat.so

all: obj $(TARGET)

obj:
	mkdir -p obj

$(TARGET): $(OBJECTS)
	g++ $(LINK_FLAGS) -o $@ $^ $(LIBRARY_PATHS) $(LIBRARIES)

obj/%.o: src/%.cpp
	g++ $(DEFINES) $(WARNINGS) $(FLAGS) $(INCLUDE_PATHS) -c -o $@ $<

analyze: tools/analyze.cpp $(filter-out src/instat.cpp,$(SOURCES))
	g++ -DTARGET_IA32E -DHOST_IA32E -DTARGET_LINUX $(LIBRARY_PATHS) -I. -I$(PIN_ROOT)/source/include/pin -I$(PIN_ROOT)/source/include/pin/gen -I$(PIN_ROOT)/extras/components/include -I$(PIN_ROOT)/extras/xed-intel64/include/xed -I$(PIN_ROOT)/source/tools/InstLib $^ -o analyze -lxed

clean:
	rm -rf obj $(TARGET) analyze *.tsv
