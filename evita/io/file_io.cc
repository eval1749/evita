// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define DEBUG_IO 0
#define DEBUG_LOAD 0
#define DEBUG_SAVE 0
#include "evita/io/file_io.h"

#include <algorithm>

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/time/time.h"
#include "evita/dom/buffer.h"
#include "evita/dom/public/api_callback.h"
#include "evita/dom/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/io/io_manager.h"
#include "evita/text/undo_stack.h"
#include "../../charset/CharsetDecoder.h"
#include "../../charset/CharsetDetector.h"

using namespace Charset;

DWORD  IoRequest::sm_dwThread;
HANDLE IoRequest::sm_hIoCompletionPort;
HANDLE IoRequest::sm_hThread;

const DWORD k_cbHugeFile = 1u << 28;

#define DVLOG_WIN32_ERROR(level, name) \
  DVLOG(level) << name ": " << this << " " << file_name_ << " err=" << dwError

//////////////////////////////////////////////////////////////////////
//
// IoRequest
//
IoRequest::IoRequest(const base::string16& file_name)
    : m_hFile(INVALID_HANDLE_VALUE), file_name_(file_name) {
}

IoRequest::~IoRequest() {
  if (INVALID_HANDLE_VALUE != m_hFile)
    ::CloseHandle(m_hFile);
}

//////////////////////////////////////////////////////////////////////
//
// FileRequest
//
FileRequest::FileRequest(const base::string16&  file_name)
    : IoRequest(file_name), m_cbFile(0), m_nFileAttrs(0) {
}

FileRequest::~FileRequest() {
}

//////////////////////////////////////////////////////////////////////
//
// FileRequest
//
FileIoRequest::FileIoRequest(const base::string16& file_name)
    : FileRequest(file_name) {
  ::ZeroMemory(&m_oOverlapped, sizeof(m_oOverlapped));
}

FileIoRequest::~FileIoRequest() {
}

class NewlineDetector {
  public: enum Type {
    Ignore,
    Newline,
    NotNewline,
  };

  private: int cr_count_;
  private: int crlf_count_;
  private: char16 last_char_;
  private: int lf_count_;
  private: NewlineMode mode_;

  public: NewlineDetector(NewlineMode mode)
      : cr_count_(0),
        crlf_count_(0),
        last_char_(0),
        lf_count_(0),
        mode_(mode) {
  }

  public: Type Check(char16 wch) {
    auto const last = last_char_;
    last_char_ = wch;
    switch (mode_) {
      case NewlineMode_Cr:
        return wch == 0x0D ? Newline : NotNewline;

      case NewlineMode_CrLf:
        return wch == 0x0A && last == 0x0D ? Newline : NotNewline;

      case NewlineMode_Detect:
        switch (wch) {
          case 0x0D:
            cr_count_++;
            return Newline;

          case 0x0A:
            if (last == 0x0D) {
              crlf_count_++;
              return Ignore;
            }

            lf_count_++;
            return Newline;
        }
        return NotNewline;

      case NewlineMode_Lf:
        return wch == 0x0A ? Newline : NotNewline;

      default:
        CAN_NOT_HAPPEN();
    }
  }

  public: NewlineMode Detect() const {
    switch (mode_) {
      case NewlineMode_Cr:
      case NewlineMode_CrLf:
      case NewlineMode_Lf:
        return mode_;

      case NewlineMode_Detect:
        if (crlf_count_ > 0) {
          return NewlineMode_CrLf;
        }

        if (lf_count_ > 0) {
          return NewlineMode_Lf;
        }

        if (cr_count_ > 0) {
          return NewlineMode_Cr;
        }

        return NewlineMode_Detect;

      default:
        CAN_NOT_HAPPEN();
    }
  }
};

/// <summary>
///  Represents file loading activity.
/// </summary>
class LoadRequest : public FileIoRequest, public CharsetDecoder::Callback {
  private: class Chunk : public DoubleLinkedNode_<Chunk> {
    public: Chunk(
        const char* pchStart,
        const char* pchEnd)
        : m_pchStart(new char[static_cast<uint>(pchEnd - pchStart)]),
          m_pchEnd(m_pchStart + (pchEnd - pchStart)) {
      myCopyMemory(m_pchStart, pchStart, 
                   static_cast<size_t>(pchEnd - pchStart));
    }

    public: ~Chunk() {
      ASSERT(!GetNext());
      ASSERT(!GetPrev());
      delete[] m_pchStart;
    }

    public: char* GetStart() const { return m_pchStart; }
    public: char* GetEnd() const { return m_pchEnd; }
    private: char* m_pchStart;
    private: char* m_pchEnd;
  };

  private: dom::ViewDelegate::LoadFileCallback callback_;
  private: DoubleLinkedList_<Chunk> m_Chunks;
  private: CharsetDecoder* m_CharsetDecoder;
  private: CharsetDetector m_CharsetDetector;
  private: NewlineDetector m_NewlineDetector;

  private: int m_nCodePage;
  private: int m_nDefaultCodePage;
  private: Buffer* m_pBuffer;

  private: int m_cwchPending;
  private: char16 m_rgwchPending[1024];

  // ctor
  public: LoadRequest(
      const dom::ViewDelegate::LoadFileCallback& callback,
      Buffer* const pBuffer,
      const base::string16& file_name,
      int const nCodePage,
      NewlineMode const eNewline)
      : FileIoRequest(file_name),
        callback_(callback),
        m_CharsetDecoder(nullptr),
        m_NewlineDetector(eNewline),
        m_cwchPending(0),
        m_nCodePage(0),
        m_nDefaultCodePage(nCodePage),
        m_pBuffer(pBuffer) {
  }

  public: ~LoadRequest() {
    while (auto chunk = m_Chunks.GetFirst()) {
      m_Chunks.Delete(chunk);
      delete chunk;
    }

    delete m_CharsetDecoder;
  }

  // [D]
  private: virtual void DecoderOutput(const char16*, const char16*) override;

  // [F]
  private: virtual void finishIo(uint) override;
  private: void flushSendBuffer();

  // [G]
  private: int GetCodePage() const;

  // [I]
  private: void insertBytes(const char*, const char*);
  private: void insertChar(char16);

  // [O]
  private: virtual void onEvent(uint) override;

  // [P]
  private: void processChunk(const char*, const char*);
  private: void processRead(const char*, const char*);

  // [R]
  private: void requestRead();

  // [S]
  private: void sendChar(char16);

  DISALLOW_COPY_AND_ASSIGN(LoadRequest);
};

class SaveRequest : public FileIoRequest {
  private: dom::ViewDelegate::SaveFileCallback callback_;
  private: NewlineMode m_eNewline;
  private: uint m_nCodePage;
  private: Posn m_lEnd;
  private: Posn m_lPosn;
  private: Buffer* m_pBuffer;
  private: char16 m_wszTempName[MAX_PATH + 1];

  // ctor
  public: SaveRequest(
      const dom::ViewDelegate::SaveFileCallback callback,
      Buffer*         pBuffer,
      const base::string16& filename,
      uint            nCodePage,
      NewlineMode     eNewline,
      Posn            lStart,
      Posn            lEnd)
      : callback_(callback),
        m_eNewline(eNewline),
        m_lEnd(lEnd),
        m_lPosn(lStart),
        m_nCodePage(nCodePage),
        m_pBuffer(pBuffer),
        FileIoRequest(filename) {
  }

  // [F]
  private: virtual void finishIo(uint) override;

  // [O]
  private: virtual void onEvent(uint) override;

  // [R]
  private: void requestWrite(uint);
  private: void retrieve();

  DISALLOW_COPY_AND_ASSIGN(SaveRequest);
};

//  Start loading from file. This consists following steps:
//      o Set not ready
//      o Discard undo log
//      o Make buffer r/w
//      o Discard contents of buffer
//      o Request "Load" to I/O thread
bool Buffer::Load(const base::string16& file_name,
                  const dom::ViewDelegate::LoadFileCallback& callback) {
  if (IsNotReady())
      return false;

  auto const pLoad = new LoadRequest(
      callback,
      this,
      file_name,
      static_cast<int>(m_nCodePage),
      NewlineMode_Detect);

  // We'll set read-only flag from file attributes.
  SetReadOnly(false);
  Delete(0, GetEnd());
  m_eState = State_Load;

  if (!pLoad->Start()) {
      FinishIo(1);
      delete pLoad;
      return false;
  }

  return true;
}

namespace {
void DummyCallback(const domapi::LoadFileCallbackData&) {
}
}

bool Buffer::Load(const base::string16& file_name) {
  return Load(file_name, base::Bind(DummyCallback));
}

bool Buffer::Save(const base::string16& filename, int const nCodePage,
    NewlineMode const eNewline,
    const dom::ViewDelegate::SaveFileCallback& callback) {
  if (IsNotReady()) {
    return false;
  }

  if (NewlineMode_Detect == eNewline) {
    // FIXME 2007-07-17 What should we do for Save with NewlineMode_Detect?
    //  1. Continuable error - User should specify newline mode.
    //  2. Get default value from Application settings.
    return false;
  }

  m_nCodePage = static_cast<uint32_t>(nCodePage);
  m_eNewline = eNewline;

  auto const pSave = new SaveRequest(
      callback,
      this,
      filename,
      static_cast<uint32_t>(nCodePage),
      eNewline,
      0,
      GetEnd());

  m_eState = State_Save;

  if (!pSave->Start()) {
    FinishIo(1);
    delete pSave;
    return false;
  }

    return true;
}

uint FileRequest::openForLoad() {
  ASSERT(m_hFile == INVALID_HANDLE_VALUE);

  m_hFile = ::CreateFile(
      file_name_.c_str(),
      GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_DELETE,
      nullptr,
      OPEN_EXISTING,
      FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN,
      nullptr);
  if (m_hFile == INVALID_HANDLE_VALUE) {
    auto const dwError = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "CreateFile");
    return dwError;
  }

  BY_HANDLE_FILE_INFORMATION oInfo;
  if (!::GetFileInformationByHandle(m_hFile, &oInfo)) {
    auto const dwError = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "GetFileInformationByHandle");
    return dwError;
  }

  if (oInfo.nFileSizeHigh != 0 || oInfo.nFileSizeLow > k_cbHugeFile) {
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  m_cbFile = static_cast<Count>(oInfo.nFileSizeLow);
  last_write_time_ = base::Time::FromFileTime(oInfo.ftLastWriteTime);
  m_nFileAttrs = oInfo.dwFileAttributes;

  return 0;
}

bool IoRequest::associate(HANDLE hFile) {
    auto const hPort = ::CreateIoCompletionPort(
      hFile,
      sm_hIoCompletionPort,
      reinterpret_cast<ULONG_PTR>(this),
      0);

  if (hPort != sm_hIoCompletionPort) {
    auto const dwError = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "GetFileInformationByHandle");
    finishIo(dwError);
    return false;
  }

  return true;
}

bool IoRequest::ensureThread() {
  if (sm_hThread) {
    return true;
  }

  sm_hIoCompletionPort = ::CreateIoCompletionPort(
      INVALID_HANDLE_VALUE,
      nullptr,
      0,
      0);
  if (!sm_hIoCompletionPort) {
    #if _DEBUG
      auto const dwError = ::GetLastError();
      DEBUG_PRINTF("CreateIoCompletionPort: %u\n", dwError);
    #endif // _DEBUG
    return false;
  }

  sm_hThread = ::CreateThread(
      nullptr,
      0,
      threadProc,
      nullptr,
      0,
      &sm_dwThread);
  if (!sm_hThread) {
    #if _DEBUG
      auto const dwError = ::GetLastError();
      DEBUG_PRINTF("CreateThread: %u\n", dwError);
    #endif // _DEBUG
    return false;
  }

  return true;
}

bool IoRequest::Start() {
  if (!ensureThread()) {
    return false;
  }

  auto const fSucceeded = ::PostQueuedCompletionStatus(
      sm_hIoCompletionPort,
      0,
      reinterpret_cast<ULONG_PTR>(this),
      nullptr);
  if (!fSucceeded) {
      #if _DEBUG
        auto const dwError = ::GetLastError();
        DEBUG_PRINTF("PostQueuedCompletionStatus: %u\n", dwError);
      #endif
      return false;
  }

    return true;
}

DWORD WINAPI IoRequest::threadProc(void*) {
  for (;;) {
      DWORD cbReceived;
      ULONG_PTR ulpKey;
      OVERLAPPED* pOverlapped;
      auto const fSucceeded = ::GetQueuedCompletionStatus(
          sm_hIoCompletionPort,
          &cbReceived,
          &ulpKey,
          &pOverlapped,
          INFINITE);

      auto const pFile = reinterpret_cast<IoRequest*>(ulpKey);

      if (!fSucceeded) {
        auto const dwError = ::GetLastError();

        #if DEBUG_IO
          DEBUG_PRINTF("GetQueuedCompletionStatus:"
              " err=%u key=%p ov=%p\n",
              dwError, ulpKey, pOverlapped);
        #endif // _DEBUG

        if (pOverlapped) {
          pFile->finishIo(dwError);
        }
        continue;
      }

    #if DEBUG_IO
      DEBUG_PRINTF("%p %u %p\n", pFile, cbReceived, pOverlapped);
    #endif
    pFile->onEvent(cbReceived);
  }
}

void LoadRequest::DecoderOutput(
    const char16* const start,
    const char16* const end) {
  ASSERT(start);
  for (auto p = start; p < end; p++) {
    insertChar(*p);
  }
}

void LoadRequest::finishIo(uint const nError) {
  #if DEBUG_LOAD
      DEBUG_PRINTF("%p err=%u\n", this, nError);
  #endif // DEBUG_LOAD

  auto eNewline = NewlineMode_Lf;
  if (! m_CharsetDetector.IsBinary()) {
    eNewline = m_NewlineDetector.Detect();
    if (!m_CharsetDecoder) {
      if (auto const nCodePage = m_CharsetDetector.Finish()) {
        m_nCodePage = nCodePage;
        m_CharsetDecoder = CharsetDecoder::Create(
            static_cast<CodePage>(nCodePage), this);
      }
    }
  }

  while (auto const chunk = m_Chunks.GetFirst()) {
    if (!m_CharsetDecoder) {
      insertBytes(chunk->GetStart(), chunk->GetEnd());
    } else {
      m_CharsetDecoder->Feed(chunk->GetStart(), chunk->GetEnd());
    }
    m_Chunks.Delete(chunk);
    delete chunk;
  }

  if (m_CharsetDecoder) {
    m_CharsetDecoder->Finish();
    delete m_CharsetDecoder;
    m_CharsetDecoder = nullptr;
  }

  flushSendBuffer();

  domapi::LoadFileCallbackData data;
  data.code_page = GetCodePage();
  data.error_code = nError == ERROR_HANDLE_EOF ? 0 : static_cast<int>(nError);
  data.last_write_time = last_write_time_;
  data.newline_mode = eNewline;
  data.readonly = m_nFileAttrs & FILE_ATTRIBUTE_READONLY;

  Application::instance()->view_event_handler()->RunCallback(
      base::Bind(callback_, data));

  delete this;
}

void LoadRequest::flushSendBuffer() {
  if (!m_cwchPending)
    return;
  Application::instance()->view_event_handler()->AppendTextToBuffer(
      m_pBuffer,
      base::string16(m_rgwchPending, static_cast<size_t>(m_cwchPending)));
  m_cwchPending = 0;
}

int LoadRequest::GetCodePage() const {
  if (m_CharsetDetector.IsBinary())
    return 0;
  if (!m_nCodePage)
    return m_nDefaultCodePage;
  return m_nCodePage;
}

void LoadRequest::insertBytes(
    const char* const start,
    const char* const end) {
  for (auto p = start; p < end; p++) {
    insertChar(static_cast<uint8>(*p));
  }
}

void LoadRequest::insertChar(char16 wch) {
  switch (m_NewlineDetector.Check(wch)) {
    case NewlineDetector::Ignore:
      break;

    case NewlineDetector::Newline:
      sendChar(0x0A);
      break;

    case NewlineDetector::NotNewline:
      sendChar(wch);
      break;

    default:
      CAN_NOT_HAPPEN();
  }
}

void LoadRequest::onEvent(uint const cbRead) {
  #if DEBUG_LOAD
    DVLOG_WIN32_ERROR(0, "LoadRequest::onEvent") << " cb=" << cbRead <<
        " ofs=" << m_oOverlapped.Offset;
  #endif

  if (m_hFile == INVALID_HANDLE_VALUE) {
    auto const nError = openForLoad();
    if (nError != 0) {
      finishIo(nError);
      return;
    }

    if (associate(m_hFile)) {
      requestRead();
    }

    return;
  }

  // Do we reach EOF?
  if (cbRead == 0) {
    finishIo(0);
    return;
  }

  // FIXME(yosi) 2011-12-29: Support UTF BOM and UTF-16

  // Advance to next read position.
  m_oOverlapped.Offset += cbRead;

  processRead(m_rgchIoBuffer, m_rgchIoBuffer + cbRead);
  requestRead();
}

void LoadRequest::processRead(const char* start, const char* end) {
  ASSERT(start < end);
  if (m_CharsetDetector.IsBinary()) {
    insertBytes(start, end);
    return;
  }

  auto runner = start;

  if (m_Chunks.IsEmpty()) {
    // Insert ASCII characters into buffer.
    while (runner < end) {
      auto const ch = static_cast<uint8>(*runner & 0xFF);
      if (ch >= 0x80 || ch == 0x1B || ch == 0) {
        break;
      }
      insertChar(ch);
      runner++;
    }

    if (runner == end) {
      return;
    }
  }

  ASSERT(runner < end);

  if (!m_CharsetDecoder) {
    if (auto const nCodePage = m_CharsetDetector.Detect(runner, end)) {
      m_nCodePage = nCodePage;
      m_CharsetDecoder = CharsetDecoder::Create(
          static_cast<CodePage>(nCodePage), this);

    } else if (m_CharsetDetector.IsBinary()) {
      while (auto const chunk = m_Chunks.GetFirst()) {
        insertBytes(chunk->GetStart(), chunk->GetEnd());
        m_Chunks.Delete(chunk);
        delete chunk;
      }
      insertBytes(runner, end);
      return;
    }
  }

  if (!m_CharsetDecoder) {
    if (m_Chunks.IsEmpty()) {
      flushSendBuffer();
    }
    m_Chunks.Append(new Chunk(runner, end));

  } else {
    while (auto const chunk = m_Chunks.GetFirst()) {
      m_CharsetDecoder->Feed(chunk->GetStart(), chunk->GetEnd());
      m_Chunks.Delete(chunk);
      delete chunk;
    }

    m_CharsetDecoder->Feed(runner, end);
  }
}

void LoadRequest::requestRead() {
  auto const fSucceeded = ::ReadFile(
    m_hFile,
    m_rgchIoBuffer,
    k_cbIoBufferSize,
    nullptr,
    &m_oOverlapped);

  if (!fSucceeded) {
    auto const dwError = ::GetLastError();

    #if DEBUG_LOAD
      DVLOG_WIN32_ERROR(0, "ReadFile");
    #endif

    switch (dwError) {
      case ERROR_HANDLE_EOF:
        finishIo(dwError);
        break;

      case ERROR_IO_PENDING:
        break;

      default:
        finishIo(dwError);
        break;
    }
  }
}

void LoadRequest::sendChar(char16 wch) {
  if (m_cwchPending == lengthof(m_rgwchPending)) {
    flushSendBuffer();
    ASSERT(m_cwchPending == 0);
  }
  m_rgwchPending[m_cwchPending] = wch;
  m_cwchPending++;
}

void SaveRequest::finishIo(uint const nError) {
  #if DEBUG_SAVE
    DEBUG_PRINTF("%p err=%u\n", this, nError);
  #endif // DEBUG_SAVE

  if (m_hFile != INVALID_HANDLE_VALUE && nError == 0) {
    {
      LARGE_INTEGER li;
      li.HighPart = 0;
      li.LowPart = static_cast<DWORD>(m_cbFile);
      if (!::SetFilePointerEx(m_hFile, li, nullptr, FILE_BEGIN)) {
        auto const dwError = ::GetLastError();
        DVLOG_WIN32_ERROR(0, "SetFilePointerEx");
        finishIo(dwError);
        return;
      }

      if (!::SetEndOfFile(m_hFile)) {
        auto const dwError = ::GetLastError();
        DVLOG_WIN32_ERROR(0, "SetEndOfFile");
        finishIo(dwError);
        return;
      }
    }

    ::CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;

    {
      auto const fSucceeded = ::MoveFileEx(
          m_wszTempName,
          file_name_.c_str(),
          MOVEFILE_REPLACE_EXISTING);

      if (!fSucceeded) {
        auto const dwError = ::GetLastError();
        DVLOG_WIN32_ERROR(0, "MoveFileEx") << " tmp=" << m_wszTempName;
        finishIo(dwError);
        return;
      }
    }

    openForLoad();
  }

  ::DeleteFile(m_wszTempName);

  domapi::SaveFileCallbackData data;
  data.error_code = static_cast<int>(nError);
  data.last_write_time = nError ? base::Time() : last_write_time_;

  Application::instance()->view_event_handler()->RunCallback(
      base::Bind(callback_, data));

  delete this;
}

void SaveRequest::onEvent(uint const cbWritten) {
  #if DEBUG_SAVE
    DVLOG_WIN32_ERROR(0, "SaveRequest::onEvent") << " cb=" << cbWritten <<
        " ofs=" << m_oOverlapped.Offset;
  #endif // DEBUG_SAVE

  if (m_hFile == INVALID_HANDLE_VALUE) {
    {
      char16 wszTempDir[MAX_PATH + 1];
      char16* pwszFile;
      auto const cwchFull = ::GetFullPathName(
          file_name_.c_str(),
          lengthof(wszTempDir),
          wszTempDir,
          &pwszFile);
      if (cwchFull == 0) {
        auto const dwError = ::GetLastError();
        DVLOG_WIN32_ERROR(0, "GetFullPathName");
        finishIo(dwError);
        return;
      }

      *pwszFile = 0;
      auto const cwchTemp = ::GetTempFileName(
          wszTempDir,
          L"ed",
          0,
          m_wszTempName);
      if (cwchTemp == 0) {
        auto const dwError = ::GetLastError();
        DVLOG_WIN32_ERROR(0, "GetTempName") << " dir=" << wszTempDir;
        finishIo(dwError);
        return;
      }
    }

    m_hFile = ::CreateFile(
        m_wszTempName,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_DELETE,
        nullptr,
        TRUNCATE_EXISTING,
        FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN,
        nullptr);
    if (m_hFile == INVALID_HANDLE_VALUE) {
      auto const dwError = ::GetLastError();
      DVLOG_WIN32_ERROR(0, "CreateFile");
      finishIo(dwError);
      return;
    }

    if (m_lPosn == m_lEnd) {
      finishIo(0);
    } else if (associate(m_hFile)) {
      retrieve();
    }

    return;
  }

  // Do we reach EOF?
  if (m_lPosn == m_lEnd) {
    finishIo(0);
    return;
  }

  // Advance to next read position.
  m_oOverlapped.Offset += cbWritten;

  retrieve();
}

void SaveRequest::requestWrite(uint const cbWrite) {
  #if DEBUG_SAVE
    DVLOG_WIN32_ERROR(0, "SaveRequest::requestWrite") << " cb=" cbWrite <<
        " ofs=" << m_oOverlapped.Offset;
  #endif // DEBUG_SAVE

  m_cbFile += cbWrite;

  auto const fSucceeded = ::WriteFile(
      m_hFile,
      m_rgchIoBuffer,
      cbWrite,
      nullptr,
      &m_oOverlapped);
  if (fSucceeded)
    return;

  auto const dwError = ::GetLastError();
  if (dwError == ERROR_IO_PENDING)
    return;

  DVLOG_WIN32_ERROR(0, "WriteFile");
  finishIo(dwError);
}

void SaveRequest::retrieve() {
  char16 rgwch[k_cbIoBufferSize];

  auto const cwch = m_pBuffer->GetText(
      rgwch,
      m_lPosn,
      std::min(m_lEnd, static_cast<Posn>(m_lPosn + lengthof(rgwch))));

  #if _DEBUG
    for (auto i = 0; i < cwch; i++) {
      ASSERT(m_pBuffer->GetCharAt(m_lPosn + i) == rgwch[i]);
    }
  #endif

  auto const pwchStart = rgwch;
  auto const pwchEnd = pwchStart + cwch;

  auto const pchEnd = m_rgchIoBuffer + lengthof(m_rgchIoBuffer);

  auto pch = m_rgchIoBuffer;
  const char16* pwch;
  for (pwch = pwchStart; pwch < pwchEnd; pwch++) {
    // We need to have at least two byte in mbcs buffer for
    // CRLF.
    if (pch + 2 > pchEnd) {
      break;
    }

    if (*pwch == 0x0A) {
      switch (m_eNewline) {
        case NewlineMode_Cr:
          *pch++ = 0x0D;
          break;

       case NewlineMode_Lf:
         *pch++ = 0x0A;
         break;

       case NewlineMode_CrLf:
         *pch++ = 0x0D;
         *pch++ = 0x0A;
         break;

       default:
         CAN_NOT_HAPPEN();
      }

    } else if (*pwch < 0x80) {
      *pch++ = static_cast<char>(*pwch);

    } else {
      BOOL fFailed;
      BOOL* pfFailed;
      switch (m_nCodePage) {
        case CP_UTF7:
        case CP_UTF8:
          // WideCharToMultiByte requires pfFailed must be nullptr.
          // I guess this conversion must not be failed.
          pfFailed = nullptr;
          break;

        default:
          pfFailed = &fFailed;
          break;
        }

      // FIXME 2007-07-23 yosi@msn.com We should call WC2MB for
      // string instead of char.
      auto const cch = ::WideCharToMultiByte(
          m_nCodePage,
          0,
          pwch,
          1,
          pch,
          static_cast<int>(pchEnd - pch),
          nullptr,
          pfFailed);
      if (cch == 0) {
        auto const dwError = ::GetLastError();
        if (dwError == ERROR_INSUFFICIENT_BUFFER) {
          break;
        }

        DVLOG_WIN32_ERROR(0, "WideCharToMultiByte");
        finishIo(dwError);
        return;
      }
      pch += cch;
    }
  }

  m_lPosn += static_cast<int>(pwch - pwchStart);
  requestWrite(static_cast<uint>(pch - m_rgchIoBuffer));
}
