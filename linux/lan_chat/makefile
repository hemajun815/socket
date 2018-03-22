cc = g++ -std=c++11
server = server.out
server_src = ./public/cjson.cc ./public/protocol.cc ./server/linked_list.cc ./server/fifo_queue.cc ./server/server.cc
client = client.out
client_src = ./public/cjson.cc ./public/protocol.cc ./client/client.cc

all: $(server) $(client)

$(server): $(server_src)
	$(cc) $(server_src) -o $(server) -lpthread

$(client): $(client_src)
	$(cc) $(client_src) -o $(client)

clean:
	rm $(server)