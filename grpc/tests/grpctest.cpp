/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thread>

#include <grpc++/grpc++.h>

#include "monster_test_generated.h"
#include "monster_test.grpc.fb.h"

using namespace MyGame::Example;

// The callback implementation of our server, that derives from the generated
// code. It implements all rpcs specified in the FlatBuffers schema.
class ServiceImpl final : public MyGame::Example::MonsterStorage::Service {
  virtual ::grpc::Status Store(::grpc::ServerContext* context,
                               const flatbuffers::BufferRef<Monster> *request,
                               flatbuffers::BufferRef<Stat> *response)
                               override {
    // Create a response from the incoming request name.
    fbb_.Clear();
    auto stat_offset = CreateStat(fbb_, fbb_.CreateString("Hello, " +
                                        request->GetRoot()->name()->str()));
    fbb_.Finish(stat_offset);
    // Since we keep reusing the same FlatBufferBuilder, the memory it owns
    // remains valid until the next call (this BufferRef doesn't own the
    // memory it points to).
    *response = flatbuffers::BufferRef<Stat>(fbb_.GetBufferPointer(),
                                             fbb_.GetSize());
    return grpc::Status::OK;
  }
  virtual ::grpc::Status Retrieve(::grpc::ServerContext *context,
                                  const flatbuffers::BufferRef<Stat> *request,
                                   ::grpc::ServerWriter< flatbuffers::BufferRef<Monster>>* writer)
                                  override {
       fbb_.Clear();
       std::cout << "Hello, " << request->GetRoot()->id()->str()
                 << request->GetRoot()->val()
                 << request->GetRoot()->count()
                 << std::endl;
       std::cout << "Streaming test.\n";

       for (int i=0; i<10; i++) {
         fbb_.Clear();
         auto monster_offset =
           CreateMonster(fbb_, 0, 0, 0, fbb_.CreateString("Fred No." + std::to_string(i)));
         fbb_.Finish(monster_offset);

         flatbuffers::BufferRef<Monster> result(
           fbb_.GetBufferPointer(), fbb_.GetSize());

         writer->Write(result);
       }

       return grpc::Status::OK;
  }

 private:
  flatbuffers::FlatBufferBuilder fbb_;
};

// Track the server instance, so we can terminate it later.
grpc::Server *server_instance = nullptr;
// Mutex to protec this variable.
std::mutex wait_for_server;
std::condition_variable server_instance_cv;

// This function implements the server thread.
void RunServer() {
  auto server_address = "0.0.0.0:50051";
  // Callback interface we implemented above.
  ServiceImpl service;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  // Start the server. Lock to change the variable we're changing.
  wait_for_server.lock();
  server_instance = builder.BuildAndStart().release();
  wait_for_server.unlock();
  server_instance_cv.notify_one();

  std::cout << "Server listening on " << server_address << std::endl;
  // This will block the thread and serve requests.
  server_instance->Wait();
}

int main(int /*argc*/, const char * /*argv*/[]) {
  // Launch server.
  std::thread server_thread(RunServer);

  // wait for server to spin up.
  std::unique_lock<std::mutex> lock(wait_for_server);
  while (!server_instance) server_instance_cv.wait(lock);

  // Now connect the client.
  auto channel = grpc::CreateChannel("localhost:50051",
                                     grpc::InsecureChannelCredentials());
  auto stub = MyGame::Example::MonsterStorage::NewStub(channel);

  {
    grpc::ClientContext context;
    // Build a request with the name set.
    flatbuffers::FlatBufferBuilder fbb;
    auto monster_offset = CreateMonster(fbb, 0, 0, 0, fbb.CreateString("Fred"));
    fbb.Finish(monster_offset);
    auto request = flatbuffers::BufferRef<Monster>(fbb.GetBufferPointer(),
                                                   fbb.GetSize());
    flatbuffers::BufferRef<Stat> response;

    // The actual RPC.
    auto status = stub->Store(&context, request, &response);

    if (status.ok()) {
      auto resp = response.GetRoot()->id();
      std::cout << "RPC response: " << resp->str() << std::endl;
    } else {
      std::cout << "RPC failed" << std::endl;
    }
  }
  {
      grpc::ClientContext context;
      // Build a request with the name set.
      flatbuffers::FlatBufferBuilder fbb;
      auto stat_offset = CreateStat(fbb, fbb.CreateString("Hi! I want to Fred"));
      fbb.Finish(stat_offset);
      auto request = flatbuffers::BufferRef<Stat>(
        fbb.GetBufferPointer(),fbb.GetSize()
      );

      flatbuffers::BufferRef<Monster> response;
      auto stream = stub->Retrieve(&context, request);
      while (stream->Read(&response)) {
        auto resp = response.GetRoot()->name();
        std::cout << "RPC Streaming response: " << resp->str() << std::endl;
      }
  }

  server_instance->Shutdown();

  server_thread.join();

  delete server_instance;

  return 0;
}
