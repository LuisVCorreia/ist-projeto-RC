# Makefile for the AS application

CC = g++
CXX = g++
LD = g++

INCLUDE_DIRS := src/client src/server src/
INCLUDES = $(addprefix -I, $(INCLUDE_DIRS))

TARGETS = src/client/User src/server/Server
TARGET_EXECS = user AS

CLIENT_SOURCES := $(wildcard src/client/*.cpp)
COMMON_SOURCES := $(wildcard src/common/*.cpp)
SERVER_SOURCES := $(wildcard src/server/*.cpp)
SOURCES := $(CLIENT_SOURCES) $(COMMON_SOURCES) $(SERVER_SOURCES)

CLIENT_HEADERS := $(wildcard src/client/*.hpp)
COMMON_HEADERS := $(wildcard src/common/*.hpp)
SERVER_HEADERS := $(wildcard src/server/*.hpp)
HEADERS := $(CLIENT_HEADERS) $(COMMON_HEADERS) $(SERVER_HEADERS)

CLIENT_OBJECTS := $(CLIENT_SOURCES:.cpp=.o)
COMMON_OBJECTS := $(COMMON_SOURCES:.cpp=.o)
SERVER_OBJECTS := $(SERVER_SOURCES:.cpp=.o)
OBJECTS := $(CLIENT_OBJECTS) $(COMMON_OBJECTS) $(SERVER_OBJECTS)

CXXFLAGS = -std=c++17
LDFLAGS = -std=c++17

CXXFLAGS += $(INCLUDES)
LDFLAGS += $(INCLUDES)

vpath # clears VPATH
vpath %.hpp $(INCLUDE_DIRS)

# Run `make OPTIM=no` to disable -O3
ifeq ($(strip $(OPTIM)), no)
	CXXFLAGS += -O0
else
	CXXFLAGS += -O3
endif

# Run `make DEBUG=true` to run with debug symbols
ifeq ($(strip $(DEBUG)), yes)
	CXXFLAGS += -g
endif

CXXFLAGS += -fdiagnostics-color=always
CXXFLAGS += -Wall
CXXFLAGS += -Werror
CXXFLAGS += -Wextra
CXXFLAGS += -Wcast-align
CXXFLAGS += -Wconversion
CXXFLAGS += -Wfloat-equal
CXXFLAGS += -Wformat=2
CXXFLAGS += -Wnull-dereference
CXXFLAGS += -Wshadow
CXXFLAGS += -Wsign-conversion
CXXFLAGS += -Wswitch-default
CXXFLAGS += -Wswitch-enum
CXXFLAGS += -Wundef
CXXFLAGS += -Wunreachable-code
CXXFLAGS += -Wunused


.PHONY: all clean fmt fmt-check package

all: $(TARGET_EXECS)

fmt: $(SOURCES) $(HEADERS)
	clang-format -i $^

fmt-check: $(SOURCES) $(HEADERS)
	clang-format -n --Werror $^

src/server/Server: $(SERVER_OBJECTS) $(SERVER_HEADERS) $(COMMON_OBJECTS) $(COMMON_HEADERS)
src/client/User: $(CLIENT_OBJECTS) $(CLIENT_HEADERS) $(COMMON_OBJECTS) $(COMMON_HEADERS)

AS: src/server/Server
	cp src/server/Server AS
user: src/client/User
	cp src/client/User user

clean:
	rm -f $(OBJECTS) $(TARGETS) $(TARGET_EXECS)

