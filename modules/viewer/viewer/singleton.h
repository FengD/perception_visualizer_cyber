#pragma once

#include <pthread.h>

namespace crdc {
namespace airi {

template <typename T>
class Singleton {
 public:
  // @brief Get the singleton instance
  static T* get() {
    pthread_once(&p_once_, &Singleton::new_);
    return instance_;
  }

 private:
  Singleton();
  ~Singleton();

  // @brief Construct the singleton instance
  static void new_() { instance_ = new T(); }

  // @brief Destruct the singleton instance
  // @note Only work with gcc/clang
  __attribute__((destructor)) static void delete_() {
    typedef char T_must_be_complete[sizeof(T) == 0 ? -1 : 1];
    (void)sizeof(T_must_be_complete);
    delete instance_;
  }

  static pthread_once_t p_once_;  // Initialization once control
  static T* instance_;            // The singleton instance
};

template <typename T>
pthread_once_t Singleton<T>::p_once_ = PTHREAD_ONCE_INIT;

template <typename T>
T* Singleton<T>::instance_ = nullptr;

}  // namespace airi
}  // namespace crdc

#define MAKE_SINGLETON(Type) \
 private:                    \
  Type() {}                  \
  ~Type() {}                 \
  friend class ::crdc::airi::Singleton<Type>
