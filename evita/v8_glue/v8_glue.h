// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_v8_glue_h)
#define INCLUDE_evita_v8_glue_v8_glue_h

// L4 C4100: 'identifier' : unreferenced formal parameter
// L1 C4191: 'operator/operation' : unsafe conversion from 'type of
// expression' to 'type required'
// L4 C4365: 'action' : conversion from 'type_1' to 'type_2', signed/unsigned
// mismatch
// L4 C4510: 'class' : default constructor could not be generated
// L4 C4512: 'class' : assignment operator could not be generated
// L4 C4610: object 'class' can never be instantiated - user-defined
// constructor required
// L4 C4623: 'derived class' : default constructor could not be generated
// because a base class default constructor is inaccessible
// L4 C4625: 'derived class' : copy constructor could not be generated because
// a base class copy constructor is inaccessible
// L4 C4626: 'derived class' : assignment operator could not be generated
// because a base class assignment operator is inaccessible
// L4 C4668: 'symbol' is not defined as a preprocessor macro, replacing with
// '0' for 'directives'
// L1 C4946: reinterpret_cast used between related classes: 'class1' and
// 'class2'
#define BEGIN_V8_INCLUDE __pragma(warning(push)) \
  __pragma(warning(disable: 4100 4191 4365 4510 4512 4610 4623 4625 4626 \
                            4668 4946))

#define END_V8_INCLUDE __pragma(warning(pop))

BEGIN_V8_INCLUDE
#undef GetMessage
#include "v8/include/v8.h"
END_V8_INCLUDE

#endif //!defined(INCLUDE_evita_v8_glue_v8_glue_h)
