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
#if !defined(INCLUDE_listener_util_h)
#define INCLUDE_listener_util_h

#include "base/logging.h"

template<class Base_>
class Castable_
{
    private: typedef Castable_<Base_> Self;

    // [D]
    public: template<class T> T* DynamicCast() const
    {
        Base_* p = static_cast<Base_*>(const_cast<Self*>(this));
        // warning C4946: reinterpret_cast used between related classes: 
        // 'class1' and 'class2'
        #pragma warning(suppress: 4946)
        return T::Is_(p) ? reinterpret_cast<T*>(p) : NULL;
    } // DynamicCast

    // [S]
    public: template<class T> T* StaticCast() const
    {
        T* p = DynamicCast<T>();
        DCHECK(NULL != p);
        return p;
    } // StaticCast
}; // Castable_

template<class Item_, class Parent_>
class DoubleLinkedList_;

class DummyParent {};

template<class Item_, class Parent_ = DummyParent>
class DoubleLinkedNode_
{
    friend class DoubleLinkedList_<Item_, Parent_>;

    private: Item_* m_pNext;
    private: Item_* m_pPrev;

    public: DoubleLinkedNode_() :
        m_pNext(NULL),
        m_pPrev(NULL) {}

    public: Item_* GetNext() const { return m_pNext; }
    public: Item_* GetPrev() const { return m_pPrev; }
}; // DoubleLinkedNode_


template<class Item_, class Parent_ = DummyParent>
class DoubleLinkedList_
{
    protected: typedef DoubleLinkedList_<Item_, Parent_> List_;
    private: typedef DoubleLinkedNode_<Item_, Parent_> Cons_;

    private: Item_* m_pFirst;
    private: Item_* m_pLast;

    public: DoubleLinkedList_() :
        m_pFirst(NULL),
        m_pLast(NULL) {}

    // [A]
    public: Item_* Append(Item_* pItem)
    {
        Cons_* pCons = static_cast<Cons_*>(pItem);
        DCHECK(!pCons->m_pNext);
        DCHECK(!pCons->m_pPrev);

        pCons->m_pNext = NULL;
        pCons->m_pPrev = m_pLast;

        if (NULL == m_pFirst)
        {
            m_pFirst = pItem;
        } // if

        if (NULL != m_pLast)
        {
            static_cast<Cons_*>(m_pLast)->m_pNext = pItem;
        } // if

        return m_pLast = pItem;
    } // Append

    // [C]
    public: int Count() const {
      auto num_items = 0;
      foreach (Enum, it, this) {
        ++num_items;
      }
      return num_items;
    }

    // [D]
    public: Item_* Delete(Item_* pItem)
    {
        #if _DEBUG
        {
            bool fFound = false;
            foreach (Enum, oEnum, this)
            {
                if (oEnum.Get() == pItem)
                {
                    fFound = true;
                    break;
                }
            } // for each item
            DCHECK(fFound);
        }
        #endif

        Cons_* pCons = static_cast<Cons_*>(pItem);

        Item_* pNext = pCons->m_pNext;
        Item_* pPrev = pCons->m_pPrev;
        if (NULL == pNext)
        {
            m_pLast = pPrev;
        }
        else
        {
            static_cast<Cons_*>(pNext)->m_pPrev = pPrev;
        } // if

        if (NULL == pPrev)
        {
            m_pFirst = pNext;
        }
        else
        {
            static_cast<Cons_*>(pPrev)->m_pNext = pNext;
        } // if

        pCons->m_pNext = NULL;
        pCons->m_pPrev = NULL;

        return pItem;
    } // Delete

    public: void DeleteAll() {
      m_pFirst = m_pLast = nullptr;
    }

    // [E]
    public: class Enum
    {
        private: Item_* m_pRunner;
        public: Enum(List_& r) : m_pRunner(r.m_pFirst) {}
        public: Enum(const List_& r) : m_pRunner(r.m_pFirst) {}
        public: Enum(List_* p) : m_pRunner(p->m_pFirst) {}
        public: Enum(const List_* p) : m_pRunner(p->m_pFirst) {}
        public: Item_* operator ->() { return Get(); }
        public: bool AtEnd() const { return m_pRunner == NULL; }
        public: Item_* Get() { return m_pRunner; }
        public: void Next()
        {
            DCHECK(! AtEnd());
            m_pRunner = static_cast<Cons_*>(m_pRunner)->m_pNext;
        } // Next
    }; // Enum

    // [G]
    public: Item_* GetFirst() const { return m_pFirst; }
    public: Item_* GetLast()  const { return m_pLast; }

    // [I]
    public: Item_* InsertAfter(Item_* pItem, Item_* pRefItem)
    {
        Item_* pNext = static_cast<Cons_*>(pRefItem)->m_pNext;
        if (NULL == pNext)
        {
            m_pLast = pItem;
        }
        else
        {
            static_cast<Cons_*>(pNext)->m_pPrev = pItem;
        }

        static_cast<Cons_*>(pItem)->m_pPrev    = pRefItem;
        static_cast<Cons_*>(pItem)->m_pNext    = pNext;
        static_cast<Cons_*>(pRefItem)->m_pNext = pItem;
        return pItem;
    } // InsertAfter

    public: Item_* InsertBefore(Item_* pItem, Item_* pRefItem)
    {
        Item_* pPrev = static_cast<Cons_*>(pRefItem)->m_pPrev;
        if (NULL == pPrev)
        {
            m_pFirst = pItem;
        }
        else
        {
            static_cast<Cons_*>(pPrev)->m_pNext = pItem;
        }

        static_cast<Cons_*>(pItem)->m_pPrev    = pPrev;
        static_cast<Cons_*>(pItem)->m_pNext    = pRefItem;
        static_cast<Cons_*>(pRefItem)->m_pPrev = pItem;
        return pItem;
    } // InsertBefore

    public: bool IsEmpty() const
    {
        return NULL == m_pFirst;
    } // IsEmpty

    // Iterator
    private: template<class IteratorT_, typename Return_>
        class Iterator_ {
      protected: Item_* item_;
      protected: List_* list_;

      protected: Iterator_(List_& list, Item_* item)
          : item_(item), list_(&list) {
      }

      public: bool operator==(const IteratorT_& other) const {
        DCHECK(list_ == other.list_);
        return item_ == other.item_;
      }

      public: bool operator!=(const IteratorT_& other) const {
        return !operator==(other);
      }

      public: Return_& operator*() const {
        DCHECK(item_);
        return *item_;
      }

      public: Return_* operator->() const {
        DCHECK(item_);
        return item_;
      }

      protected: void decrement() {
        DCHECK(item_);
        item_ = static_cast<Cons_*>(item_)->m_pPrev;
      }

      protected: void increment() {
        DCHECK(item_);
        item_ = static_cast<Cons_*>(item_)->m_pNext;
      }
    };

    public: class Iterator : public Iterator_<Iterator, Item_> {
      public: Iterator(List_& list, Item_* item)
          : Iterator_(list, item) {
      }
      public: Iterator& operator--() {
        decrement();
        return *this;
      }
      public: Iterator& operator++() {
        increment();
        return *this;
      }
    };

    public: class ConstIterator
        : public Iterator_<ConstIterator, const Item_> {
      public: ConstIterator(const List_& list, Item_* item)
          : Iterator_(const_cast<List_&>(list), item) {
      }
      public: ConstIterator& operator--() {
        decrement();
        return *this;
      }
      public: ConstIterator& operator++() {
        increment();
        return *this;
      }
    };

    Iterator begin() { return Iterator(*this, m_pFirst); }
    Iterator end() { return Iterator(*this, nullptr); }
    ConstIterator begin() const { return ConstIterator(*this, m_pFirst); }
    ConstIterator end() const { return ConstIterator(*this, nullptr); }

    public: Item_* Prepend(Item_* item) {
      auto const cons = static_cast<Cons_*>(item);
      DCHECK(!cons->m_pNext);
      DCHECK(!cons->m_pPrev);
      cons->m_pPrev = nullptr;
      cons ->m_pNext = m_pFirst;
      if (!m_pLast)
        m_pLast = item;
      if (m_pFirst)
        static_cast<Cons_*>(m_pFirst)->m_pPrev= item;
      return m_pFirst = item;
    }
}; // DoubleLinkedList_


template<class Parent_, class Item_>
class ChildList_;

template<class Parent_, class Item_>
class ChildNode_ : public DoubleLinkedNode_<Item_, Parent_>
{
    friend class ChildList_<Parent_, Item_>;

    protected: Parent_*   m_pParent;

    public: ChildNode_() : m_pParent(NULL) {}
}; // ChildNode_


template<class Parent_, class Item_>
class ChildList_ : public DoubleLinkedList_<Item_, Parent_>
{
    protected: typedef DoubleLinkedList_<Item_, Parent_> List;
    protected: typedef ChildList_<Parent_, Item_> ChildList;

    public: Item_* Append(Parent_* pParent, Item_* pItem)
    {
        List::Append(pItem);
        pItem->m_pParent = pParent;
        return pItem;
    } // Append
};  // ChiildList_

#endif //!defined(INCLUDE_listener_util_h)
