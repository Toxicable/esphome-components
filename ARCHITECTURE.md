# Component Architecture

ESPHome components in this repo can be split into host-independent core code and an ESPHome adapter. Use this layout when device behavior may need to run from another host, such as STM32 firmware, as well as from ESPHome.

## Layers

### Protocol

`*_protocol.h` / `*_protocol.cpp`

Protocol files own device facts:

- register addresses and bit masks
- command encoders
- status and measurement structs
- register decoding
- string mappings for decoded device states

Protocol files must not include ESPHome headers, talk to I2C directly, log, or know about Home Assistant entities.

### Bus

`*_bus.h`

The bus file defines the minimal host interface needed by the core service. It is the boundary between reusable device behavior and the platform that performs I/O.

Examples:

```cpp
class RegisterBus {
 public:
  virtual bool read_register(uint8_t reg, uint8_t *value) = 0;
  virtual bool write_register(uint8_t reg, uint8_t value) = 0;
  virtual void delay_ms(uint32_t ms) = 0;
};
```

Use the narrowest interface the chip needs. Byte-oriented devices can expose single-register calls; devices that naturally use register blocks can expose block reads and writes.

### Service

`*_service.h` / `*_service.cpp`

Service files own reusable device behavior:

- probing the device
- reading status and measurements
- applying limits
- requesting operating modes
- resetting watchdogs or command state

Services talk only through the bus interface. They must not include ESPHome headers, own ESPHome entities, emit ESPHome logs, or know about YAML configuration.

Core service class names should be component-specific, not generic:

- `husb238_core::HusbService`
- `bq25756_core::Bq25756Service`

Avoid exported names like `Service` because multiple component cores may be reused in one firmware image.

### ESPHome Wrapper

`<component>.h` / `<component>.cpp` plus `__init__.py`

The ESPHome wrapper owns ESPHome integration:

- YAML schema and code generation
- sensors, text sensors, switches, selects, and buttons
- ESPHome logging
- ESPHome I2C adapter methods
- translating ESPHome lifecycle calls into service calls

The wrapper implements the core bus interface and holds the service instance.

```cpp
class HUSB238Component : public PollingComponent,
                         public i2c::I2CDevice,
                         public husb238_core::RegisterBus {
 public:
  HUSB238Component() : service_(this) {}

 private:
  husb238_core::HusbService service_;
};
```

Another host can reuse the same service by implementing the same bus interface:

```cpp
class Husb238Stm32Bus : public husb238_core::RegisterBus {
  // Implement reads, writes, and delays with the STM32 HAL.
};

Husb238Stm32Bus bus;
husb238_core::HusbService husb(&bus);
```

## File Layout

Keep reusable files directly under `components/<name>/`:

```text
components/<name>/
  __init__.py
  <name>.h
  <name>.cpp
  <name>_bus.h
  <name>_protocol.h
  <name>_protocol.cpp
  <name>_service.h
  <name>_service.cpp
```

ESPHome external components collect direct component files by default. Do not hide reusable `.cpp` files in nested source directories unless the component explicitly owns the needed source discovery behavior.

## Documentation

Each component README should keep only a short code-organization summary and link back to this file. Component-specific active constraints belong in `components/<name>/AGENTS_KNOWLEDGE.md`; edit maps and read order belong in `components/<name>/CONTEXT_INDEX.md`.
