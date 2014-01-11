// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_view_window_set_h)
#define INCLUDE_view_window_set_h

#include <unordered_set>

namespace view {

class Window;

class WindowSet {
  public: typedef std::unordered_set<Window*> Set;
  private: Set set_;

  public: WindowSet(Set&& set);
  public: ~WindowSet();

  public: Set::const_iterator begin() const { return set_.begin(); }
  public: Set::const_iterator end() const { return set_.end(); }
};

}  // namespace view

#endif //!defined(INCLUDE_view_window_set_h)
