#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - listener pane
// listener/winapp/ap_lisp_thread.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ap_lisp_thread.cpp#2 $
//
#include "./ap_lisp_thread.h"

#include "../../platform/win/ILispEngine.h"

#include "./listener.h"

static ILispEngine* sm_pIEngine;

// LispThread ctor
LispThread::LispThread() :
    m_dwThread(0),
    m_hDll(NULL),
    m_hThread(NULL),
    m_hwnd(NULL),
    m_pvLisp(NULL) {}


//////////////////////////////////////////////////////////////////////
//
// LispThread dtor
//
LispThread::~LispThread()
{
    if (m_hThread != NULL) ::CloseHandle(m_hThread);
} // LispThread::~LispThread


namespace
{

char16 const k_wszTitle[] = L"Evita Common Lisp";

//////////////////////////////////////////////////////////////////////
//
// EnumArg
//  Enumerates command line arguments.
//
class EnumArg
{
    enum { MAX_WORD_LEN = MAX_PATH };

    enum State
    {
        State_Start,
    }; // State

    LPCWSTR m_pwszRunner;
    char16 m_wsz[MAX_WORD_LEN];

    public: EnumArg(LPCWSTR pwsz) :
        m_pwszRunner(pwsz)
      { next(); }

    public: bool AtEnd() const
        { return 0 == *m_pwszRunner && 0 == *m_wsz; }

    public: LPCWSTR Get() const
        { ASSERT(! AtEnd()); return m_wsz; }

    public: void Next()
        { ASSERT(! AtEnd()); next(); }

    static bool isspace(char16 wch)
        { return ' ' == wch || '\t' == wch; }

    void next()
    {
        while (isspace(*m_pwszRunner)) m_pwszRunner++;
        char16* pwsz = m_wsz;
        if (0x22 != *m_pwszRunner)
        {
            while (0 != *m_pwszRunner)
            {
                if (isspace(*m_pwszRunner)) break;
                *pwsz++ = *m_pwszRunner++;
            } // while
        }
        else
        {
            m_pwszRunner++;
            while (0 != *m_pwszRunner)
            {
                if (0x22 == *m_pwszRunner)
                {
                    m_pwszRunner++;
                    break;
                }
                *pwsz++ = *m_pwszRunner++;
            } // while
        } // if
        *pwsz = 0;
    } // next
}; // EnumArg

} // namespace


//////////////////////////////////////////////////////////////////////
//
// LispThread::Init
//
uint LispThread::Init()
{
    if (m_hThread != NULL)
    {
        return 0;
    } // if

    if (sm_pIEngine == NULL)
    {
        char16 wszDll[MAX_PATH];
        wszDll[0] = 0;

        char16 wszImage[MAX_PATH];
        wszImage[0] = 0;

        foreach (EnumArg, oEnum, ::GetCommandLine())
        {
            if (::lstrcmpW(oEnum.Get(), L"-image") == 0)
            {
                oEnum.Next();
                if (! oEnum.AtEnd()) ::lstrcpyW(wszImage, oEnum.Get());
            }
            else if (::lstrcmpW(oEnum.Get(), L"-dll") == 0)
            {
                oEnum.Next();
                if (! oEnum.AtEnd()) ::lstrcpyW(wszDll, oEnum.Get());
            } // if
        } // for each arg

        if (*wszDll == 0)
        {
            ::GetModuleFileName(NULL, wszDll, lengthof(wszDll));
            char16* pwsz = wszDll + ::lstrlenW(wszDll);
            for (;;)
            {
                --pwsz;
                if ('.' == *pwsz) break;
                if (wszDll == pwsz) break;
            } // for

            ::lstrcpyW(pwsz + 1, L"dll");
        } // if

        m_hDll = ::LoadLibrary(wszDll);

        if (m_hDll == NULL)
        {
            DWORD dwError = ::GetLastError();
            char16 wsz[100];
            ::wsprintf(wsz, L"LoadLibrary %s %u", wszDll, dwError);
            ::MessageBox(NULL, wsz, L"Listener", 0);
            return dwError;
        } // if

        // warning C4191: 'operator/operation': unsafe conversion from 'type 
        // of expression' to 'type required'
        #define DEFINE_DLL_ENTRY(type, name) \
          __pragma(warning(suppress: 4191)) \
          type name = reinterpret_cast<type>(::GetProcAddress(m_hDll, #name));
        DEFINE_DLL_ENTRY(GetEngineFn, GetEngine);
        if (GetEngine == NULL)
        {
            DWORD dwError = ::GetLastError();
            char16 wsz[100];
            ::wsprintf(wsz, L"GetProcAddress GetEngine %u", dwError);
            ::MessageBox(NULL, wsz, L"Listener", 0);
            return dwError;
        } // if

        sm_pIEngine = GetEngine();

        const size_t k_cbHeap = 1024 * 1024 * sizeof(void*) * 32;
        sm_pIEngine->Init(k_cbHeap);

        if (*wszImage == 0)
        {
            ::GetModuleFileName(m_hDll, wszImage, lengthof(wszImage));
            char16* pwsz = wszImage + ::lstrlenW(wszImage);
            for (;;)
            {
                --pwsz;
                if ('.' == *pwsz) break;
                if (wszImage == pwsz) break;
            } // for

            ::lstrcpyW(pwsz + 1, L"image");
        } // if

        HRESULT hr = sm_pIEngine->LoadImage(wszImage);
        if (FAILED(hr))
        {
            char16 wsz[100];
            ::wsprintf(wsz, L"LoadImage %s 0x%08X", wszImage, hr);
            ::MessageBox(NULL, wsz, L"Listener", 0);
            return static_cast<uint>(hr);
        } // if
    } // if

    m_hThread = ::CreateThread(
        NULL,               // lpThreadAttributes
        0,                  // dwStackSize
        threadProc_,        // threadProc
        this,               // lpParameter
        CREATE_SUSPENDED,   // dwCreationFlags
        &m_dwThread );      // out_dwThreadId
    if (m_hThread == NULL)
    {
        DWORD dwError = ::GetLastError();
        DEBUG_PRINTF("CreateThread: %u\n", dwError);
        return dwError;
    } // if

    return 0;
} // LispThread::Init


//////////////////////////////////////////////////////////////////////
//
// LispThread::Interrupt
//
void LispThread::Interrupt()
{
    if (m_pvLisp != NULL)
    {
        sm_pIEngine->Interrupt(m_pvLisp);
    } // if
} // LispThread::Interrupt


uint LispThread::Start(HWND hwnd)
{
    ASSERT(hwnd != NULL);

    ASSERT(m_hwnd == NULL);

    if (m_hThread == NULL) return 0;

    m_hwnd = hwnd;

    ::ResumeThread(m_hThread);

    return 0;
} // LispThread::Start


//////////////////////////////////////////////////////////////////////
//
// LispThread::threadProc
//
DWORD LispThread::threadProc()
{
    const size_t k_cbThread = 128 * 1024;

    ListenerInfo oInfo;
    oInfo.m_hwndListener = m_hwnd;

    m_pvLisp = sm_pIEngine->Bless(k_cbThread, &oInfo);

    return static_cast<DWORD>(sm_pIEngine->Start(NULL));
} // Listener::threadProc
