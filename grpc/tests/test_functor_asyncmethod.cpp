#include <thread>
#include <tuple>
#include <grpc++/grpc++.h>

#include "monster_test.grpc.fb.h"
#include "monster_test_generated.h"

const bool cause_segfault = false;

template <class AsyncService>
struct CallNested {
  static void call(AsyncService &s) {}
};

template <class Nested, template <class> class WithAsyncMethod>
struct CallNested<WithAsyncMethod<Nested>> {
  static void call(WithAsyncMethod<Nested> &async) {
    if (cause_segfault) {
      // The next line is the objective. Of course, in real code, it's not all zeros.
      async.Request(0, 0, 0, 0, 0, 0);
    }
    CallNested<Nested>::call(async);
  };
};

template <class AsyncService>
void call_nested(AsyncService &async) {
  CallNested<AsyncService>::call(async);
}

int nested_async_methods() {
  MyGame::Example::MonsterStorage::AsyncService async;
  call_nested(async);
  return 0;
}
