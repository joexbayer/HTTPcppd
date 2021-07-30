CFLAGS = -std=c++11 -g -Wall -Wextra -lm

C_FILES = $(find httpcpp/*.cpp)

all: compile run

compile: $(C_FILES)
	g++ httpcpp/*.cpp $(CFLAGS) -o server

run: compile
	rm -r ./server.dSYM && sudo ./server
