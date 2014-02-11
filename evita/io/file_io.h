// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_file_io_h)
#define INCLUDE_evita_io_file_io_h

#include "base/basictypes.h"
#include "base/strings/string16.h"

//////////////////////////////////////////////////////////////////////
//
// IoRequest
//
class IoRequest {
  protected: HANDLE m_hFile;
  protected: base::string16 file_name_;

  private: static DWORD sm_dwThread;
  private: static HANDLE sm_hIoCompletionPort;
  private: static HANDLE sm_hThread;

  protected: IoRequest(const base::string16& file_name);
  public: virtual ~IoRequest();

  protected: bool associate(HANDLE);
  protected: static bool ensureThread();
  protected: virtual void finishIo(uint) = 0;
  protected: virtual void onEvent(uint) = 0;
  public: bool Start();
  protected: static DWORD WINAPI threadProc(void*);

  DISALLOW_COPY_AND_ASSIGN(IoRequest);
};

//////////////////////////////////////////////////////////////////////
//
// FileRequest
//
class FileRequest : public IoRequest {
  protected: Count m_cbFile;
  protected: uint32 m_nFileAttrs;
  protected: FILETIME m_ftLastWrite;

  protected: FileRequest(const base::string16& file_name);
  public: virtual ~FileRequest();

  protected: uint openForLoad();

  DISALLOW_COPY_AND_ASSIGN(FileRequest);
};

//////////////////////////////////////////////////////////////////////
//
// FileIoRequest
//
class FileIoRequest : public FileRequest {
  protected: enum {
    k_cbIoBufferSize = 4096,
  };

  protected: OVERLAPPED m_oOverlapped;
  protected: char m_rgchIoBuffer[k_cbIoBufferSize];

  protected: FileIoRequest(const base::string16& file_name);
  public: virtual ~FileIoRequest();

  DISALLOW_COPY_AND_ASSIGN(FileIoRequest);
};

#endif //!defined(INCLUDE_evita_io_file_io_h)
