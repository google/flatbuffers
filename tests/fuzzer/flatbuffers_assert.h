#ifndef FLATBUFFERS_ASSERT_IMPL_H_
#define FLATBUFFERS_ASSERT_IMPL_H_

extern "C" void __builtin_trap(void);

#define flatbuffers_assert_impl(x) (!!(x) ? static_cast<void>(0) : __builtin_trap())

#endif
