SRCFILES = ./main.cc

FMT = -Ivendor/fmt/include/ -Lvendor/fmt/build/ -lfmt
PATHARGS = ${FMT}

build:
	mkdir -p build
	g++ -o build/tqgen -std=c++2a -g -Wall  ${SRCFILES} ${PATHARGS} -lboost_program_options

.PHONY: build
