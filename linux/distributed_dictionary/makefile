cc = g++

all: aws client

bin:
	test -d bin || mkdir -p bin

aws: bin aws.cc
	$(cc) aws.cc -o bin/aws.out

client: bin client.cc
	$(cc) client.cc -o bin/client.out

clean:
	rm -rf bin
