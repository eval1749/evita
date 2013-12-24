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
        ASSERT(NULL != p);
        return p;
    } // StaticCast
}; // Castable_

template<class T, class Base_, typename KindType_ = const char*>
class HasKind_ : public Base_
{
    public: virtual KindType_ GetKind() const override
        { return T::Kind_(); }

    public: static bool Is_(const Base_* p)
        { return p->GetKind() == T::Kind_(); }
}; // HasKind_

template<int t_N = 100>
class CharSink_
{
    private: int     m_cwch;
    private: char16* m_pwsz;
    private: char16* m_pwszBuffer;
    private: char16  m_wsz[t_N];

    // ctor
    public: CharSink_() :
        m_cwch(t_N),
        m_pwsz(m_wsz),
        m_pwszBuffer(m_wsz) {}

    // dtor
    public: ~CharSink_()
        { releaseBuffer(); }

    // [A]
    public: void Add(char16 wch)
    {
        if (m_pwsz - m_pwszBuffer >= m_cwch)
        {
            auto const cwch = m_cwch * 13 / 10;
            char16* pwszNew = new char16[static_cast<size_t>(cwch)];
            myCopyMemory(pwszNew, m_pwsz, sizeof(char16) * m_cwch);
            releaseBuffer();
            m_pwszBuffer = pwszNew;
            m_pwsz       = pwszNew + m_cwch;
            m_cwch       = cwch;
        }

        *m_pwsz++ = wch;
    } // Add

    public: void Add(const char16* s)
    {
        while (0 != *s) Add(*s++);
    } // Add

    // [G]
    public: const char16* GetStart() const
        { return m_pwszBuffer; }

    // [R]
    protected: void releaseBuffer()
    {
        if (m_wsz != m_pwszBuffer)
        {
            delete[] m_pwszBuffer;
        }
    } // releaseBuffer
}; // CharSink_


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
        ASSERT(!pCons->m_pNext);
        ASSERT(!pCons->m_pPrev);

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
            ASSERT(fFound);
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
            ASSERT(! AtEnd());
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
        ASSERT(list_ == other.list_);
        return item_ == other.item_;
      }

      public: bool operator!=(const IteratorT_& other) const {
        return !operator==(other);
      }

      public: Return_& operator*() const {
        ASSERT(item_);
        return *item_;
      }

      public: Return_* operator->() const {
        ASSERT(item_);
        return item_;
      }

      protected: void decrement() {
        ASSERT(item_);
        item_ = static_cast<Cons_*>(item_)->m_pPrev;
      }

      protected: void increment() {
        ASSERT(item_);
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
      ASSERT(!cons->m_pNext);
      ASSERT(!cons->m_pPrev);
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


//////////////////////////////////////////////////////////////////////
//
// StringKey
//
class StringKey
{
    private: int            m_cwch;
    private: const char16*  m_pwch;

    public: StringKey(const char16* pwsz) :
        m_cwch(::lstrlenW(pwsz)),
        m_pwch(pwsz) {}

    public: StringKey(const char16* pwch, int cwch) :
        m_cwch(cwch),
        m_pwch(pwch) {}

    public: bool EqualKey(const StringKey* p) const
    {
        if (m_cwch != p->m_cwch) return false;
        return 0 == ::memcmp(m_pwch, p->m_pwch, sizeof(char16) * m_cwch);
    } // Equal

    // Hash - returns hash code.
    public: int Hash() const
    {
        uint nHashCode = 0;
        const char16* s = m_pwch;
        const char16* e = s + m_cwch;
        for (const char16* p = s; p < e; p++)
        {
            uint nHigh = nHashCode >> (sizeof(uint) * 8 - 5);
            nHashCode |= *p;
            nHashCode <<= 5;
            nHashCode |= nHigh;
        } // for p
        return static_cast<int>(nHashCode & ((1<<28)-1)) & MAXINT;
    } // Hash

    public: static StringKey* Removed()
        { return reinterpret_cast<StringKey*>(1); }
}; // StringKey


//////////////////////////////////////////////////////////////////////
//
// HashTable_
//
template<class Key_, typename Value_, int t_N = 31>
class HashTable_
{
    private: struct Slot
    {
        const Key_* m_pKey;
        Value_      m_Value;
    }; // Slot

    private: Slot*  m_prgSlot;
    private: int    m_cAlloc;
    private: int    m_cItems;

    public: HashTable_() :
        m_cAlloc(t_N),
        m_cItems(0),
        m_prgSlot(new Slot[t_N])
    {
        ::ZeroMemory(m_prgSlot, sizeof(Slot) * m_cAlloc);
    } // Cache_
        
    public: Value_* Get(const Key_* pKey) const
    {
        int iHashCode = pKey->Hash();
        const Slot* pTop    = &m_prgSlot[0];
        const Slot* pBottom = &m_prgSlot[m_cAlloc];
        const Slot* pStart  = &m_prgSlot[iHashCode % m_cAlloc];
        const Slot* pRunner = pStart;
        do
        {
            const Key_* pPresent = pRunner->m_pKey;
            if (NULL == pPresent)
            {
                return NULL;
            }

            if (Key_::Removed() == pPresent)
            {
                // removed
            }
            else if (pPresent->EqualKey(pKey))
            {
                return const_cast<Value_*>(&pRunner->m_Value);
            }

            pRunner++;
            if (pRunner == pBottom) pRunner = pTop;
        } while (pRunner != pStart);
        CAN_NOT_HAPPEN();
    } // Get

    public: void Put(const Key_* pKey, Value_ oValue)
    {
        if (m_cItems * 100 > m_cAlloc * 60)
        {
            rehash();
        }

        int iHashCode = pKey->Hash();
        Slot* pTop    = &m_prgSlot[0];
        Slot* pBottom = &m_prgSlot[m_cAlloc];
        Slot* pStart  = &m_prgSlot[iHashCode % m_cAlloc];
        Slot* pRunner = pStart;
        Slot* pHome = NULL;

        do
        {
            const Key_* pPresent = pRunner->m_pKey;
            if (NULL == pPresent)
            {
                if (NULL == pHome) pHome = pRunner;
                pHome->m_pKey  = pKey;
                pHome->m_Value = oValue;
                m_cItems += 1;
                return;
            }

            if (Key_::Removed() == pPresent)
            {
                if (NULL == pHome) pHome = pRunner;
            }
            else if (pPresent->EqualKey(pKey))
            {
                return;
            }

            pRunner++;
            if (pRunner == pBottom) pRunner = pTop;
        } while (pRunner != pStart);
        CAN_NOT_HAPPEN();
    } // Put

    private: void rehash()
    {
        Slot* prgStart = m_prgSlot;
        auto const cAllocs = m_cAlloc;
        auto cItems  = m_cItems;
        
        m_cAlloc = m_cAlloc * 130 / 100;
        m_cItems  = 0;
        m_prgSlot = new Slot[static_cast<size_t>(m_cAlloc)];
        ::ZeroMemory(m_prgSlot, sizeof(Slot) * m_cAlloc);

        Slot* prgEnd = prgStart + cAllocs;
        for (Slot* pRunner = prgStart; pRunner < prgEnd; pRunner++)
        {
            const Key_* pPresent = pRunner->m_pKey;
            if (NULL != pPresent && Key_::Removed() != pPresent)
            {
                Put(pRunner->m_pKey, pRunner->m_Value);
                cItems -= 1;
                if (0 == cItems) break;
            }
        } // for pRunner
    } // rehash
}; // HashTable_

// TODO(yosi) We should use std::unique_ptr<T> instead of OwnPtr<T>.
namespace common {
template<class T>
class OwnPtr {
  private: T* ptr_;
  public: OwnPtr() : ptr_(nullptr) {}
  public: explicit OwnPtr(T* ptr) : ptr_(ptr) { ASSERT(ptr_); }
  public: explicit OwnPtr(T& obj) : ptr_(&obj) {}
  public: OwnPtr(OwnPtr& other) : ptr_(other.ptr_) {}
  public: OwnPtr(OwnPtr&& other) : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }
  public: ~OwnPtr() { delete ptr_; }
  public: operator T*() const { ASSERT(ptr_); return ptr_; }
  public: T* operator ->() const { ASSERT(ptr_); return ptr_; }
  public: T& operator*() const { ASSERT(ptr_); return *ptr_; }
  public: operator bool() const { return ptr_; }
  public: bool operator!() const { return !ptr_; }
  public: OwnPtr& operator=(OwnPtr&) = delete;
  public: OwnPtr& operator=(OwnPtr&& other) {
    delete ptr_;
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
    return *this;
  }
  public: bool operator==(const OwnPtr<T>& other) const {
    return ptr_ == other.ptr_;
  }
  public: bool operator!=(const OwnPtr<T>& other) const {
    return ptr_ == other.ptr_;
  }
  public: bool operator==(const T* other) const { return ptr_ == other; }
  public: bool operator!=(const T* other) const { return ptr_ != other; }
  public: bool operator==(const T& other) const { return ptr_ == &other; }
  public: bool operator!=(const T& other) const { return ptr_ != &other; }
  public: T& DeprecatedDetach() {
    auto const ptr = ptr_;
    ptr_ = nullptr;
    return *ptr;
  }
};
}  // namespace common

// RefCounted
template<class T>
class RefCounted_ {
  private: mutable int ref_count_;
  protected: RefCounted_() : ref_count_(0) {}
  public: ~RefCounted_() {
    ASSERT(!ref_count_);
  }
  public: int ref_count() const { return ref_count_; }
  public: void AddRef() const { ++ref_count_; }
  public: void Release() const {
    ASSERT(ref_count_ >= 1);
    --ref_count_;
    if (!ref_count_) {
      delete static_cast<T*>(const_cast<RefCounted_*>(this));
    }
  }
};

template<class T> class ScopedRefCount_ {
  private: T* const pointer_;
  public: ScopedRefCount_(T& object) : pointer_(&object) { object.AddRef(); }
  public: ~ScopedRefCount_() { if (pointer_) pointer_->Release(); }
  public: operator T*() const { return pointer_; }
  public: T* operator->() const { return pointer_; }
  public: T& operator*() const { return *pointer_; }

  public: T& Detach() {
    ASSERT(!!pointer_);
    auto const retval = pointer_;
    pointer_ = nullptr;
    return retval;
  }

  DISALLOW_COPY_AND_ASSIGN(ScopedRefCount_);
};

#endif //!defined(INCLUDE_listener_util_h)
