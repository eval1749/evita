// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_castable_h)
#define INCLUDE_base_castable_h

namespace base {

// T* as()
// const char* class_name()
// bool is<T>()
// static const char* static_class_name()
class Castable {
  public: template<class Class> Class* as() {
    return is<Class>() ? static_cast<Class*>(this) : nullptr;
  }
  public: template<class Class> const Class* as() const {
    return is<Class>() ? static_cast<const Class*>(this) : nullptr;
  }
  public: virtual const char* class_name() const {
    return static_class_name();
  }
  public: template<class Class> bool is() const {
    return is_class_of(Class::static_class_name());
  }
  protected: virtual bool is_class_of(const char* other_name) const {
    return class_name() == other_name;
  }
  public: static const char* static_class_name() { return "Castable"; }
};

#define DECLARE_CASTABLE_CLASS(this_name, base_name) \
  public: static const char* static_class_name() { return #this_name; } \
  public: virtual const char* class_name() const override { \
    return static_class_name(); \
  } \
  protected: virtual bool is_class_of( \
      const char* other_name) const override { \
    return class_name() == other_name || base_name::is_class_of(other_name); \
  }

} // namespace base

#endif //!defined(INCLUDE_base_castable_h)
