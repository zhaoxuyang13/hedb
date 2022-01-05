
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
	cd $(BENCHMARK_DIR) && psql -U postgres -d test -f db_schemas/tpch-schema-encrypted.sql 
	cd $(BENCHMARK_DIR) && psql -U postgres -d test -f db_schemas/tpch-index.sql 
	cd $(BENCHMARK_DIR) && java -Dlog4j.configuration=log4j.properties -jar bin/tpch.jar -b tpch -o output -s 100 --config config/tpch_config.xml --load true --execute false

load-tpch-native:
	cd $(BENCHMARK_DIR) && psql -U postgres -d test -f db_schemas/tpch-schema.sql 
	cd $(BENCHMARK_DIR) && psql -U postgres -d test -f db_schemas/
tpch-index.sql 
	cd $(BENCHMARK_DIR) && java -Dlog4j.configuration=log4j.properties -jar bin/tpch.jar -b tpch -o output -s 100 --config config/tpch_config.xml --load true --execute false

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