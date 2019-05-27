#include <memory>

namespace some_namespace {
template<typename T> using custom_ptr = std::shared_ptr<T>;
}  // namespace some_namespace

namespace ns {
namespace some_namespace {}
}  // namespace ns

#include "custom_ptr_attribute_generated.h"

int main() {}
