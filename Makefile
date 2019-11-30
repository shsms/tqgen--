SRCFILES = ./main.cc

FMT = -Ivendor/fmt/include/ -Lvendor/fmt/build/ -lfmt
PATHARGS = ${FMT}

bootstrap:
	git submodule update --init --recursive
	mkdir -p vendor/fmt/build
	cd vendor/fmt/build && 	cmake ..
	make -C vendor/fmt/build

native:
	mkdir -p build
	g++ -o build/tqgen -std=c++17 -mtune=native -march=native  -O2 -Wall  ${SRCFILES} ${PATHARGS} -lboost_program_options

build:
	mkdir -p build
	g++ -o build/tqgen -std=c++17 -O2 -Wall  ${SRCFILES} ${PATHARGS} -lboost_program_options

debug-build:
	mkdir -p build
	g++ -o build/tqgen -std=c++2a -g -pg -Wall  ${SRCFILES} ${PATHARGS} -lboost_program_options

.PHONY: build
