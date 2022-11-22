# Go Echo Example

A simple example demonstrating how to send flatbuffers over the network in Go.

## Generate flatbuffer code

```
flatc -g --gen-object-api --go-module-name echo hero.fbs net.fbs
```

## Running example

1. Run go mod tidy to get dependencies
```
go mod tidy
```

2. Start a server
```
go run server/server.go
```

3. Run the client in another terminal
```
go run client/client.go
```

