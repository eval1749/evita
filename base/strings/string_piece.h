// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_strings_string_piece_h)
#define INCLUDE_base_strings_string_piece_h

#include "base/base_export.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include <iosfwd>

namespace base {

template<typename StringType> class BasicStringPiece;
typedef BasicStringPiece<std::string> StringPiece;
typedef BasicStringPiece<base::string16> StringPiece16;

namespace impl {

template<typename StringType> class StringPieceImpl {
  // Standard STL container boilerplate
  public: typedef size_t size_type;
  public: typedef typename StringType::value_type value_type;
  public: typedef const value_type* pointer;
  public: typedef const value_type& reference;
  public: typedef const value_type& const_reference;
  public: typedef ptrdiff_t difference_type;
  public: typedef const value_type* const_iterator;
  public: typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  public: static const size_type npos;

  protected: const value_type* data_;
  protected: size_type length_;

  public: StringPieceImpl() : data_(nullptr), length_(0) {
  }
  public: StringPieceImpl(const value_type* str)
      : data_(str), length_(str ? StringType::traits_type::length(str) : 0) {
  }
  public: StringPieceImpl(const StringType& str)
      : data_(str.data()), length_(str.size()) {
  }
  public: StringPieceImpl(const value_type* offset, size_type length)
      : data_(offset), length_(length) {
  }
  public: StringPieceImpl(
      const typename StringType::const_iterator& begin,
      const typename StringType::const_iterator& end)
      : data_(end > begin ? &*begin : nullptr),
        length_(end > begin ? static_cast<size_type>(end - begin) :
                              static_cast<size_type>(0)) {
  }

  public: value_type operator[](size_type index) const {
    DCHECK_GE(index, static_cast<size_type>(0));
    DCHECK_LT(index, length_);
    return data_[index];
  }

  public: const value_type* data() const { return data_; }
  public: size_type length() const { return length_; }
  public: size_type size() const { return length_; }
  public: bool empty() const { return !length_; }

  public: StringType as_string() const {
    return empty() ? StringType() : StringType(data(), size());
  }

  public: const_iterator begin() const { return data_; }
  public: const_iterator end() const { return data_ + length_; }
};

template<typename StringType>
const typename StringPieceImpl<StringType>::size_type
StringPieceImpl<StringType>::npos =
    static_cast<typename StringPieceImpl<StringType>::size_type>(-1);
}  // namespace impl

template<typename StringType> class BasicStringPiece
    : public impl::StringPieceImpl<StringType> {
  public: typedef typename impl::StringPieceImpl<StringType>::value_type
      value_type;
  public: typedef typename impl::StringPieceImpl<StringType>::size_type
      size_type;

  public: BasicStringPiece() {
  }
  public: BasicStringPiece(const value_type* str)
      : impl::StringPieceImpl<StringType>(str) {
  }
  public: BasicStringPiece(const StringType& str)
      : impl::StringPieceImpl<StringType>(str) {
  }
  public: BasicStringPiece(const value_type* offset, size_type length)
      : impl::StringPieceImpl<StringType>(offset, length) {
  }
  public: BasicStringPiece(
      const typename StringType::const_iterator& begin,
      const typename StringType::const_iterator& end)
      : impl::StringPieceImpl<StringType>(begin, end) {
  }
};

template<> class BasicStringPiece<std::string>
    : public impl::StringPieceImpl<std::string> {
  public: BasicStringPiece() {
  }
  public: BasicStringPiece(const char* str)
      : impl::StringPieceImpl<std::string>(str) {
  }
  public: BasicStringPiece(const std::string& str)
      : impl::StringPieceImpl<std::string>(str) {
  }
  public: BasicStringPiece(const char* offset, size_type length)
      : impl::StringPieceImpl<std::string>(offset, length) {
  }
  public: BasicStringPiece(const std::string::const_iterator& begin,
                           const std::string::const_iterator& end)
      : impl::StringPieceImpl<std::string>(begin, end) {
  }
};

//#ifndef _MSC_VER
template class BASE_EXPORT BasicStringPiece<std::string>;
template class BASE_EXPORT BasicStringPiece<string16>;
//#endif

BASE_EXPORT bool operator==(const StringPiece& x, const StringPiece& y);
BASE_EXPORT bool operator==(const StringPiece16& x, const StringPiece16& y);

inline bool operator!=(const StringPiece& x, const StringPiece& y) {
  return !(x == y);
}

inline bool operator!=(const StringPiece16& x, const StringPiece16& y) {
  return !(x == y);
}

BASE_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                     const StringPiece& piece);

} // namespace base

#endif //!defined(INCLUDE_base_strings_string_piece_h)
