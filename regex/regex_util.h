//////////////////////////////////////////////////////////////////////////////
//
// Regex - Utilities
// regex_util.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_util.h#2 $
//
#ifndef REGEX_REGEX_UTIL_H_
#define REGEX_REGEX_UTIL_H_

#include "base/logging.h"

namespace Regex {
namespace RegexPrivate {

template <class T>
class Castable_ {
 private:
  typedef Castable_<T> Self;

  // [D]
 public:
  template <class T>
  T* DynamicCast() const {
    return const_cast<Self*>(this)->DynamicCast<T>();
  }

 public:
  template <class T>
  T* DynamicCast() {
    return this ? Is_(T::Kind_()) ? static_cast<T*>(this) : nullptr : nullptr;
  }

  // [G]
 public:
  virtual const char* GetKind() const = 0;

  // [I]
 public:
  template <class T>
  bool Is() const {
    return this && Is_(T::Kind_());
  }

 public:
  virtual bool Is_(const char*) const { return false; }

  // [S]
 public:
  template <class T>
  T* StaticCast() const {
    return const_cast<T*>(this)->StaticCast<T>();
  }

 public:
  template <class T>
  T* StaticCast() {
    T* p = DynamicCast<T>();
    DCHECK(p);
    return p;
  }
};

template <class Item_, class Parent_>
class DoubleLinkedList_;

class GlobalObject {};
class DummyParent {};

template <class Item_, class Parent_ = DummyParent>
class DoubleLinkedItem_ {
 public:
  DoubleLinkedItem_() : m_pNext(nullptr), m_pPrev(nullptr) {}

  Item_* GetNext() const { return m_pNext; }
  Item_* GetPrev() const { return m_pPrev; }

 private:
  friend class DoubleLinkedList_<Item_, Parent_>;

  Item_* m_pNext;
  Item_* m_pPrev;
};

template <class Item_, class Parent_ = DummyParent>
class DoubleLinkedList_ {
 protected:
  typedef DoubleLinkedList_<Item_, Parent_> List_;

 private:
  typedef DoubleLinkedItem_<Item_, Parent_> Cons_;

 public:
  class Enum {
   public:
    explicit Enum(const List_* p) : m_pRunner(p->m_pFirst) {}

    bool AtEnd() const { return nullptr == m_pRunner; }
    Item_* Get() const { return m_pRunner; }
    void Next() {
      DCHECK(!AtEnd());
      m_pRunner = static_cast<Cons_*>(m_pRunner)->m_pNext;
    }

   private:
    Item_* m_pRunner;
  };

  class EnumReverse {
   public:
    explicit EnumReverse(const List_* p) : m_pRunner(p->m_pLast) {}

    bool AtEnd() const { return nullptr == m_pRunner; }
    Item_* Get() const { return m_pRunner; }
    void Next() {
      DCHECK(!AtEnd());
      m_pRunner = static_cast<Cons_*>(m_pRunner)->m_pPrev;
    }

   private:
    Item_* m_pRunner;
  };

  DoubleLinkedList_() : m_pFirst(nullptr), m_pLast(nullptr) {}

  Item_* Append(Item_* pItem) {
    Cons_* pCons = static_cast<Cons_*>(pItem);

    pCons->m_pNext = nullptr;
    pCons->m_pPrev = m_pLast;

    if (nullptr == m_pFirst) {
      m_pFirst = pItem;
    }

    if (nullptr != m_pLast)
      static_cast<Cons_*>(m_pLast)->m_pNext = pItem;

    return m_pLast = pItem;
  }

  int Count() const {
    auto n = 0;
    for (Enum oEnum(this), !oEnum.AtEnd(); oEnum.Next())
      n += 1;
    return n;
  }

  Item_* Delete(Item_* pItem) {
    Cons_* pCons = static_cast<Cons_*>(pItem);

    Item_* pNext = pCons->m_pNext;
    Item_* pPrev = pCons->m_pPrev;
    if (nullptr == pNext) {
      m_pLast = pPrev;
    } else {
      static_cast<Cons_*>(pNext)->m_pPrev = pPrev;
    }

    if (nullptr == pPrev)
      m_pFirst = pNext;
    else
      static_cast<Cons_*>(pPrev)->m_pNext = pNext;

    pCons->m_pNext = nullptr;
    pCons->m_pPrev = nullptr;

    return pItem;
  }

  void DeleteAll() {
    while (Item_* pItem = GetFirst()) {
      Delete(pItem);
    }
  }

  Item_* GetFirst() const { return m_pFirst; }
  Item_* GetLast() const { return m_pLast; }

  Item_* InsertAfter(Item_* pItem, Item_* pRefItem) {
    Item_* pNext = static_cast<Cons_*>(pRefItem)->m_pNext;
    if (!pNext)
      m_pLast = pItem;
    else
      static_cast<Cons_*>(pNext)->m_pPrev = pItem;

    static_cast<Cons_*>(pItem)->m_pPrev = pRefItem;
    static_cast<Cons_*>(pItem)->m_pNext = pNext;
    static_cast<Cons_*>(pRefItem)->m_pNext = pItem;
    return pItem;
  }

  Item_* InsertBefore(Item_* pItem, Item_* pRefItem) {
    Item_* pPrev = static_cast<Cons_*>(pRefItem)->m_pPrev;
    if (!pPrev)
      m_pFirst = pItem;
    else
      static_cast<Cons_*>(pPrev)->m_pNext = pItem;

    static_cast<Cons_*>(pItem)->m_pPrev = pPrev;
    static_cast<Cons_*>(pItem)->m_pNext = pRefItem;
    static_cast<Cons_*>(pRefItem)->m_pPrev = pItem;
    return pItem;
  }

  bool IsEmpty() const { return nullptr == m_pFirst; }

  Item_* Pop() {
    if (auto pItem = GetFirst())
      return Delete(pItem);
    return nullptr;
  }

  Item_* Prepend(Item_* pItem) {
    Cons_* pCons = static_cast<Cons_*>(pItem);

    pCons->m_pNext = m_pFirst;
    pCons->m_pPrev = nullptr;

    if (nullptr == m_pLast) {
      m_pLast = pItem;
    }

    if (nullptr != m_pFirst) {
      static_cast<Cons_*>(m_pFirst)->m_pPrev = pItem;
    }

    return m_pFirst = pItem;
  }

  Item_* Replace(Item_* pNew, Item_* pOld) {
    InsertBefore(pNew, pOld);
    Delete(pOld);
    return pNew;
  }

 private:
  Item_* m_pFirst;
  Item_* m_pLast;
};

template <class Item_, class Parent_>
class ChildList_;

template <class Item_, class Parent_>
class ChildItem_ : public DoubleLinkedItem_<Item_, Parent_> {
 public:
  // TODO(eval1749) We should not use default parameter.
  explicit ChildItem_(Parent_* p = nullptr) : m_pParent(p) {}

 protected:
  Parent_* m_pParent;

 private:
  friend class ChildList_<Item_, Parent_>;
};

template <class Item_, class Parent_>
class ChildList_ : public DoubleLinkedList_<Item_, Parent_> {
 protected:
  typedef ChildList_<Item_, Parent_> ChildList;
  typedef ChildItem_<Item_, Parent_> ChildItem;

 private:
  typedef DoubleLinkedList_<Item_, Parent_> DoubleLinkedList;

 public:
  Item_* Append(Item_* pItem) {
    DoubleLinkedList::Append(pItem);

    static_cast<ChildItem*>(pItem)->m_pParent = static_cast<Parent_*>(this);

    return pItem;
  }

  Item_* Delete(Item_* pItem) {
    DoubleLinkedList::Delete(pItem);
    static_cast<ChildItem*>(pItem)->m_pParent = nullptr;
    return pItem;
  }

  Item_* InsertAfter(Item_* pItem, Item_* pRefItem) {
    DoubleLinkedList::InsertAfter(pItem, pRefItem);

    static_cast<ChildItem*>(pItem)->m_pParent =
        static_cast<ChildItem*>(pRefItem)->m_pParent;

    return pItem;
  }

  Item_* InsertBefore(Item_* pItem, Item_* pRefItem) {
    DoubleLinkedList::InsertBefore(pItem, pRefItem);

    static_cast<ChildItem*>(pItem)->m_pParent =
        static_cast<ChildItem*>(pRefItem)->m_pParent;

    return pItem;
  }

  Item_* Prepend(Item_* pItem) {
    DoubleLinkedList::Prepend(pItem);

    static_cast<ChildItem*>(pItem)->m_pParent = static_cast<Parent_*>(this);

    return pItem;
  }

  Item_* Replace(Item_* pNew, Item_* pOld) {
    InsertBefore(pNew, pOld);
    Delete(pOld);
    return pNew;
  }
};

/// <remark>
///   C-String enumerator.
/// </remark>
class EnumChar {
  /// <summary>
  ///   Argument for EnumChar
  /// </summary>
 public:
  struct Arg {
    const char16* m_pwch;
    const char16* m_pwchEnd;

    Arg(const char16* pwch, int cwch) : m_pwch(pwch), m_pwchEnd(pwch + cwch) {}
  };

  /// <summary>
  ///   Construct C-String enumerator.
  /// </summary>
  explicit EnumChar(Arg oArg)
      : m_pwch(oArg.m_pwch), m_pwchEnd(oArg.m_pwchEnd) {}

  /// <summary>
  ///  Check enumereator at end.
  /// </summary>
  bool AtEnd() const { return m_pwch >= m_pwchEnd; }

  /// <summary>
  ///  Returns current character
  /// </summary>
  char16 Get() const {
    DCHECK(!AtEnd());
    return *m_pwch;
  }

  /// <summary>
  ///  Advance current position
  /// </summary>
  void Next() {
    DCHECK(!AtEnd());
    m_pwch++;
  }

 private:
  const char16* m_pwch;
  const char16* m_pwchEnd;
};

//////////////////////////////////////////////////////////////////////
//
// LocalHeap
//
class LocalHeap {
 public:
  LocalHeap() : m_hHeap(::HeapCreate(HEAP_NO_SERIALIZE, 0, 0)) {}

  ~LocalHeap() {
    if (nullptr != m_hHeap) {
      ::HeapDestroy(m_hHeap);
    }
  }

  void* Alloc(size_t cb) { return ::HeapAlloc(m_hHeap, 0, cb); }
  void Free(void* pv) { ::HeapFree(m_hHeap, 0, pv); }

 private:
  HANDLE m_hHeap;
};

class LocalObject {
 public:
  void operator delete(void*) = delete;
  void(operator delete[])(void*) = delete;

  void* operator new(size_t cb, LocalHeap* pHeap) { return pHeap->Alloc(cb); }

  void*(operator new[])(size_t cb, LocalHeap* pHeap) {
    return pHeap->Alloc(cb);
  }
};

//////////////////////////////////////////////////////////////////////
//
// CharSink
//
class CharSink final {
 public:
  explicit CharSink(LocalHeap* pHeap)
      : m_pHeap(pHeap),
        m_pwch(m_rgwch),
        m_pwchEnd(m_rgwch + ARRAYSIZE(m_rgwch)),
        m_pwchStart(m_rgwch) {}

  void Add(char16 ch) {
    if (m_pwch + 2 > m_pwchEnd)
      grow();
    *m_pwch++ = ch;
    *m_pwch = 0;
  }

  char16 Get(int iIndex) const {
    DCHECK_GE(iIndex, 0);
    DCHECK_LT(iIndex, GetLength());
    return m_pwchStart[iIndex];
  }
  int GetLength() const { return static_cast<int>(m_pwch - m_pwchStart); }
  const char16* GetStart() const { return m_pwchStart; }
  void Reset() { m_pwch = m_pwchStart; }

  char16* Save(LocalHeap* pHeap) const {
    int cwch = GetLength();
    char16* pwsz =
        reinterpret_cast<char16*>(pHeap->Alloc(sizeof(char16) * (cwch + 1)));
    ::CopyMemory(pwsz, m_pwchStart, sizeof(char16) * cwch);
    pwsz[cwch] = 0;
    return pwsz;
  }

 private:
  void grow() {
    int cwch = GetLength();
    int cwchNew = cwch * 130 / 100;

    char16* pwchNew =
        reinterpret_cast<char16*>(m_pHeap->Alloc(sizeof(char16) * cwchNew));

    CopyMemory(pwchNew, m_pwchStart, sizeof(char16) * cwch);

    m_pwchStart = pwchNew;
    m_pwchEnd = pwchNew + cwchNew;
    m_pwch = pwchNew + cwch;
  }

  LocalHeap* m_pHeap;
  char16* m_pwch;
  char16* m_pwchEnd;
  char16* m_pwchStart;
  char16 m_rgwch[20];
};

//////////////////////////////////////////////////////////////////////
//
// Sink
//
template <class T>
class Sink final {
 public:
  explicit Sink(LocalHeap* pHeap)
      : m_pHeap(pHeap),
        m_pwch(m_rgwch),
        m_pwchEnd(m_rgwch + ARRAYSIZE(m_rgwch)),
        m_pwchStart(m_rgwch) {}

  void Add(T ch) {
    if (m_pwch + 1 > m_pwchEnd) {
      grow();
    }

    *m_pwch++ = ch;
  }

  const T* Get() const { return m_pwchStart; }
  T Get(int iIndex) const {
    DCHECK_GE(iIndex, 0);
    DCHECK_LT(iIndex, GetLength());
    return m_pwchStart[iIndex];
  }

  int GetLength() const { return static_cast<int>(m_pwch - m_pwchStart); }
  size_t GetSize() const { return sizeof(T) * GetLength(); }

  void Serialize(void* pv) const {
    size_t cb = GetLength() * sizeof(T);
    ::CopyMemory(pv, m_pwchStart, cb);
  }

  void Set(int iIndex, T val) {
    DCHECK_GE(iIndex, 0);
    DCHECK_LT(iIndex, GetLength());
    m_pwchStart[iIndex] = val;
  }

  void Shrink() {
    DCHECK_GT(m_pwch, m_pwchStart);
    --m_pwch;
  }

 private:
  void grow() {
    int cwch = GetLength();
    int cwchNew = cwch * 130 / 100;

    T* pwchNew = reinterpret_cast<T*>(m_pHeap->Alloc(sizeof(T) * cwchNew));

    ::CopyMemory(pwchNew, m_pwchStart, sizeof(T) * cwch);

    m_pwchStart = pwchNew;
    m_pwchEnd = pwchNew + cwchNew;
    m_pwch = pwchNew + cwch;
  }

  LocalHeap* m_pHeap;
  T* m_pwch;
  T* m_pwchEnd;
  T* m_pwchStart;
  T m_rgwch[20];
};

char16* lstrchrW(const char16* pwsz, char16 wch);
bool IsWhitespace(char16 wch);

}  // namespace RegexPrivate
}  // namespace Regex

#endif  // REGEX_REGEX_UTIL_H_
