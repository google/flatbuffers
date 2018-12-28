# description

This sample code is a synchronous client and server borrowed from [here](https://github.com/grpc/grpc/tree/master/examples/cpp/helloworld).

# build

```
./make
```

# run

- client

```
./greeter_server &
Server listening on 0.0.0.0:50051
```

- server

```
./greeter_client 
Greeter received: Hello, world
Greeter received: Many hellos, world
Greeter received: Many hellos, world
Greeter received: Many hellos, world
Greeter received: Many hellos, world
Greeter received: Many hellos, world
Greeter received: Many hellos, world
Greeter received: Many hellos, world
Greeter received: Many hellos, world
Greeter received: Many hellos, world
Greeter received: Many hellos, world
```

