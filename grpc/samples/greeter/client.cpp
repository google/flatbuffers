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

  std::string SayHello(const std::string &name) {
    auto name_offset = mb_.CreateString(name);
    auto request_offset = CreateHelloRequest(mb_, name_offset);
    mb_.Finish(request_offset);
    auto request_msg = mb_.ReleaseMessage<HelloRequest>();

    flatbuffers::grpc::Message<HelloReply> response_msg;

    grpc::ClientContext context;

    auto status = stub_->SayHello(&context, request_msg, &response_msg);
    if (status.ok()) {
      const HelloReply *response = response_msg.GetRoot();
      return response->message()->str();
    } else {
      std::cerr << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  void SayManyHellos(const std::string &name, int num_greetings,
                     std::function<void(const std::string &)> callback) {
    auto name_offset = mb_.CreateString(name);
    auto request_offset =
        CreateManyHellosRequest(mb_, name_offset, num_greetings);
    mb_.Finish(request_offset);
    auto request_msg = mb_.ReleaseMessage<ManyHellosRequest>();

    flatbuffers::grpc::Message<HelloReply> response_msg;

    grpc::ClientContext context;

    auto stream = stub_->SayManyHellos(&context, request_msg);
    while (stream->Read(&response_msg)) {
      const HelloReply *response = response_msg.GetRoot();
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

int main(int argc, char **argv) {
  std::string server_address("localhost:50051");

  auto channel =
      grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
  GreeterClient greeter(channel);

  std::string name("world");

  std::string message = greeter.SayHello(name);
  std::cerr << "Greeter received: " << message << std::endl;

  int num_greetings = 10;
  greeter.SayManyHellos(name, num_greetings, [](const std::string &message) {
    std::cerr << "Greeter received: " << message << std::endl;
  });

  return 0;
}
