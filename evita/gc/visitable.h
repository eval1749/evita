// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_GC_VISITABLE_H_
#define EVITA_GC_VISITABLE_H_

#include "base/macros.h"

namespace gc {

class Visitor;

class Visitable {
 public:
  Visitable();
  virtual ~Visitable();

  virtual const char* visitable_class_name() const = 0;

  virtual void Accept(Visitor* visitor);

 private:
  DISALLOW_COPY_AND_ASSIGN(Visitable);
};

}  // namespace gc

#endif  // EVITA_GC_VISITABLE_H_
