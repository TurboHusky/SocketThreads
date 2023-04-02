INCLUDE_PATH := ../include

CXX := g++
CXXFLAGS := -std=c++20 -isystem ${INCLUDE_PATH} -Wall -Wextra -Wpedantic -Werror
LDFLAGS := -L ./
LDLIBS := 

SOURCE := main.cpp

# =  - Allows recursive expansion
# $@ - Target
# $? - All prereqs newer than target
# $^ - All prereqs
# |  - Order only prereqs on the right (Not re-built if changed)
all: $(SOURCE)
	@echo Building $@ for Linux...
	$(CXX) $? $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o main

.PHONY : clean
clean :
	@echo Not implemented.