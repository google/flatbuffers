// Verifies that the generated gRPC callback client stub methods are present.
// This is a compile-time only test: it never performs an actual RPC.
// It supplements grpctest_callback_compile.cpp (server side) by checking
// client-side async_ / reactor entry points.

#include <type_traits>

#include "monster_test.grpc.fb.h"

#if defined(FLATBUFFERS_GENERATED_GRPC_CALLBACK_API) && \
    defined(GRPC_CALLBACK_API_NONEXPERIMENTAL)
using Stub = MyGame::Example::MonsterStorage::Stub;
using namespace MyGame::Example;  // NOLINT

// Unary async overloads
static_assert(std::is_member_function_pointer<
                  decltype(static_cast<void (Stub::*)(
                               ::grpc::ClientContext*,
                               const flatbuffers::grpc::Message<Monster>&,
                               flatbuffers::grpc::Message<Stat>*,
                               std::function<void(::grpc::Status)>)>(
                      &Stub::async_Store))>::value,
              "Function-form unary async_Store missing");
static_assert(
    std::is_member_function_pointer<
        decltype(static_cast<void (Stub::*)(
                     ::grpc::ClientContext*,
                     const flatbuffers::grpc::Message<Monster>&,
                     flatbuffers::grpc::Message<Stat>*,
                     ::grpc::ClientUnaryReactor*)>(&Stub::async_Store))>::value,
    "Reactor-form unary async_Store missing");

// Streaming reactor entry points
static_assert(
    std::is_member_function_pointer<
        decltype(static_cast<void (Stub::*)(
                     ::grpc::ClientContext*,
                     const flatbuffers::grpc::Message<Stat>&,
                     ::grpc::ClientReadReactor<flatbuffers::grpc::Message<
                         Monster> >*)>(&Stub::async_Retrieve))>::value,
    "Server streaming reactor async_Retrieve missing");
static_assert(
    std::is_member_function_pointer<
        decltype(static_cast<void (Stub::*)(
                     ::grpc::ClientContext*, flatbuffers::grpc::Message<Stat>*,
                     ::grpc::ClientWriteReactor<flatbuffers::grpc::Message<
                         Monster> >*)>(&Stub::async_GetMaxHitPoint))>::value,
    "Client streaming reactor async_GetMaxHitPoint missing");
static_assert(std::is_member_function_pointer<
                  decltype(static_cast<void (Stub::*)(
                               ::grpc::ClientContext*,
                               ::grpc::ClientBidiReactor<
                                   flatbuffers::grpc::Message<Monster>,
                                   flatbuffers::grpc::Message<Stat> >*)>(
                      &Stub::async_GetMinMaxHitPoints))>::value,
              "Bidi streaming reactor async_GetMinMaxHitPoints missing");
#endif  // FLATBUFFERS_GENERATED_GRPC_CALLBACK_API &&
        // GRPC_CALLBACK_API_NONEXPERIMENTAL

int main() { return 0; }
