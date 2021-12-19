CXXFLAGS ?= -I../../../include
LDFLAGS ?=

.PHONY: all
all: server client

greeter_generated: greeter.fbs
	flatc --grpc --cpp $<

server: greeter_generated server.cpp greeter.grpc.fb.cc greeter.grpc.fb.h
	g++ -std=c++11 -O2 $(CXXFLAGS) $(LDFLAGS) -lgpr -lgrpc -lgrpc++ server.cpp greeter.grpc.fb.cc -o $@

client: greeter_generated client.cpp greeter.grpc.fb.cc greeter.grpc.fb.h
	g++ -std=c++11 -O2 $(CXXFLAGS) $(LDFLAGS) -lgpr -lgrpc -lgrpc++ client.cpp greeter.grpc.fb.cc -o $@
