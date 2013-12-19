//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_util.h#1 $
//
#if !defined(INCLUDE_listener_winapp_editor_util_h)
#define INCLUDE_listener_winapp_editor_util_h

//////////////////////////////////////////////////////////////////////
//
// CharSink_
//
template<int t_N = 80>
class LocalCharSink_
{
    HANDLE  m_hObjHeap;
    char16* m_p;
    char16* m_pwch;
    uint    m_cAlloc;
    char16  m_rgwch[t_N];

    // CharSink
    public: LocalCharSink_(HANDLE hHeap) :
        m_hObjHeap(hHeap),
        m_pwch(m_rgwch),
        m_p(m_rgwch),
        m_cAlloc(lengthof(m_rgwch)) {}

    // [A]
    public: void Add(char16 wch)
        { Add(&wch, 1); }

    public: void Add(const char16* pwch, uint cwch)
    {
        if (m_p + cwch >= m_pwch + m_cAlloc)
        {
            alloc(130, cwch);
        }

        ::CopyMemory(m_p, pwch, sizeof(char16) * cwch);
        m_p += cwch;
    } // Add

    private: void alloc(uint n, uint k)
    {
        uint cwch = static_cast<uint>(m_p - m_pwch);

        uint cAlloc = (cwch + k) * n / 100;

        char16* pwch = reinterpret_cast<char16*>(
            ::HeapAlloc(m_hObjHeap, 0, sizeof(char16) * cAlloc) );

        ::CopyMemory(pwch, m_pwch, sizeof(char16) * cwch);

        if (m_pwch != m_rgwch) ::HeapFree(m_hObjHeap, 0, m_pwch);

        m_cAlloc = cAlloc;
        m_pwch = pwch;
        m_p = m_pwch + cwch;
    } // alloc

    // [G]
    public: uint GetLength() const
        { return static_cast<uint>(m_p - m_pwch); }

    public: const char16* GetStart() const
        { return m_pwch; }

    // [F]
    public: char16* Fix()
    {
        if (m_p == m_pwch) return NULL;

        if (m_pwch == m_rgwch)
        {
            alloc(100, 0);
        } // if

        return m_pwch;
    } // Fix

    // [R]
    public: void Reset()
    {
        m_cAlloc = lengthof(m_rgwch);
        m_pwch   = m_rgwch;
        m_p      = m_pwch;
    } // Reset
}; // LocalCharSink_

#endif //!defined(INCLUDE_listener_winapp_editor_util_h)
