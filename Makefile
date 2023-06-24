
.PHONY: all build configure run test clean install load-tpch

all: build

build: configure
	cmake --build build

configure:
	cmake -B build -S ./src

install: 
	sudo cmake --install build

BENCHMARK_DIR=benchmark

load-tpch: 
	cd $(BENCHMARK_DIR) && bash load_db.sh
load-tpch-native:
	cd $(BENCHMARK_DIR) && bash load_db.sh -p
load-tpcc-native: 
	echo "not impl"

load-tpcc: 
	echo "not impl"

run:
	echo "run not impl"

test:
	echo "test not impl"

clean:
	rm -rf build