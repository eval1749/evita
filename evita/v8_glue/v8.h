#if !defined(INCLUDE_evita_v8_glue_v8_h)
#define INCLUDE_evita_v8_glue_v8_h

#pragma warning(push)

// warning C4100: 'identifier' : unreferenced formal parameter
#pragma warning(disable: 4100)

// warning C4127: conditional expression is constant
#pragma warning(disable: 4127)

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
#pragma warning(pop)

#endif // !defined(INCLUDE_evita_v8_glue_v8_h)
