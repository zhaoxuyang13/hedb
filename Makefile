include config.mk 

.PHONY: all build configure run test clean install load-tpch load-tpch-native

all: build

build: configure
	cmake --build build 

configure:
	cmake -B build -S ./src -DTEE_TYPE=SGX



install: 
	sudo cmake --install build

BENCHMARK_DIR=benchmark

load-tpcc: 
	cd $(BENCHMARK_DIR) && psql -h ${PG_SERVER_IP} -p ${PG_SERVER_PORT} -U postgres -d test -f db_schemas/tpcc-schema_encrypted.sql 
	cd $(BENCHMARK_DIR) && java -Dlog4j.configuration=log4j.properties -jar bin/oltp.jar -b tpcc -o output -s 20 --config config/tpcc_config.xml --load true --execute false

load-tpcc-native: 
	cd $(BENCHMARK_DIR) && psql -h ${PG_SERVER_IP} -p ${PG_SERVER_PORT} -U postgres -d test -f db_schemas/tpcc-schema.sql 
	cd $(BENCHMARK_DIR) && java -Dlog4j.configuration=log4j.properties -jar bin/oltp.jar -b tpcc -o output -s 20 --config config/tpcc_config.xml --load true --execute false

load-tpch: 
	cd $(BENCHMARK_DIR) && psql -h ${PG_SERVER_IP} -p ${PG_SERVER_PORT} -U postgres -d test -f db_schemas/tpch-schema-encrypted.sql 
	cd $(BENCHMARK_DIR) && psql -h ${PG_SERVER_IP} -p ${PG_SERVER_PORT} -U postgres -d test -f db_schemas/tpch-index.sql 
	cd $(BENCHMARK_DIR) && java -Dlog4j.configuration=log4j.properties -jar bin/tpch.jar -b tpch -o output -s 100 --config config/tpch_config.xml --load true --execute false

load-tpch-native: 
	cd $(BENCHMARK_DIR) && psql -h ${PG_SERVER_IP} -p ${PG_SERVER_PORT} -U postgres -d test -f db_schemas/tpch-schema.sql 
	cd $(BENCHMARK_DIR) && psql -h ${PG_SERVER_IP} -p ${PG_SERVER_PORT} -U postgres -d test -f db_schemas/tpch-index.sql 
	cd $(BENCHMARK_DIR) && java -Dlog4j.configuration=log4j.properties -jar bin/tpch.jar -b tpch -o output -s 100 --config config/tpch_config.xml --load true --execute false

run:
	echo "run not impl"

test:
	echo "test not impl"

clean:
	rm -rf build
	make clean -C src/enclave/trustzone