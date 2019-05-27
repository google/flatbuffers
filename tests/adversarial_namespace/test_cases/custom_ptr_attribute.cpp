#include <memory>

namespace some_namespace {
template<typename T> struct custom_ptr : public std::shared_ptr<T> {
  custom_ptr() {}
  custom_ptr(T *p) : std::shared_ptr<T>(p) {}

  T *get() const { return std::shared_ptr<T>::get(); }
};
}  // namespace some_namespace

namespace ns {
namespace some_namespace {}
}  // namespace ns

#include "custom_ptr_attribute_generated.h"

int main() {}
