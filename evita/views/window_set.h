// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#ifndef EVITA_VIEWS_WINDOW_SET_H_
#define EVITA_VIEWS_WINDOW_SET_H_

#include <unordered_set>

#include "base/macros.h"

namespace views {

class Window;

class WindowSet {
 public:
  typedef std::unordered_set<Window*> Set;

  explicit WindowSet(Set&& set);
  ~WindowSet();

  Set::const_iterator begin() const { return set_.begin(); }
  Set::const_iterator end() const { return set_.end(); }

 private:
  Set set_;

  DISALLOW_COPY_AND_ASSIGN(WindowSet);
};

}  // namespace views

#endif  // EVITA_VIEWS_WINDOW_SET_H_
