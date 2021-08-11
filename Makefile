CFLAGS = -std=c++11 -g -Wall -Wextra -lm

C_FILES = $(find httpcpp/*.cpp)

all: http_server run

http_server: $(C_FILES)
	g++ httpcpp/*.cpp $(CFLAGS) -o build/server

run: http_server
	rm -r ./build/server.dSYM && sudo ./build/server
