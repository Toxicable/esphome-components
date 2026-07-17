# Component Architecture

This repository contains ESPHome external components ranging from small device adapters to stateful controllers and multi-device procedures. The architecture should make those components easier to reason about without adding boilerplate that ESPHome does not need.

Use this document for new components and when substantially refactoring an existing component. Existing components may migrate incrementally; compatibility with committed consumer YAML is normally more important than making every component immediately conform.

See `COMPONENTS.md` for the lifecycle status and known consumers of each component.

## Language baseline

The repository host checks use GNU C++20, matching the pinned ESPHome 2026.5.3 build configuration. C++20 features are acceptable when they materially improve clarity, including designated initializers for register manifests and typed configuration aggregates.

## Component classes

Choose the lightest structure that fits the component.

### Simple adapter

A small read-only sensor, output, or device adapter may remain a normal ESPHome component with `__init__.py`, one header, and one source file. Do not create protocol, bus, and service classes merely to satisfy a directory pattern.

Typical examples are devices with a small register surface and no meaningful reconciliation, fault policy, or reusable procedure logic.

### Stateful writable device

Use explicit protocol/bus/service/wrapper boundaries when a component:

- owns a substantial desired configuration;
- reconciles device state after reset or communication failure;
- performs command sequences or retries;
- exposes non-trivial status and fault decoding; or
- is likely to be reused from a non-ESPHome host.

### Orchestrator

A component coordinating other devices or procedures should own a typed C++ state machine. It should consume narrow capability interfaces from the controlled components rather than reading or commanding their Home Assistant entities.

Entities are presentation and user-control surfaces. They are not the preferred machine-to-machine API inside one firmware image.

An orchestrator must also have one explicit ownership authority. Manual control and procedures acquire the same lock; a procedure returns requested outputs to the core, and the core applies safety limits, hardware limits and cross-device interlocks. Procedures never directly drive hardware. Keep the public operational state small (`idle`, `running`, `fault`) and publish procedure-specific progress separately when useful.

## ESPHome external-component constraints

These constraints are part of the architecture because they determine which source layouts ESPHome can discover and compile.

### External-component roots

`external_components.source.path` must point at the directory containing component directories:

```text
components/
  bq25756/
    __init__.py
  programmable_load/
    __init__.py
```

The repository supports both local and Git sources, but consumer repositories commonly use a local checkout of this repository.

### The `components` list is an import allowlist

A consumer configuration such as:

```yaml
external_components:
  - source:
      type: local
      path: ../../esphome-components/components
    refresh: 0s
    components:
      - bq25756
      - programmable_load
```

makes only those named component packages available from that external source. The list is not merely a build optimisation.

This matters for internal helper components: a helper requested through `AUTO_LOAD` must also be present in every explicit external-component allowlist that may need it. Otherwise ESPHome's external-component importer cannot resolve the helper package.

Do not add an internal helper to the YAML as a top-level configuration block unless it intentionally has a user-facing `CONFIG_SCHEMA`.

### Shared code must be an ESPHome component package

Cross-component shared code should live in an internal component package under the same external-component root:

```text
components/
  component_common/
    __init__.py
    register_field.h
    operation_result.h
  bq25756/
    __init__.py
    bq25756.cpp
    bq25756.h
```

A consuming component requests it in Python:

```python
AUTO_LOAD = ["component_common"]
```

The consumer repository must make both packages available:

```yaml
external_components:
  - source:
      type: local
      path: ../../esphome-components/components
    components:
      - component_common
      - bq25756
```

The helper's `__init__.py` may contain only namespace/type declarations and metadata. It does not need a YAML schema.

Use `AUTO_LOAD` for implementation helpers. Use `DEPENDENCIES` only when the user must configure another component explicitly; `DEPENDENCIES` does not substitute for an internal shared-code package.

### Family-level internal packages

When two related chips share mechanics but not register maps or product policy, use an internal family component rather than putting chip details into `component_common`.

For example, `mcf83xx_common` owns only the MCx83xx register-bus contract, control-word/frame encoding, read-modify-write and pulse operations. `mcf8316d` and `mcf8329a` retain their own registers, faults, scaling, tuning, startup sequencing and ESPHome entities.

A public family member loads the helper transitively:

```python
AUTO_LOAD = ["mcf83xx_common", "sensor", "switch"]
```

An explicit external-component allowlist must permit the complete chain:

```yaml
components: [ component_common, mcf83xx_common, mcf8316d ]
```

Do not make the family helper a top-level YAML block. Do not move a field into the family package merely because both chips happen to use a similarly named register.

### Source discovery is package-local and non-recursive

ESPHome external components collect supported C/C++ source files directly inside each loaded component package. Do not assume that nested implementation directories will be copied into the generated source tree.

Prefer:

```text
components/component_common/
  __init__.py
  register_field.h
  operation_result.h
  operation_result.cpp
```

Do not rely on:

```text
components/component_common/
  __init__.py
  registers/
    register_field.h
  faults/
    operation_result.cpp
```

Keep reusable `.cpp` files directly in the component directory. Header-only helpers are preferable where they remain small and do not create excessive compile-time cost.

Host-independent component cores should use sibling-relative includes so the same files compile in repository host tests and after ESPHome copies component packages:

```cpp
#include "../component_common/bit_field.h"
#include "../mcf83xx_common/register_access.h"
```

ESPHome-only facade code may use generated `esphome/components/...` include paths.

Re-check these assumptions when changing the pinned ESPHome version. They reflect the external-component loader and source-copy behavior expected by this repository, not a general CMake source-discovery mechanism.

### Keep explicit allowlists

Do not remove consumer `external_components.components` allowlists merely to make helper discovery easier. This repository contains local components whose names can overlap upstream ESPHome components. Making every local component available can accidentally shadow upstream behavior.

When adding or removing an internal helper dependency, update the relevant consumer allowlists in the same change or in a coordinated pull request.

## Layers for stateful components

### Registers and protocol

Recommended files:

- `<component>_registers.h` for addresses, masks, field definitions, and device constants;
- `<component>_protocol.h` / `.cpp` for host-independent encoding and decoding.

Protocol code owns device facts:

- register addresses and bit masks;
- command encoders;
- status and measurement structs;
- register decoding;
- physical-unit conversion;
- string mappings for decoded device states.

Protocol files must not include ESPHome headers, perform I2C transactions, log, own entities, or know about YAML configuration.

Do not call a class `Protocol` when it directly inherits `i2c::I2CDevice` or performs transport operations. Name that class for its real role, such as `I2CTransport` or `RegisterBusAdapter`.

### Bus

`<component>_bus.h` defines the narrowest host interface needed by reusable service logic. It is the boundary between device behavior and the platform that performs I/O.

```cpp
class RegisterBus {
 public:
  virtual bool read_register(uint8_t reg, uint8_t *value) = 0;
  virtual bool write_register(uint8_t reg, uint8_t value) = 0;
  virtual void delay_ms(uint32_t ms) = 0;
};
```

Byte-oriented devices may expose single-register calls. Devices that naturally transfer blocks should expose block reads and writes. Avoid leaking ESPHome I2C error types through this interface.

### Service or core

`<component>_service.h` / `.cpp`, or another clearly named core class, owns reusable behavior:

- probing the device;
- validating and applying desired configuration;
- reading status and measurements;
- reconciling state after reset;
- retries and command sequencing;
- requesting operating modes;
- resetting watchdogs or command state.

Services communicate only through the bus interface. They must not include ESPHome headers, own ESPHome entities, emit ESPHome logs, or depend on YAML representation.

Core class names must be component-specific:

- `husb238_core::HusbService`
- `bq25756_core::Bq25756Service`

Avoid exported generic names such as `Service`, `Config`, or `Status` in a namespace shared by multiple component families.

### ESPHome wrapper

`<component>.h` / `.cpp` plus `__init__.py` own ESPHome integration:

- YAML schema and code generation;
- sensors, text sensors, switches, selects, numbers, and buttons;
- ESPHome logging;
- ESPHome I2C/SPI/UART adapters;
- persistence adapters;
- translation of ESPHome lifecycle calls into core calls;
- publication of snapshots produced by the core.

The wrapper may implement the core bus interface and hold the service instance:

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

Another host can reuse the same service by implementing the same bus interface.

## Configuration contract

For stateful components, use this flow:

```text
YAML schema -> typed component Config -> core validation -> service reconciliation
```

### Typed aggregate

Prefer one typed C++ configuration aggregate over dozens of independent setters:

```cpp
struct ChargerConfig {
  float charge_voltage_v;
  float battery_current_limit_a;
  float input_current_limit_a;
  bool disable_pfm;
};
```

Python code generation should construct or populate the aggregate and pass it to the component/core as one logical desired state.

Avoid:

- parallel `has_value` flags and values where `std::optional` or a complete desired-state model is clearer;
- private synthetic Python dictionary keys used only to ferry values between schema stages;
- raw register values in YAML when a physical unit or named enum exists;
- large setter storms that make cross-field validation and reconciliation difficult.

### Complete desired state versus patch semantics

Every writable component must deliberately choose one model:

- **complete desired state** — omitted values receive explicit defaults and the component owns the resulting hardware state;
- **patch/preserve** — omitted values leave existing device state unchanged.

Document the choice. Product-owned hardware should normally use complete desired state so startup is deterministic. Do not accidentally mix the two models.

### Validation

Perform friendly cross-field validation in Python so configuration errors point to YAML. Repeat safety-critical bounds defensively in C++ because the core may be reused without ESPHome.

Register scaling, masks, and quantisation belong in C++ protocol code, not in Python code generation.

## Register handling

Do not build one universal runtime register database for unrelated devices. Chip-specific addresses, fields, scaling, reset values, and command rules remain with the chip component or family.

A small shared helper component may provide genuinely generic facilities such as:

- endian load/store helpers;
- typed bit-field extraction and insertion;
- checked clamp and quantisation;
- read-modify-write helpers;
- a common transport operation result;
- lightweight retry/availability bookkeeping.

Shared helpers must not contain chip-specific register numbers, fault enums, configuration defaults, or policy.

Prefer named fields and physical units at the service boundary. Raw register access should be reserved for tightly scoped debugging or unsupported device features.

## State and fault contract

Do not overload one text sensor with connection state, operating state, and hardware faults.

### Connection state

Connection state describes transport availability, not configuration readiness or device operation. A common conceptual set is:

- disconnected;
- connecting;
- connected;
- failed.

A communication timeout changes connection state. It is not automatically a hardware protection fault. Configuration readiness should normally drive ESPHome component warning/error status rather than adding another public state entity.

### Operational state

Operational state is component-specific and user-relevant, for example:

- idle;
- running;
- fault;

or charger-specific states such as charging, complete, or suspended.

### Fault representation

`component_common/status.h` provides only the policy-free connection-state enum. Each component keeps its own fault bitset, decoder, and formatter because fault meaning and ordering are device-specific.

Stateful components should expose a typed internal snapshot containing enough information for policy and diagnostics:

```cpp
struct DeviceSnapshot {
  component_common::ConnectionState connection_state;
  DeviceOperatingState state;
  uint32_t active_faults;
  bool configuration_ready;
  uint32_t raw_status;  // optional diagnostic data
};
```

The ESPHome-facing wrapper may publish the active or latched bitset as one deterministic comma-delimited text sensor when users need the complete set. Multiple simultaneous safety causes should not be discarded merely to choose one primary fault.

- `raw_status` is diagnostic and optional.

The ESPHome wrapper decides which parts become entities.

## Component-to-component interfaces

When one component controls another, expose a narrow typed C++ capability from the controlled component.

Example shape:

```cpp
class ChargerInterface {
 public:
  virtual bool request_enabled(bool enabled) = 0;
  virtual ChargerSnapshot snapshot() const = 0;
  virtual ChargerCapabilities capabilities() const = 0;
};
```

The controlling component should receive the component ID and resolve it to the typed interface during code generation. It should not consume the controlled component's `sensor::Sensor`, `switch_::Switch`, or `text_sensor::TextSensor` entities as its control protocol.

Entities may observe and command the same underlying core, but the internal procedure should continue to work even when those entities are hidden or not exposed to Home Assistant.

## Entity exposure

Core knowledge and Home Assistant exposure are separate decisions.

### Normal

Expose by default only information and controls useful in ordinary operation:

- primary measurements;
- operational state;
- primary actionable fault;
- essential safe controls.

### Diagnostic

Optional diagnostics may include:

- communication counters;
- learned/calibration values;
- secondary measurements;
- decoded status summaries;
- active-fault indicators.

Use ESPHome's diagnostic entity category where appropriate.

### Debug

Raw register dumps, manual internal modes, verbose telemetry, and implementation-level controls should require an explicit debug configuration section. Do not expose them merely because the core can report them.

### Manufacturing and irreversible actions

Irreversible calibration-memory writes, OTP programming, destructive resets, and manufacturing-only operations require an explicit manufacturing/advanced configuration section and prominent documentation. Reversible runtime calibration may live in a dedicated `calibration:` section, but must be atomic, validated, idle-only, and expose persistence/reset behavior clearly. Marking an irreversible button as diagnostic is not sufficient protection.

## Python organisation

A component may keep a single user-facing YAML block while splitting a large Python implementation into private modules within the same component package, for example:

```text
components/bq76952/
  __init__.py
  _schema.py
  _entities.py
  _codegen.py
```

Python submodules are part of the same component package and do not create additional YAML components. Keep the public schema and `to_code` entry points discoverable from `__init__.py`.

Do not split small integrations simply to reduce file length.

## File layout

A typical stateful component may use:

```text
components/<name>/
  __init__.py
  <name>.h
  <name>.cpp
  <name>_registers.h
  <name>_bus.h
  <name>_protocol.h
  <name>_protocol.cpp
  <name>_service.h
  <name>_service.cpp
```

Use only the files the component needs. Reusable C/C++ source files must remain directly under the component directory unless source discovery is explicitly tested and documented.

## Testing expectations

Architecture changes should be validated at the same boundaries they introduce:

- compile committed `test_config.yaml` files against the pinned ESPHome version;
- compile real consumer configurations when changing external-component loading or public YAML;
- host-test protocol and service logic with a fake bus where practical;
- test valid and invalid schema combinations;
- verify host-independent files do not include ESPHome headers;
- preserve intentional YAML/API compatibility or document the migration.

Syntax-only checks are useful but do not prove that ESPHome can discover shared components or compile a consumer configuration.

## Documentation ownership

Each component README should keep a concise usage and code-organisation summary and link back to this file for repository-wide rules.

Component-specific active constraints belong in `components/<name>/AGENTS_KNOWLEDGE.md`; edit maps and read order belong in `components/<name>/CONTEXT_INDEX.md`.

When a component gains a shared helper dependency, document it in the component README and update known consumer allowlists. When a component's lifecycle changes, update `COMPONENTS.md` in the same pull request.
