#ifndef __FREERTOS_PLUS_CPP_LOCK_H__
#define __FREERTOS_PLUS_CPP_LOCK_H__

#include <Arduino_FreeRTOS.h>
#include <semphr.h>

class LockGuard {
 public:
  explicit LockGuard(SemaphoreHandle_t mutex_handle)
      : mutex_handle_(mutex_handle) {
    xSemaphoreTake(mutex_handle_, portMAX_DELAY);
  }

  ~LockGuard() { xSemaphoreGive(mutex_handle_); }

 private:
  SemaphoreHandle_t mutex_handle_;
};

struct DeferLock_t {};
struct TryToLock_t {};
struct AdoptLock_t {};

constexpr DeferLock_t DeferLock{};
constexpr TryToLock_t TryToLock{};
constexpr AdoptLock_t AdoptLock{};

class UniqueLock {
 public:
  explicit UniqueLock(SemaphoreHandle_t mutex_handle)
      : mutex_handle_(mutex_handle), owns_(false) {
    Lock();
  }

  explicit UniqueLock(SemaphoreHandle_t mutex_handle, DeferLock_t t)
      : mutex_handle_(mutex_handle), owns_(false) {}

  explicit UniqueLock(SemaphoreHandle_t mutex_handle, TryToLock_t t)
      : mutex_handle_(mutex_handle), owns_(false) {
    TryLock();
  }

  explicit UniqueLock(SemaphoreHandle_t mutex_handle, AdoptLock_t t)
      : mutex_handle_(mutex_handle), owns_(true) {}

  explicit UniqueLock(SemaphoreHandle_t mutex_handle, uint32_t duration_ms)
      : mutex_handle_(mutex_handle), owns_(true) {
    TryLockFor(duration_ms);
  }

  ~UniqueLock() {
    if (owns_) {
      xSemaphoreGive(mutex_handle_);
    }
  }

  void Lock() {
    configASSERT(mutex_handle_);
    configASSERT(!owns_);
    xSemaphoreTake(mutex_handle_, portMAX_DELAY);
    owns_ = true;
  }

  void Unlock() {
    configASSERT(mutex_handle_);
    configASSERT(owns_);
    xSemaphoreGive(mutex_handle_);
    owns_ = false;
  }

  bool TryLock() {
    configASSERT(mutex_handle_);
    configASSERT(!owns_);
    owns_ = (xSemaphoreTake(mutex_handle_, 0) != pdFALSE);
    return owns_;
  }

  bool TryLockFor(uint32_t duration_ms) {
    configASSERT(mutex_handle_);
    configASSERT(!owns_);
    owns_ =
        (xSemaphoreTake(mutex_handle_, pdMS_TO_TICKS(duration_ms)) != pdFALSE);
    return owns_;
  }

 private:
  SemaphoreHandle_t mutex_handle_;
  bool owns_;
};

#endif  // __FREERTOS_PLUS_CPP_LOCK_H__
