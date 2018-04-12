cc = g++

all: serverA aws client

bin:
	test -d bin || mkdir -p bin

serverA: bin serverA.cc
	$(cc) serverA.cc -o bin/serverA.out

aws: bin aws.cc
	$(cc) aws.cc -o bin/aws.out

client: bin client.cc
	$(cc) client.cc -o bin/client.out

clean:
	rm -rf bin
