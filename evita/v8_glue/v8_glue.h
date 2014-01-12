// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_v8_glue_h)
#define INCLUDE_evita_v8_glue_v8_glue_h

// L4 C4100:'identifier' : unreferenced formal parameter
// L4 C4510: 'class' : default constructor could not be generated
// L4 C4512: 'class' : assignment operator could not be generated
// L4 C4610: object 'class' can never be instantiated - user-defined
// constructor required
// L4 C4625: 'derived class' : copy constructor could not be generated because
// a base class copy constructor is inaccessible
#define BEGIN_V8_INCLUDE __pragma(warning(push)) \
  __pragma(warning(disable: 4100 4510 4512 4610 4625))

#define END_V8_INCLUDE __pragma(warning(pop))

#endif //!defined(INCLUDE_evita_v8_glue_v8_glue_h)
