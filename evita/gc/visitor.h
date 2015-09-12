// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_GC_VISITOR_H_
#define EVITA_GC_VISITOR_H_

namespace gc {

class Visitable;

namespace internal {
class AbstractCollectable;
}

class Visitor {
 public:
  typedef internal::AbstractCollectable Collectable;
  Visitor() = default;
  virtual ~Visitor() = default;
  virtual void Visit(Collectable* collectable) = 0;
  virtual void Visit(Visitable* visitable) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(Visitor);
};

}  // namespace gc

#endif  // EVITA_GC_VISITOR_H_
