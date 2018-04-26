BUILDTYPE ?= Release

CXX := $(CXX)
CXXFLAGS := $(CXXFLAGS) -std=c++11 -fvisibility=hidden
LDFLAGS := $(LDFLAGS) -lz -lpthread -lboost_program_options -lboost_filesystem -lboost_system
WARNING_FLAGS := -Wall -Wextra -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wlong-long -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual

MASON_HOME := ./mason_packages/.link

RELEASE_FLAGS := -O3 -DNDEBUG
DEBUG_FLAGS := -O0 -DDEBUG -fno-inline-functions -fno-omit-frame-pointer

ifeq ($(BUILDTYPE),Release)
	FINAL_FLAGS := -g $(WARNING_FLAGS) $(RELEASE_FLAGS)
else
	FINAL_FLAGS := -g $(WARNING_FLAGS) $(DEBUG_FLAGS)
endif

all: osm-tiler

mason_packages:
	./bootstrap.sh

osm-tiler: mason_packages handler.hpp osm-tiler.cpp
	$(CXX) osm-tiler.cpp -o osm-tiler -isystem$(MASON_HOME)/include -L$(MASON_HOME)/lib $(CXXFLAGS) $(FINAL_FLAGS) $(LDFLAGS);

chs.osm.pbf:
	curl ###TODO: ADD VALID EXTRACT### -o chs.osm.pbf

test: chs.osm.pbf osm-tiler
	./osm-tiler -z 9 -o ./output chs.osm.pbf;

clean:
	rm -f osm-tiler; rm -rf output; mkdir output;

clean-mason:
	rm -rf ./mason_packages;
