#pragma once

#define STRICT
#define INC_OLE2
// L4 C4668: 'symbol' is not defined as a preprocessor macro, replacing with
//           '0' for 'directives'
#pragma warning(push)
#pragma warning(disable: 4668)
#include <windows.h>
#pragma warning(pop)

// warning  C4251: 'identifier' : class 'type' needs to have dll-interface to
// be used by clients of class 'type2'
// Example: std::unique_ptr<T>
#pragma warning(disable: 4251)

// warning C4530: C++ exception handler used, but unwind semantics are not
// enabled. Specify /EHsc
#pragma warning(disable: 4530)

// warning C4800: 'BOOL' : forcing value to bool 'true' or 'false'
// (performance warning)
#pragma warning(disable:4800)
