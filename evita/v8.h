#if !defined(INCLUDE_evita_v8_h)
#define INCLUDE_evita_v8_h

// warning C4100: 'identifier' : unreferenced formal parameter
#pragma warning(disable: 4100)

// warning C4623: 'derived class' : default constructor could not be generated
// because a base class default constructor is inaccessible
#pragma warning(disable: 4623)

// warning C4625: 'derived class' : copy constructor could not be generated
// because a base class copy constructor is inaccessible
#pragma warning(disable: 4625)

// warning C4626: 'derived class' : assignment operator could not be generated
// because a base class assignment operator is inaccessible
#pragma warning(disable: 4626)
#include "third_party/v8/include/v8.h"
#pragma warning(default: 4100)
#pragma warning(default: 4623)
#pragma warning(default: 4625)
#pragma warning(default: 4626)

#endif // !defined(INCLUDE_evita_v8_h)
