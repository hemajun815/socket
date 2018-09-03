cc = g++ -Wall -std=c++17 -finput-charset=UTF-8
lib = -lWS2_32
src_server = ./src/server.cc
out_server = ./server.exe
src_client = ./src/client.cc
out_client = ./client.exe

all: server client

server: $(src_server)
	$(cc) $(src_server) -o $(out_server) $(lib)

client: $(src_client)
	$(cc) $(src_client) -o $(out_client) $(lib)

clean:
	rm -f $(out_server) $(out_client)
