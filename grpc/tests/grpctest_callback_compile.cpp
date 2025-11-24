#include "monster_test.grpc.fb.h"

// This test only verifies that the generated CallbackService compiles when the
// callback API is available. It does not run any RPCs.

#if defined(FLATBUFFERS_GENERATED_GRPC_CALLBACK_API) && \
    defined(GRPC_CALLBACK_API_NONEXPERIMENTAL)
class CallbackServiceImpl
    : public MyGame::Example::MonsterStorage::CallbackService {
 public:
  // For brevity we don't override methods; user code will provide reactors.
};
#endif

int main() {
#if defined(FLATBUFFERS_GENERATED_GRPC_CALLBACK_API) && \
    defined(GRPC_CALLBACK_API_NONEXPERIMENTAL)
  CallbackServiceImpl svc;
  (void)svc;  // suppress unused
#endif
  return 0;
}
