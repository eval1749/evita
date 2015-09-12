//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/li_util.h#1 $
//
#ifndef EVITA_LI_UTIL_H_
#define EVITA_LI_UTIL_H_

#include "base/logging.h"

template <class Base_>
class Castable_ {
 public:
  template <class T>
  T* DynamicCast() const {
    Base_* p = static_cast<Base_*>(const_cast<Self*>(this));
// warning C4946: reinterpret_cast used between related classes:
// 'class1' and 'class2'
#pragma warning(suppress : 4946)
    return T::Is_(p) ? reinterpret_cast<T*>(p) : nullptr;
  }

  template <class T>
  T* StaticCast() const {
    T* p = DynamicCast<T>();
    DCHECK(nullptr != p);
    return p;
  }

 private:
  typedef Castable_<Base_> Self;
};

template <class Item_, class Parent_>
class DoubleLinkedList_;

class DummyParent {};

template <class Item_, class Parent_ = DummyParent>
class DoubleLinkedNode_ {
 public:
  DoubleLinkedNode_() : m_pNext(nullptr), m_pPrev(nullptr) {}

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

 public:
  DoubleLinkedList_() : m_pFirst(nullptr), m_pLast(nullptr) {}

  Item_* Append(Item_* pItem) {
    Cons_* pCons = static_cast<Cons_*>(pItem);
    DCHECK(!pCons->m_pNext);
    DCHECK(!pCons->m_pPrev);

    pCons->m_pNext = nullptr;
    pCons->m_pPrev = m_pLast;

    if (nullptr == m_pFirst)
      m_pFirst = pItem;

    if (nullptr != m_pLast)
      static_cast<Cons_*>(m_pLast)->m_pNext = pItem;

    return m_pLast = pItem;
  }

  int Count() const {
    auto num_items = 0;
    for (Enum it(this); !it.AtEnd(); it.Next())
      ++num_items;
    return num_items;
  }

  Item_* Delete(Item_* pItem) {
#if _DEBUG
    {
      bool fFound = false;
      for (Enum oEnum(this); !oEnum.AtEnd(); oEnum.Next()) {
        if (oEnum.Get() == pItem) {
          fFound = true;
          break;
        }
      }
      DCHECK(fFound);
    }
#endif

    Cons_* pCons = static_cast<Cons_*>(pItem);

    Item_* pNext = pCons->m_pNext;
    Item_* pPrev = pCons->m_pPrev;
    if (nullptr == pNext) {
      m_pLast = pPrev;
    } else {
      static_cast<Cons_*>(pNext)->m_pPrev = pPrev;
    }

    if (nullptr == pPrev) {
      m_pFirst = pNext;
    } else {
      static_cast<Cons_*>(pPrev)->m_pNext = pNext;
    }

    pCons->m_pNext = nullptr;
    pCons->m_pPrev = nullptr;

    return pItem;
  }

  void DeleteAll() { m_pFirst = m_pLast = nullptr; }

  class Enum final {
   public:
    explicit Enum(List_& r) : m_pRunner(r.m_pFirst) {}
    explicit Enum(const List_& r) : m_pRunner(r.m_pFirst) {}
    explicit Enum(List_* p) : m_pRunner(p->m_pFirst) {}
    explicit Enum(const List_* p) : m_pRunner(p->m_pFirst) {}
    Item_* operator->() { return Get(); }
    bool AtEnd() const { return m_pRunner == nullptr; }
    Item_* Get() { return m_pRunner; }
    void Next() {
      DCHECK(!AtEnd());
      m_pRunner = static_cast<Cons_*>(m_pRunner)->m_pNext;
    }  // Next

   private:
    Item_* m_pRunner;
  };

  Item_* GetFirst() const { return m_pFirst; }
  Item_* GetLast() const { return m_pLast; }

  Item_* InsertAfter(Item_* pItem, Item_* pRefItem) {
    Item_* pNext = static_cast<Cons_*>(pRefItem)->m_pNext;
    if (nullptr == pNext) {
      m_pLast = pItem;
    } else {
      static_cast<Cons_*>(pNext)->m_pPrev = pItem;
    }

    static_cast<Cons_*>(pItem)->m_pPrev = pRefItem;
    static_cast<Cons_*>(pItem)->m_pNext = pNext;
    static_cast<Cons_*>(pRefItem)->m_pNext = pItem;
    return pItem;
  }

  Item_* InsertBefore(Item_* pItem, Item_* pRefItem) {
    Item_* pPrev = static_cast<Cons_*>(pRefItem)->m_pPrev;
    if (nullptr == pPrev) {
      m_pFirst = pItem;
    } else {
      static_cast<Cons_*>(pPrev)->m_pNext = pItem;
    }

    static_cast<Cons_*>(pItem)->m_pPrev = pPrev;
    static_cast<Cons_*>(pItem)->m_pNext = pRefItem;
    static_cast<Cons_*>(pRefItem)->m_pPrev = pItem;
    return pItem;
  }

  bool IsEmpty() const { return nullptr == m_pFirst; }  // IsEmpty

  // Iterator
  template <class IteratorT_, typename Return_>
  class Iterator_ {
   public:
    bool operator==(const IteratorT_& other) const {
      DCHECK(list_ == other.list_);
      return item_ == other.item_;
    }

   public:
    bool operator!=(const IteratorT_& other) const {
      return !operator==(other);
    }

   public:
    Return_& operator*() const {
      DCHECK(item_);
      return *item_;
    }

   public:
    Return_* operator->() const {
      DCHECK(item_);
      return item_;
    }

   protected:
    Iterator_(List_& list, Item_* item) : item_(item), list_(&list) {}

    void decrement() {
      DCHECK(item_);
      item_ = static_cast<Cons_*>(item_)->m_pPrev;
    }

    void increment() {
      DCHECK(item_);
      item_ = static_cast<Cons_*>(item_)->m_pNext;
    }

    Item_* item_;
    List_* list_;
  };

  class Iterator final : public Iterator_<Iterator, Item_> {
   public:
    Iterator(List_& list, Item_* item) : Iterator_(list, item) {}

    Iterator& operator--() {
      decrement();
      return *this;
    }

    Iterator& operator++() {
      increment();
      return *this;
    }
  };

  class ConstIterator final : public Iterator_<ConstIterator, const Item_> {
   public:
    ConstIterator(const List_& list, Item_* item)
        : Iterator_(const_cast<List_&>(list), item) {}

    ConstIterator& operator--() {
      decrement();
      return *this;
    }

    ConstIterator& operator++() {
      increment();
      return *this;
    }
  };

  Iterator begin() { return Iterator(*this, m_pFirst); }
  Iterator end() { return Iterator(*this, nullptr); }
  ConstIterator begin() const { return ConstIterator(*this, m_pFirst); }
  ConstIterator end() const { return ConstIterator(*this, nullptr); }

  Item_* Prepend(Item_* item) {
    auto const cons = static_cast<Cons_*>(item);
    DCHECK(!cons->m_pNext);
    DCHECK(!cons->m_pPrev);
    cons->m_pPrev = nullptr;
    cons->m_pNext = m_pFirst;
    if (!m_pLast)
      m_pLast = item;
    if (m_pFirst)
      static_cast<Cons_*>(m_pFirst)->m_pPrev = item;
    return m_pFirst = item;
  }

 private:
  typedef DoubleLinkedNode_<Item_, Parent_> Cons_;

  Item_* m_pFirst;
  Item_* m_pLast;
};

template <class Parent_, class Item_>
class ChildList_;

template <class Parent_, class Item_>
class ChildNode_ : public DoubleLinkedNode_<Item_, Parent_> {
 public:
  ChildNode_() : m_pParent(nullptr) {}

 protected:
  Parent_* m_pParent;

 private:
  friend class ChildList_<Parent_, Item_>;
};

template <class Parent_, class Item_>
class ChildList_ : public DoubleLinkedList_<Item_, Parent_> {
 public:
  Item_* Append(Parent_* pParent, Item_* pItem) {
    List::Append(pItem);
    pItem->m_pParent = pParent;
    return pItem;
  }
 protected:
  typedef DoubleLinkedList_<Item_, Parent_> List;
  typedef ChildList_<Parent_, Item_> ChildList;
};

#endif  // EVITA_LI_UTIL_H_
