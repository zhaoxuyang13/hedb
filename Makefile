
.PHONY: all build configure run test clean install

all: build

build: configure
	cmake --build build

configure:
	cmake -B build -S ./src

install:
	sudo cmake --install build

run:
	echo "run not impl"

test:
	echo "test not impl"

clean:
	rm -rf build