cc = g++

all: serverA serverB serverC aws monitor client

bin:
	test -d bin || mkdir -p bin

serverA: bin serverA.cc
	$(cc) serverA.cc -o bin/serverA.out

serverB: bin serverB.cc
	$(cc) serverB.cc -o bin/serverB.out

serverC: bin serverC.cc
	$(cc) serverC.cc -o bin/serverC.out

aws: bin aws.cc
	$(cc) aws.cc -o bin/aws.out

monitor: bin monitor.cc
	$(cc) monitor.cc -o bin/monitor.out

client: bin client.cc
	$(cc) client.cc -o bin/client.out

clean:
	rm -rf bin
