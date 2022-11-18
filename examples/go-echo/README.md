# Go Echo Example

A simple example demonstrating how to send a flatbuffers over the network in Go.

## Generate flatbuffer code

```
flatc -g --gen-object-api --go-module-name echo hero.fbs net.fbs
```

## Running example

1. Start a server
```
go run server/server.go
```

2. Run the client in another terminal
```
go run client/client.go
```

