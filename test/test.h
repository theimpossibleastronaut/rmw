
// assert() does nothing if NDEBUG is defined, so we'll make sure it isn't.
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a)[0])
