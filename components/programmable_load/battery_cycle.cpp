// ESPHome compiles every .cpp file in an external component. The legacy
// programmable_load.cpp translation unit also includes this path directly.
// Compile the implementation only when this file is the top-level source;
// when included from programmable_load.cpp it intentionally emits nothing.
#if __INCLUDE_LEVEL__ == 0
#include "battery_cycle.inc"
#endif
