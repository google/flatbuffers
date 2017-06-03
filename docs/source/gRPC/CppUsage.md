Use in C++    {#flatbuffers_grpc_guide_use_cpp}
==========

## Before you get started

Before diving into the FlatBuffers gRPC usage in C++, you should already be
familiar with the following:

- FlatBuffers as a serialization format
- [gRPC](http://www.grpc.io/docs/) usage

## Using the FlatBuffers gRPC C++ library

NOTE: The examples below are also in the `examples/grpc/greeter` directory.

We will illustrate usage with the following schema:

    table HelloReply {
      message:string;
    }

    table HelloRequest {
      name:string;
    }

    table ManyHellosRequest {
      name:string;
      num_greetings:int;
    }

    rpc_service Greeter {
      SayHello(HelloRequest):HelloReply;
      SayManyHellos(ManyHellosRequest):HelloReply (streaming: "server");
    }

When we run `flatc`, we pass in the `--grpc` option and generage an additional
`greeter.grpc.fb.h` and `greeter.grpc.fb.cc`.

Example server code looks like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    #include "greeter.grpc.fb.h"
    #include "greeter_generated.h"

    #include <grpc++/grpc++.h>

    #include <iostream>
    #include <memory>
    #include <string>

    class GreeterServiceImpl final : public Greeter::Service {
      virtual grpc::Status SayHello(
          grpc::ServerContext *context,
          const flatbuffers::grpc::Message<HelloRequest> *request_msg,
          flatbuffers::grpc::Message<HelloReply> *response_msg) override {
        // flatbuffers::grpc::MessageBuilder mb_;
        // We call GetRoot to "parse" the message. Verification is already
        // performed by default. See the notes below for more details.
        const HelloRequest *request = request_msg->GetRoot();

        // Fields are retrieved as usual with FlatBuffers
        const std::string &name = request->name()->str();

        // `flatbuffers::grpc::MessageBuilder` is a `FlatBufferBuilder` with a
        // special allocator for efficient gRPC buffer transfer, but otherwise
        // usage is the same as usual.
        auto msg_offset = mb_.CreateString("Hello, " + name);
        auto hello_offset = CreateHelloReply(mb_, msg_offset);
        mb_.Finish(hello_offset);

        // The `ReleaseMessage<T>()` function detaches the message from the
        // builder, so we can transfer the resopnse to gRPC while simultaneously
        // detaching that memory buffer from the builer.
        *response_msg = mb_.ReleaseMessage<HelloReply>();
        assert(response_msg->Verify());

        // Return an OK status.
        return grpc::Status::OK;
      }

      virtual grpc::Status SayManyHellos(
          grpc::ServerContext *context,
          const flatbuffers::grpc::Message<ManyHellosRequest> *request_msg,
          grpc::ServerWriter<flatbuffers::grpc::Message<HelloReply>> *writer)
          override {
        // The streaming usage below is simply a combination of standard gRPC
        // streaming with the FlatBuffers usage shown above.
        const ManyHellosRequest *request = request_msg->GetRoot();
        const std::string &name = request->name()->str();
        int num_greetings = request->num_greetings();

        for (int i = 0; i < num_greetings; i++) {
          auto msg_offset = mb_.CreateString("Many hellos, " + name);
          auto hello_offset = CreateHelloReply(mb_, msg_offset);
          mb_.Finish(hello_offset);
          writer->Write(mb_.ReleaseMessage<HelloReply>());
        }

        return grpc::Status::OK;
      }

      flatbuffers::grpc::MessageBuilder mb_;
    };

    void RunServer() {
      std::string server_address("0.0.0.0:50051");
      GreeterServiceImpl service;

      grpc::ServerBuilder builder;
      builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
      builder.RegisterService(&service);
      std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
      std::cerr << "Server listening on " << server_address << std::endl;

      server->Wait();
    }

    int main(int argc, const char *argv[]) {
      RunServer();
      return 0;
    }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Example client code looks like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    #include "greeter.grpc.fb.h"
    #include "greeter_generated.h"

    #include <grpc++/grpc++.h>

    #include <iostream>
    #include <memory>
    #include <string>

    class GreeterClient {
     public:
      GreeterClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(Greeter::NewStub(channel)) {}

      std::string SayHello(const std::string& name) {
        auto name_offset = mb_.CreateString(name);
        auto request_offset = CreateHelloRequest(mb_, name_offset);
        mb_.Finish(request_offset);
        auto request_msg = mb_.ReleaseMessage<HelloRequest>();

        flatbuffers::grpc::Message<HelloReply> response_msg;

        grpc::ClientContext context;

        auto status = stub_->SayHello(&context, request_msg, &response_msg);
        if (status.ok()) {
          const HelloReply* response = response_msg.GetRoot();
          return response->message()->str();
        } else {
          std::cerr << status.error_code() << ": " << status.error_message()
                    << std::endl;
          return "RPC failed";
        }
      }

      template <class F>
      void SayManyHellos(const std::string& name, int num_greetings, F&& callback) {
        auto name_offset = mb_.CreateString(name);
        auto request_offset =
            CreateManyHellosRequest(mb_, name_offset, num_greetings);
        mb_.Finish(request_offset);
        auto request_msg = mb_.ReleaseMessage<ManyHellosRequest>();

        flatbuffers::grpc::Message<HelloReply> response_msg;

        grpc::ClientContext context;

        auto stream = stub_->SayManyHellos(&context, request_msg);
        while (stream->Read(&response_msg)) {
          const HelloReply* response = response_msg.GetRoot();
          callback(response->message()->str());
        }
        auto status = stream->Finish();
        if (!status.ok()) {
          std::cerr << status.error_code() << ": " << status.error_message()
                    << std::endl;
          callback("RPC failed");
        }
      }

     private:
      std::unique_ptr<Greeter::Stub> stub_;
      flatbuffers::grpc::MessageBuilder mb_;
    };

    int main(int argc, char** argv) {
      std::string server_address("localhost:50051");

      auto channel =
          grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
      GreeterClient greeter(channel);

      std::string name("world");

      std::string message = greeter.SayHello(name);
      std::cerr << "Greeter received: " << message << std::endl;

      int num_greetings = 10;
      greeter.SayManyHellos(name, num_greetings, [](const std::string& message) {
        std::cerr << "Greeter received: " << message << std::endl;
      });

      return 0;
    }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
