// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_file_io_h)
#define INCLUDE_evita_io_file_io_h

#include "base/strings/string16.h"

//////////////////////////////////////////////////////////////////////
//
// IoRequest
//
class IoRequest
{
    protected: HANDLE   m_hFile;
    protected: base::string16 file_name_;

    private: static DWORD   sm_dwThread;
    private: static HANDLE  sm_hIoCompletionPort;
    private: static HANDLE  sm_hThread;

    // ctor
    protected: IoRequest(const base::string16& file_name)
        : m_hFile(INVALID_HANDLE_VALUE), file_name_(file_name) {
    }

    // ctor
    public: virtual ~IoRequest()
    {
        if (INVALID_HANDLE_VALUE != m_hFile)
        {
            ::CloseHandle(m_hFile);
        } // if
    } // ~IoRequest

    // [A]
    protected: bool associate(HANDLE);

    // [E]
    protected: static bool ensureThread();

    // [F]
    protected: virtual void finishIo(uint) = 0;

    // [O]
    protected: virtual void onEvent(uint) = 0;

    // [S]
    public: bool Start();

    // [T]
    protected: static DWORD WINAPI threadProc(void*);
}; // IoRequest


//////////////////////////////////////////////////////////////////////
//
// FileRequest
//
class FileRequest : public IoRequest
{
    protected: enum
    {
        k_cbIoBufferSize = 4096,
    }; // enum

    protected: Count    m_cbFile;
    protected: uint32   m_nFileAttrs;
    protected: FILETIME m_ftLastWrite;

    // FileRequest ctor
    protected: FileRequest(const base::string16&  file_name)
        : IoRequest(file_name), m_cbFile(0), m_nFileAttrs(0) {
    }

    protected: uint openForLoad();
}; // FileIoRequest


//////////////////////////////////////////////////////////////////////
//
// FileIoRequest
//
class FileIoRequest : public FileRequest
{
    protected: enum
    {
        k_cbIoBufferSize = 4096,
    }; // enum

    protected: OVERLAPPED   m_oOverlapped;
    protected: char         m_rgchIoBuffer[k_cbIoBufferSize];

    // FileIoRequest ctor
    protected: FileIoRequest(const base::string16& file_name)
        : FileRequest(file_name) {
        myZeroMemory(&m_oOverlapped, sizeof(m_oOverlapped));
    } // FileIoRequest
}; // FileIoRequest

#endif //!defined(INCLUDE_evita_io_file_io_h)
