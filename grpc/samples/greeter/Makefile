CXXFLAGS ?= -I../../../include
LDFLAGS ?= -L/usr/local/lib/ -L/usr/lib/

.PHONY: all
all: greeter_generated.h greeter_server greeter_client

greeter_generated.h: greeter.fbs
	flatc --grpc --cpp $<

greeter_server: server.cpp greeter.grpc.fb.cc greeter_generated.h greeter.grpc.fb.h
	g++ -std=c++11 -O2 $(CXXFLAGS) $(LDFLAGS) server.cpp greeter.grpc.fb.cc -lgpr -lgrpc -lgrpc++ -o $@

greeter_client: client.cpp greeter.grpc.fb.cc greeter_generated.h greeter.grpc.fb.h
	g++ -std=c++11 -O2 $(CXXFLAGS) $(LDFLAGS) client.cpp greeter.grpc.fb.cc -lgpr -lgrpc -lgrpc++ -o $@
