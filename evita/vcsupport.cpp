#include "precomp.h"

typedef unsigned char uint8;

#if defined(NDEBUG)
extern "C"
{

#if 0 && defined (_M_IX86)
void* __cdecl memset(void* dst, int c, size_t count)
{
    uint8* d = reinterpret_cast<uint8*>(dst);
    uint8* e = d + count;
    while (d < e) *d++ = static_cast<uint8>(c);
    return dst;
} // memset
#endif // defined (_M_IX86)

#if defined (_M_IX86)
void* myZeroMemory(void* dst, size_t count)
{
    uint8* d = reinterpret_cast<uint8*>(dst);
    uint8* e = d + count;
    while (d < e) *d++ = 0;
    return dst;
} // myZeroMemory
#endif // defined (_M_IX86)
} // extern "C"
#endif // NDEBUG

#if !defined(_DEBUG)

void* __cdecl operator new(size_t cb)
    { return ::HeapAlloc(::GetProcessHeap(), 0, cb); }

void __cdecl operator delete(void* pv)
    { ::HeapFree(::GetProcessHeap(), 0, pv); }

extern "C"
{

typedef unsigned char uint8;

int __cdecl _purecall(void) { return 0; }

#undef myCopyMemory
void* myCopyMemory(void* dst, const void* src, size_t count)
{
    const uint8* s = reinterpret_cast<const uint8*>(src);
    const uint8* e = s + count;
    uint8* d = reinterpret_cast<uint8*>(dst);
    while (s < e) *d++ = *s++;
    return dst;
} // memcpy

} // extern "C"

#endif // !defined(_DEBUG)


#if defined(NDEBUG)
extern "C"
{

void* myMoveMemory(void* dst, const void* src, size_t count)
{
    const uint8* s = reinterpret_cast<const uint8*>(src);
    const uint8* e = s + count;
    uint8* d = reinterpret_cast<uint8*>(dst);
    if (s < d && d < e)
    {
        d += count;
        while (e > s) *--d = *--e;
    }
    else
    {
        while (s < e) *d++ = *s++;
    }
    return dst;
} // myMoveMemory

#if _M_X64
void* myZeroMemory(void* dst, size_t count)
{
    uint8* d = reinterpret_cast<uint8*>(dst);
    uint8* e = d + count;
    while (d < e) *d++ = 0;
    return dst;
} // myZeroMemory
#endif //  _M_X64

#if defined (_M_IX86)
#pragma function(memcpy)

void* __cdecl memcpy(void* dst, const void* src, size_t count)
{
    DCHECK(NULL != dst);
    DCHECK(NULL != src);
    const uint8* s = reinterpret_cast<const uint8*>(src);
    const uint8* e = s + count;
    uint8* d = reinterpret_cast<uint8*>(dst);
    while (s < e) *d++ = *s++;
    return dst;
} // memcpy
#endif // defined (_M_IX86)

// FIXME 2007-07-28 yosi@msn.com We should remove dependency of LIBCMT.
// It seems GDI+ requires LIBCMT for floating point operation.(?)
#if 0
// See crt/src/crtexe.c
void __cdecl WinMainCRTStartup()
{
    int iRet = WinMain(::GetModuleHandle(NULL), NULL, NULL, 0);
    ::ExitProcess(iRet);
} // WinMainCRTStartup
#endif

} // extern "C"
#endif // NDEBUG
