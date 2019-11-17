SRCFILES = ./main.cc

build:
	mkdir -p build
	${env} g++ -o build/tqgen -std=c++17 -O2 ${SRCFILES} -lboost_program_options

.PHONY: build
