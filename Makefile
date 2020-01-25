.PHONY: all debug release

all: debug

debug:
	@clang++ -W -Wall -pedantic -std=c++17 -ggdb -O0 teamsic.cpp -o hc2020 -fsanitize=address

release:
	@clang++ -W -Wall -pedantic -std=c++17 -O2 -march=native teamsic.cpp -o hc2020
