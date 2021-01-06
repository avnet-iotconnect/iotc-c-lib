CC = gcc
CXX = g++
CFLAGS = -O0 -g -Wall -Wextra -std=c99 -pedantic-errors
CXXFLAGS = -O0 -g -Wall -Wextra -pedantic-errors

CJSON_EXTRAS = -I../include -I./cJSON/ ./cJSON/cJSON.c
EVENT_SOURCES = event.c ../src/iotconnect_event.c ../src/iotconnect_common.c ../src/iotconnect_lib.c
TELEMETRY_SOURCES = telemetry.c ../src/iotconnect_telemetry.c ../src/iotconnect_common.c ../src/iotconnect_lib.c

all: test-event test-telemetry test-telemetry-cpp test-event-cpp
.PHONY: setup test-event test-telemetry
test-event: setup $(EVENT_SOURCES)
	$(CC) $(CFLAGS) -o $@  $(EVENT_SOURCES) $(CJSON_EXTRAS)

test-telemetry: $(TELEMETRY_SOURCES)
	$(CC) $(CFLAGS) -o $@  $(TELEMETRY_SOURCES) $(CJSON_EXTRAS)

test-event-cpp: setup $(EVENT_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@  $(EVENT_SOURCES) $(CJSON_EXTRAS)

test-telemetry-cpp: $(TELEMETRY_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@  $(TELEMETRY_SOURCES) $(CJSON_EXTRAS)

clean:
	rm -f test-* test-* *.o

