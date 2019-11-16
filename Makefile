SRCFILES = ./main.cc

build:
	mkdir -p build
	g++ -o build/tqgen -lboost_program_options ${SRCFILES}

.PHONY: build
