// ESPHome compiles every .cpp file in an external component. The battery-cycle
// implementation is amalgamated into programmable_load.cpp, so the standalone
// translation unit must stay empty to avoid duplicate symbols.
#if defined(__INCLUDE_LEVEL__) && __INCLUDE_LEVEL__ > 0
#include "battery_cycle_impl.h"
#endif
