INCLUDE_PATH := /usr/x86_64-w64-mingw32/include

CXX := x86_64-w64-mingw32-g++
CXXFLAGS := -std=c++20 -I ${INCLUDE_PATH} -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wpedantic -Werror
LDFLAGS := -L/usr/x86_64-w64-mingw32/lib
LDLIBS := -lws2_32
# -lws2_32 -lkernel32 -luser32 -lcomctl32 -lgdi32 -lwsock32

SOURCE := main.cpp

# =  - Allows recursive expansion
# $@ - Target
# $? - All prereqs newer than target
# $^ - All prereqs
# |  - Order only prereqs on the right (Not re-built if changed)
all: $(SOURCE)
	@echo Building $@.exe...
	$(CXX) $(CXXFLAGS) $? -o main.exe $(LDFLAGS) $(LDLIBS)

.PHONY : clean
clean :
	@echo Not implemented.