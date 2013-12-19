#pragma once

#define STRICT
#define INC_OLE2
#define WIN32_LEAN_AND_MEAN

// wrning C4350: behavior change: 'member1' called instead of 'member2'
#pragma warning(disable: 4350)

// warning C4365: 'argument' : conversion from 'const int' to 'const size_t', signed/unsigned mismatch
#pragma warning(disable: 4365)

// warning C4512: 'base::Name' : assignment operator could not be generated
#pragma warning(disable: 4512)

// warning C4530:  C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#pragma warning(disable: 4530)

// warning C4668: '__midl' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning(disable: 4668)

// warning C4722: 'Common::GlobalMemoryZone::~GlobalMemoryZone' : destructor never returns, potential memory leak
#pragma warning(disable: 4722)

// warning C4820: '_SECURITY_QUALITY_OF_SERVICE' : '2' bytes padding added after data member '_SECURITY_QUALITY_OF_SERVICE::EffectiveOnly'
#pragma warning(disable: 4820)

#include <windows.h>
#include <stddef.h> // ptrdiff_t
