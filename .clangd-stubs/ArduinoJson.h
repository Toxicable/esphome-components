#pragma once

#include <cstddef>

namespace ArduinoJson {

class Allocator {
 public:
  virtual ~Allocator() = default;
  virtual void *allocate(size_t size) = 0;
  virtual void deallocate(void *ptr) = 0;
  virtual void *reallocate(void *ptr, size_t new_size) = 0;
};

class JsonObject {};

class JsonVariant {
 public:
  void set(const char * /*value*/) {}
};

class JsonDocument {
 public:
  JsonDocument() = default;
  explicit JsonDocument(Allocator * /*allocator*/) {}

  template <typename T> T to() { return T{}; }
};

}  // namespace ArduinoJson

using ArduinoJson::JsonDocument;
using ArduinoJson::JsonObject;
using ArduinoJson::JsonVariant;
