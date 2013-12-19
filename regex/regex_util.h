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
#if !defined(INCLUDE_regex_util_h)
#define INCLUDE_regex_util_h

namespace Regex
{

namespace RegexPrivate
{

template<class T>
class Castable_ {
  private: typedef Castable_<T> Self;

  // [D]
  public: template<class T> T* DynamicCast() const {
    return const_cast<Self*>(this)->DynamicCast<T>();
  }

  public: template<class T> T* DynamicCast() {
    return this
        ? Is_(T::Kind_()) ? static_cast<T*>(this) : nullptr
        : nullptr;
  }

  // [G]
  public: virtual const char* GetKind() const = 0;

  // [I]
  public: template<class T> bool Is() const {
    return this && Is_(T::Kind_());
  }

  public: virtual bool Is_(const char*) const { return false; }

  // [S]
  public: template<class T> T* StaticCast() const {
    return const_cast<T*>(this)->StaticCast<T>();
  }

  public: template<class T> T* StaticCast() {
    T* p = DynamicCast<T>();
    ASSERT(!!p);
    return p;
  }
}; // Castable_

template<class Item_, class Parent_>
class DoubleLinkedList_;

class GlobalObject {};
class DummyParent {};

template<class Item_, class Parent_ = DummyParent>
class DoubleLinkedItem_
{
    friend class DoubleLinkedList_<Item_, Parent_>;

    private: Item_* m_pNext;
    private: Item_* m_pPrev;

    public: DoubleLinkedItem_() :
        m_pNext(NULL),
        m_pPrev(NULL) {}

    public: Item_* GetNext() const { return m_pNext; }
    public: Item_* GetPrev() const { return m_pPrev; }
}; // DoubleLinkedItem_


template<class Item_, class Parent_ = DummyParent>
class DoubleLinkedList_
{
    protected: typedef DoubleLinkedList_<Item_, Parent_> List_;
    private:   typedef DoubleLinkedItem_<Item_, Parent_> Cons_;

    private: Item_* m_pFirst;
    private: Item_* m_pLast;

    public: DoubleLinkedList_() :
        m_pFirst(NULL),
        m_pLast(NULL) {}

    // [A]
    public: Item_* Append(Item_* pItem)
    {
        Cons_* pCons = static_cast<Cons_*>(pItem);

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
    public: int Count() const
    {
        int n = 0;
        foreach (Enum, oEnum, this)
        {
            n += 1;
        } // for
        return n;
    } // Count

    // [D]
    public: Item_* Delete(Item_* pItem)
    {
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

    public: void DeleteAll()
    {
        while (Item_* pItem = GetFirst())
        {
            Delete(pItem);
        } // while
    } // DeleteAll

    // [E]
    public: class Enum
    {
        private: Item_* m_pRunner;
        public: Enum(const List_* p) : m_pRunner(p->m_pFirst) {}
        public: bool AtEnd() const { return NULL == m_pRunner; }
        public: Item_* Get() const { return m_pRunner; }
        public: void Next()
        {
            ASSERT(! AtEnd());
            m_pRunner = static_cast<Cons_*>(m_pRunner)->m_pNext;
        } // Next
    }; // Enum

    public: class EnumReverse
    {
        private: Item_* m_pRunner;
        public: EnumReverse(const List_* p) : m_pRunner(p->m_pLast) {}
        public: bool AtEnd() const { return NULL == m_pRunner; }
        public: Item_* Get() const { return m_pRunner; }
        public: void Next()
        {
            ASSERT(! AtEnd());
            m_pRunner = static_cast<Cons_*>(m_pRunner)->m_pPrev;
        } // Next
    }; // EnumReverse

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

    // [P]
    public: Item_* Pop()
    {
        if (Item_* pItem = GetFirst())
        {
            return Delete(pItem);
        }
        return NULL;
    } // Pop

    public: Item_* Prepend(Item_* pItem)
    {
        Cons_* pCons = static_cast<Cons_*>(pItem);

        pCons->m_pNext = m_pFirst;
        pCons->m_pPrev = NULL;

        if (NULL == m_pLast)
        {
            m_pLast = pItem;
        } // if

        if (NULL != m_pFirst)
        {
            static_cast<Cons_*>(m_pFirst)->m_pPrev = pItem;
        } // if

        return m_pFirst = pItem;
    } // Prepend

    // [R]
    public: Item_* Replace(Item_* pNew, Item_* pOld)
    {
        InsertBefore(pNew, pOld);
        Delete(pOld);
        return pNew;
    } // Replace
}; // DoubleLinkedList_


template<class Item_, class Parent_>
class ChildList_;

template<class Item_, class Parent_>
class ChildItem_ : public DoubleLinkedItem_<Item_, Parent_>
{
    friend class ChildList_<Item_, Parent_>;

    protected: Parent_*   m_pParent;

    public: ChildItem_(Parent_* p = NULL) : m_pParent(p) {}
}; // ChildItem_


template<class Item_, class Parent_>
class ChildList_ : public DoubleLinkedList_<Item_, Parent_>
{
    private: typedef DoubleLinkedList_<Item_, Parent_> DoubleLinkedList;
    protected: typedef ChildList_<Item_, Parent_> ChildList;
    protected: typedef ChildItem_<Item_, Parent_> ChildItem;

    // [A]
    public: Item_* Append(Item_* pItem)
    {
        DoubleLinkedList::Append(pItem);

        static_cast<ChildItem*>(pItem)->m_pParent =
            static_cast<Parent_*>(this);

        return pItem;
    } // Append

    // [D]
    public: Item_* Delete(Item_* pItem)
    {
        DoubleLinkedList::Delete(pItem);
        static_cast<ChildItem*>(pItem)->m_pParent = NULL;
        return pItem;
    } // Delete

    // [I]
    public: Item_* InsertAfter(Item_* pItem, Item_* pRefItem)
    {
        DoubleLinkedList::InsertAfter(pItem, pRefItem);

        static_cast<ChildItem*>(pItem)->m_pParent = 
            static_cast<ChildItem*>(pRefItem)->m_pParent;

        return pItem;
    } // InsertAfter

    public: Item_* InsertBefore(Item_* pItem, Item_* pRefItem)
    {
        DoubleLinkedList::InsertBefore(pItem, pRefItem);

        static_cast<ChildItem*>(pItem)->m_pParent =
            static_cast<ChildItem*>(pRefItem)->m_pParent;

        return pItem;
    } // InsertBefore

    // [P]
    public: Item_* Prepend(Item_* pItem)
    {
        DoubleLinkedList::Prepend(pItem);

        static_cast<ChildItem*>(pItem)->m_pParent =
            static_cast<Parent_*>(this);

        return pItem;
    } // Prepend

    // [R]
    public: Item_* Replace(Item_* pNew, Item_* pOld)
    {
        InsertBefore(pNew, pOld);
        Delete(pOld);
        return pNew;
    } // Replace
};  // ChiildList_

/// <remark>
///   C-String enumerator.
/// </remark>
class EnumChar
{
    /// <summary>
    ///   Argument for EnumChar
    /// </summary>
    public: struct Arg
    {
        const char16*   m_pwch;
        const char16*   m_pwchEnd;

        Arg(
            const char16*   pwch,
            int             cwch ) :
                m_pwch(pwch),
                m_pwchEnd(pwch + cwch) {}
    }; // Arg

    private: const char16*   m_pwch;
    private: const char16*   m_pwchEnd;

    /// <summary>
    ///   Construct C-String enumerator.
    /// </summary>
    public: EnumChar(Arg oArg) :
            m_pwch(oArg.m_pwch),
            m_pwchEnd(oArg.m_pwchEnd) {}

    /// <summary>
    ///  Check enumereator at end.
    /// </summary>
    public: bool AtEnd() const
    {
        return m_pwch >= m_pwchEnd;
    } // AtEnd

    /// <summary>
    ///  Returns current character
    /// </summary>
    public: char16 Get() const
    {
        ASSERT(! AtEnd());
        return *m_pwch;
    } // Get

    /// <summary>
    ///  Advance current position
    /// </summary>
    public: void Next()
    {
        ASSERT(! AtEnd());
        m_pwch++;
    } // AtEnd
}; // EnumChar

//////////////////////////////////////////////////////////////////////
//
// LocalHeap
//
class LocalHeap
{
    private: HANDLE m_hHeap;

    public: LocalHeap() :
        m_hHeap(::HeapCreate(HEAP_NO_SERIALIZE, 0, 0)) {}

    public: ~LocalHeap()
    {
        if (NULL != m_hHeap)
        {
            ::HeapDestroy(m_hHeap);
        }
    } // ~Context

    // [A]
    public: void* Alloc(size_t cb)
        { return ::HeapAlloc(m_hHeap, 0, cb); }

    // [F]
    public: void Free(void* pv)
        { ::HeapFree(m_hHeap, 0, pv); }
}; // LocalHeap

class LocalObject
{
    private: void operator delete(void*) {}
    private: void (operator delete[])(void*) {}

    public: void* operator new(size_t cb, LocalHeap* pHeap)
        { return pHeap->Alloc(cb); }

    public: void* (operator new[])(size_t cb, LocalHeap* pHeap)
        { return pHeap->Alloc(cb); }
}; // LocalObject

//////////////////////////////////////////////////////////////////////
//
// CharSink
//
class CharSink
{
    private: LocalHeap* m_pHeap;
    private: char16*    m_pwch;
    private: char16*    m_pwchEnd;
    private: char16*    m_pwchStart;
    private: char16     m_rgwch[20];

    // ctor
    public: CharSink(LocalHeap* pHeap) :
        m_pHeap(pHeap),
        m_pwch(m_rgwch),
        m_pwchEnd(m_rgwch + ARRAYSIZE(m_rgwch)),
        m_pwchStart(m_rgwch) {}

    // [A]
    public: void Add(char16 ch)
    {
        if (m_pwch + 2 > m_pwchEnd)
        {
            grow();
        }

        *m_pwch++ = ch;
        *m_pwch = 0;
    } // Add

    // [G]
    public: char16 Get(int iIndex) const
    {
        ASSERT(static_cast<uint>(iIndex) < static_cast<uint>(GetLength()));
        return m_pwchStart[iIndex];
    } // Get

    public: int GetLength() const
        { return static_cast<int>(m_pwch - m_pwchStart); }

    public: const char16* GetStart() const
        { return m_pwchStart; }

    private: void grow()
    {
        int cwch = GetLength();
        int cwchNew = cwch * 130 / 100;

        char16* pwchNew = reinterpret_cast<char16*>(
            m_pHeap->Alloc(sizeof(char16) * cwchNew) );

        CopyMemory(pwchNew, m_pwchStart, sizeof(char16) * cwch);

        m_pwchStart = pwchNew;
        m_pwchEnd   = pwchNew + cwchNew;
        m_pwch      = pwchNew + cwch;
    } // grow

    // [R]
    public: void Reset()
        { m_pwch = m_pwchStart; }

    // [S]
    public: char16* Save(LocalHeap* pHeap) const
    {
        int cwch = GetLength();
        char16* pwsz = reinterpret_cast<char16*>(
            pHeap->Alloc(sizeof(char16) * (cwch + 1)) );
        ::CopyMemory(pwsz, m_pwchStart, sizeof(char16) * cwch);
        pwsz[cwch] = 0;
        return pwsz;
    } // Save

    #if 0
        public: char16* Save() const
        {
            int cwch = GetLength();
            char16* pwsz = new char16[cwch + 1];
            ::CopyMemory(pwsz, m_pwchStart, sizeof(char16) * cwch);
            pwsz[cwch] = 0;
            return pwsz;
        } // Get
    #endif

    public: void xShrink()
    {
        ASSERT(m_pwch > m_pwchStart);
        --m_pwch;
    } // Shrink
}; // CharSink

//////////////////////////////////////////////////////////////////////
//
// Sink
//
template<class T>
class Sink
{
    private: LocalHeap* m_pHeap;
    private: T*         m_pwch;
    private: T*         m_pwchEnd;
    private: T*         m_pwchStart;
    private: T          m_rgwch[20];

    // ctor
    public: Sink(LocalHeap* pHeap) :
        m_pHeap(pHeap),
        m_pwch(m_rgwch),
        m_pwchEnd(m_rgwch + ARRAYSIZE(m_rgwch)),
        m_pwchStart(m_rgwch) {}

    // [A]
    public: void Add(T ch)
    {
        if (m_pwch + 1 > m_pwchEnd)
        {
            grow();
        }

        *m_pwch++ = ch;
    } // Add

    // [G]
    public: const T* Get() const
        { return m_pwchStart; }

    public: T Get(int iIndex) const
    {
        ASSERT(iIndex < GetLength());
        return m_pwchStart[iIndex];
    } // Get

    public: int GetLength() const
        { return static_cast<int>(m_pwch - m_pwchStart); }

    public: size_t GetSize() const
        { return sizeof(T) * GetLength(); }

    private: void grow()
    {
        int cwch = GetLength();
        int cwchNew = cwch * 130 / 100;

        T* pwchNew = reinterpret_cast<T*>(
            m_pHeap->Alloc(sizeof(T) * cwchNew) );

        ::CopyMemory(pwchNew, m_pwchStart, sizeof(T) * cwch);

        m_pwchStart = pwchNew;
        m_pwchEnd   = pwchNew + cwchNew;
        m_pwch      = pwchNew + cwch;
    } // grow

    // [S]
    public: void Serialize(void* pv) const
    {
        size_t cb = GetLength() * sizeof(T);
        ::CopyMemory(pv, m_pwchStart, cb);
    } // Serialize

    public: void Set(int iIndex, T val)
    {
        ASSERT(iIndex < GetLength());
        m_pwchStart[iIndex] = val;
    } // Set

    public: void Shrink()
    {
        ASSERT(m_pwch > m_pwchStart);
        --m_pwch;
    } // Shrink
}; // Sink


char16* lstrchrW(const char16* pwsz, char16 wch);
bool IsWhitespace(char16 wch);

} // RegexPrivate
} // Regex

#endif // !defined(INCLUDE_regex_util_h)
