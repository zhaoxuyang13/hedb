include config.mk 

.PHONY: all build configure run test clean install load-tpch load-tpch-native gen-tpch

all: build

build: 
	cmake --build build

BUILDDIR := build
TYPES := int text timestamp
SOURCES :=  ${addsuffix _ops.c, ${addprefix plain_, ${TYPES}}}
SOURCE_DIR := src/enclave/plain_ops

BC_OBJECTS := ${addprefix ${BUILDDIR}/, $(SOURCES:%.c=%.bc)} 


CLANG_FLAGS := -I src/include -I src/enclave/include -I src/enclave -emit-llvm -c -g

${BUILDDIR}/%.bc: ${SOURCE_DIR}/%.c
	mkdir -p build
	@echo from file $@ $<
	clang ${CLANG_FLAGS} $< -o $@

klee_wrapper: ${BC_OBJECTS}
	clang ${CLANG_FLAGS} src/enclave/like_match.c -o ${BUILDDIR}/like_match.bc
	clang ${CLANG_FLAGS} src/enclave/klee_wrapper/wrapper.c -o ${BUILDDIR}/wrapper.bc
	clang ${CLANG_FLAGS} -I src/enclave/klee_wrapper src/enclave/klee_wrapper/prefix_udf.c -o ${BUILDDIR}/prefix_udf.bc
	llvm-link ${BUILDDIR}/wrapper.bc ${BUILDDIR}/like_match.bc ${BC_OBJECTS} ${BUILDDIR}/prefix_udf.bc  -o ${BUILDDIR}/whole.bc

configure_sgx:
	cmake -B build -S ./src -DTEE_TYPE=SGX

configure_tz:
	cmake -B build -S ./src -DTEE_TYPE=TZ

configure_sim:
	cmake -B build -S ./src -DTEE_TYPE=SIM

configure_sim_parallel:
	cmake -B build -S ./src -DTEE_TYPE=SIM -DENABLE_PARALLEL=ON
 
install: 
	sudo cmake --install build

load-tpcc: 
	cd benchmark && psql -h ${PG_SERVER_IP} -p ${PG_SERVER_PORT} -U postgres -d test -f db_schemas/tpcc-schema_encrypted.sql 
	cd benchmark && java -Dlog4j.configuration=log4j.properties -jar bin/oltp.jar -b tpcc -o output -s 20 --config config/tpcc_config.xml --load true --execute false

load-tpcc-native: 
	cd benchmark && psql -h ${PG_SERVER_IP} -p ${PG_SERVER_PORT} -U postgres -d test -f db_schemas/tpcc-schema.sql 
	cd benchmark && java -Dlog4j.configuration=log4j.properties -jar bin/oltp.jar -b tpcc -o output -s 20 --config config/tpcc_config.xml --load true --execute false


# take a command line argument s for scale factor
s?=1
gen-tpch:
	cd benchmark/tools && ./dbgen -vf  -s $(s)

load-tpch: 
	cd benchmark && bash load_db.sh 
load-tpch-native: 
	cd benchmark && bash_load_db.sh -p
run:
	echo "run not impl"

test:
	echo "test not impl"

clean:
	rm -rf build
	make clean -C src/enclave/trustzone
