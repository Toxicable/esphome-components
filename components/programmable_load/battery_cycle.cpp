// ESPHome compiles every direct .cpp file in an external component. The
// implementation is included by programmable_load.cpp so it can share that
// translation unit's private helpers; compiling it here as well would create
// duplicate symbols.
#if __INCLUDE_LEVEL__ > 0
#include "battery_cycle_impl.inc"
#endif
