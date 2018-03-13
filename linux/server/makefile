target = server.out
cc = g++
lib = -lpthread
source = ./server.cc

$(target):$(source)
	$(cc) $(source) -o $(target) $(lib)

clean: $(target)
	rm $(target)