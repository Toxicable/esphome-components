# Component Common

Internal, header-only C++ helpers shared by components in this repository. This package has no user-facing YAML schema and must be loaded transitively with `AUTO_LOAD`.

A consuming component declares:

```python
AUTO_LOAD = ["component_common"]
```

Any consumer using an explicit external-component allowlist must make the helper package available as well:

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ component_common, bq25756 ]
```

Host-independent component code should use sibling-relative includes, for example:

```cpp
#include "../component_common/bit_field.h"
```

That form resolves both in this source tree and after ESPHome copies the loaded packages to `esphome/components/`. ESPHome-only wrapper code may use a generated `esphome/components/...` include path.

## Helpers

- `bit_field.h`: contiguous register-field encode/decode/replace and masked updates.
- `byte_order.h`: fixed-width unsigned little-endian and big-endian load/store.
- `charger.h`: typed charger capabilities, snapshots, and control boundary for component composition.
- `status.h`: lifecycle states and a generic typed fault snapshot for stateful components.

Keep this package small and policy-free. Chip addresses, reset values, scaling, faults, and configuration defaults remain in the owning component or chip family.
