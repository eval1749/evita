#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - Editor - Buffer File I/O
// listener/winapp/ed_file.cpp
//
// Copyright (C) 1996-2011 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_FileIo.cpp#5 $
//
#define DEBUG_IO 0
#define DEBUG_LOAD 0
#define DEBUG_SAVE 0
#include "evita/vi_FileIo.h"

#include "evita/text/modes/mode.h"
#include "evita/ed_undo.h"

#include "evita/dom/buffer.h"
#include "evita/vi_IoManager.h"

#include "../charset/CharsetDecoder.h"
#include "../charset/CharsetDetector.h"
#include <algorithm>

using namespace Charset;

DWORD  IoRequest::sm_dwThread;
HANDLE IoRequest::sm_hIoCompletionPort;
HANDLE IoRequest::sm_hThread;

const DWORD k_cbHugeFile = 1u << 28;

/// <summary>
///   Represents file information request used by Buffer.UpdateFileStatus.
/// </summary>
class InfoRequest : public FileRequest {
  private: Buffer* m_pBuffer;

  public: InfoRequest(
      Buffer* const pBuffer,
      const char16* const pwszFileName)
        : m_pBuffer(pBuffer),
          FileRequest(pwszFileName) {
  }

  private: void finishIo(uint nError) {
    #if DEBUG_IO
      DEBUG_PRINTF("%p err=%d buf=%s\n", this, nError, m_pBuffer->GetName());
    #endif
    if (nError != 0) {
      m_pBuffer->SetObsolete(Buffer::Obsolete_Unknown);
      return;
    }

    auto const iDiff = m_pBuffer->GetLastWriteTime()->Compare(&m_ftLastWrite);

    m_pBuffer->SetObsolete(0 == iDiff
        ? Buffer::Obsolete_No
        : Buffer::Obsolete_Yes);

    m_pBuffer->SetReadOnly((m_nFileAttrs & FILE_ATTRIBUTE_READONLY) != 0);
  } // finishIo

  private: void onEvent(uint) {
    finishIo(openForLoad());
    delete this;
  } // onEvent
}; // InfoRequest

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
      ASSERT(GetNext() == nullptr);
      ASSERT(GetPrev() == nullptr);
      delete[] m_pchStart;
    }

    public: char* GetStart() const { return m_pchStart; }
    public: char* GetEnd() const { return m_pchEnd; }
    private: char* m_pchStart;
    private: char* m_pchEnd;
  };

  private: DoubleLinkedList_<Chunk> m_Chunks;
  private: CharsetDecoder* m_CharsetDecoder;
  private: CharsetDetector m_CharsetDetector;
  private: NewlineDetector m_NewlineDetector;

  private: uint m_nCodePage;
  private: uint m_nDefaultCodePage;
  private: Buffer* m_pBuffer;

  private: int m_cwchPending;
  private: char16 m_rgwchPending[1024];

  // ctor
  public: LoadRequest(
      Buffer* const pBuffer,
      const char16* const pwszFileName,
      uint const nCodePage,
      NewlineMode const eNewline)
      : FileIoRequest(pwszFileName),
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
}; // LoadRequest

class SaveRequest : public FileIoRequest {
  private: NewlineMode m_eNewline;
  private: uint m_nCodePage;
  private: Posn m_lEnd;
  private: Posn m_lPosn;
  private: Buffer* m_pBuffer;
  private: char16 m_wszTempName[MAX_PATH + 1];

  // ctor
  public: SaveRequest(
      Buffer*         pBuffer,
      const char16*   pwszFileName,
      uint            nCodePage,
      NewlineMode     eNewline,
      Posn            lStart,
      Posn            lEnd)
      : m_eNewline(eNewline),
        m_lEnd(lEnd),
        m_lPosn(lStart),
        m_nCodePage(nCodePage),
        m_pBuffer(pBuffer),
        FileIoRequest(pwszFileName) {
  }

  // [F]
  private: virtual void finishIo(uint) override;

  // [O]
  private: virtual void onEvent(uint) override;

  // [R]
  private: void requestWrite(uint);
  private: void retrieve();
}; // SaveRequest

void Buffer::FinishIo(uint const nError) {
  m_eObsolete = nError == 0 ? Obsolete_No : Obsolete_Unknown;
  m_eState = State_Ready;
  m_tickLastCheck = ::GetTickCount();
} // Buffer::FinishIo

//  Start loading from file. This consists following steps:
//      o Set not ready
//      o Discard undo log
//      o Make buffer r/w
//      o Discard contents of buffer
//      o Request "Load" to I/O thread
bool Buffer::Load(const char16* const pwszFileName) {
  if (IsNotReady()) {
      return false;
  }

  auto const pLoad = new LoadRequest(
      this,
      pwszFileName,
      m_nCodePage,
      NewlineMode_Detect);

  // We'll set read-only flag from file attributes.
  SetReadOnly(false);

  Delete(0, GetEnd());

  if (m_pUndo) {
    m_pUndo->Empty();
  } // if

  m_eObsolete = Obsolete_Checking;
  m_eState = State_Load;

  if (!pLoad->Start()) {
      FinishIo(1);
      delete pLoad;
      return false;
  } // if

  return true;
} // Buffer::Load

bool Buffer::Reload() {
  return !GetFileName().empty() && Load(GetFileName().c_str());
}

bool Buffer::Save(
    const char16* const pwszFileIn,
    uint const nCodePage,
    NewlineMode const eNewline) {
  if (IsNotReady()) {
    return false;
  }

  if (NewlineMode_Detect == eNewline) {
    // FIXME 2007-07-17 What should we do for Save with NewlineMode_Detect?
    //  1. Continuable error - User should specify newline mode.
    //  2. Get default value from Application settings.
    return false;
  }

  auto const pwszFile = *pwszFileIn ? base::string16(pwszFileIn) : filename_;
  m_nCodePage = nCodePage;
  m_eNewline = eNewline;

  auto const pSave = new SaveRequest(
      this,
      pwszFile.c_str(),
      nCodePage,
      eNewline,
      0,
      GetEnd());

  m_eObsolete = Obsolete_Checking;
  m_eState = State_Save;

  if (!pSave->Start()) {
    FinishIo(1);
    delete pSave;
    return false;
  } // if

    return true;
} // Buffer::Save

//  Markes the buffer visited to specified file.
void text::Buffer::SetFile(
    const base::string16& filename,
    const FileTime& last_write_time) {
  filename_ =  filename;
  m_ftLastWrite = last_write_time;
  m_nSaveTick = m_nCharTick;
  m_eObsolete = Obsolete_No;
} // Buffer::SetFile

/// <summary>
///   Updates buffer obsolete status from associated file.
/// </summary>
/// <param name="fForce">
///   If true, ignore cached status.
/// </param>
void Buffer::UpdateFileStatus(bool const fForce) {
  if (filename_.empty())
    return;

  if (IsNotReady()) {
    return;
  }

  if (m_eObsolete == Obsolete_Checking ||m_eObsolete == Obsolete_Ignore)
      return;

  auto const tickNow = ::GetTickCount();

  if (!fForce) {
    if (tickNow - m_tickLastCheck < k_tickFileCheck) {
      return;
    }
  }

  m_tickLastCheck = tickNow;

  auto const pCheck = new InfoRequest(this, filename_.c_str());
  m_eObsolete = Obsolete_Checking;
  if (!pCheck->Start()) {
    m_eObsolete = Obsolete_Unknown;
    delete pCheck;
  }
} // Buffer::UpdateFileStatus

uint FileRequest::openForLoad() {
  ASSERT(m_hFile == INVALID_HANDLE_VALUE);

  m_hFile = ::CreateFile(
      m_wszFileName,
      GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_DELETE,
      nullptr,
      OPEN_EXISTING,
      FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN,
      nullptr);
  if (m_hFile == INVALID_HANDLE_VALUE) {
    auto const dwError = ::GetLastError();
    DEBUG_PRINTF("CreateFile: %p %ls %u\n",
        this, m_wszFileName, dwError);
    return dwError;
  } // if

  BY_HANDLE_FILE_INFORMATION oInfo;
  if (!::GetFileInformationByHandle(m_hFile, &oInfo)) {
    auto const dwError = ::GetLastError();
    DEBUG_PRINTF("GetFileInformationByHandle: %p %ls %u\n",
        this, m_wszFileName, dwError);
    return dwError;
  }

  if (oInfo.nFileSizeHigh != 0 || oInfo.nFileSizeLow > k_cbHugeFile) {
    return ERROR_NOT_ENOUGH_MEMORY;
  } // if

  m_cbFile = static_cast<Count>(oInfo.nFileSizeLow);
  m_ftLastWrite = oInfo.ftLastWriteTime;
  m_nFileAttrs = oInfo.dwFileAttributes;

  return 0;
} // FileRequest::openForLoad

bool IoRequest::associate(HANDLE hFile) {
    auto const hPort = ::CreateIoCompletionPort(
      hFile,
      sm_hIoCompletionPort,
      reinterpret_cast<ULONG_PTR>(this),
      0);

  if (hPort != sm_hIoCompletionPort) {
    auto const dwError = ::GetLastError();
    DEBUG_PRINTF("CreateIoCompletionPort: %p %ls %u\n",
        this, m_wszFileName, dwError);
    finishIo(dwError);
    return false;
  } // if

  return true;
} // IoRequest::associate

bool IoRequest::ensureThread() {
  if (sm_hThread) {
    return true;
  }

  sm_hIoCompletionPort = ::CreateIoCompletionPort(
      INVALID_HANDLE_VALUE,
      nullptr,
      0,
      0);
  if (sm_hIoCompletionPort == nullptr) {
    #if _DEBUG
      auto const dwError = ::GetLastError();
      DEBUG_PRINTF("CreateIoCompletionPort: %u\n", dwError);
    #endif // _DEBUG
    return false;
  } // if

  sm_hThread = ::CreateThread(
      nullptr,
      0,
      threadProc,
      nullptr,
      0,
      &sm_dwThread);
  if (sm_hThread == nullptr) {
    #if _DEBUG
      auto const dwError = ::GetLastError();
      DEBUG_PRINTF("CreateThread: %u\n", dwError);
    #endif // _DEBUG
    return false;
  } // if

  return true;
} // IoRequest::ensureThread

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
  } // if

    return true;
} // IoRequest::Start

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
      } // if

    #if DEBUG_IO
      DEBUG_PRINTF("%p %u %p\n", pFile, cbReceived, pOverlapped);
    #endif
    pFile->onEvent(cbReceived);
  } // for
} // IoRequest::threadProc

void LoadRequest::DecoderOutput(
    const char16* const start,
    const char16* const end) {
  ASSERT(start != nullptr);
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
    if (m_CharsetDecoder == nullptr) {
      if (auto const nCodePage = m_CharsetDetector.Finish()) {
        m_nCodePage = static_cast<uint>(nCodePage);
        m_CharsetDecoder = CharsetDecoder::Create(
            static_cast<CodePage>(nCodePage), this);
      }
    }
  }

  while (auto const chunk = m_Chunks.GetFirst()) {
    if (m_CharsetDecoder == nullptr) {
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

  m_pBuffer->SetCodePage(
      m_CharsetDetector.IsBinary()
          ? 0
          : m_nCodePage == 0
          ? m_nDefaultCodePage
          : m_nCodePage);

  IoManager::FinishLoad(
      m_pBuffer,
      m_wszFileName,
      nError,
      eNewline,
      m_nFileAttrs,
      &m_ftLastWrite);

  delete this;
} // LoadRequest::finishIo

void LoadRequest::flushSendBuffer() {
  if (m_cwchPending > 0) {
    IoManager::InsertString(
      m_pBuffer,
      m_pBuffer->GetEnd(),
      m_rgwchPending,
      m_cwchPending);
    m_cwchPending = 0;
  }
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
      DEBUG_PRINTF("%p %ls cb=%u @%u\n",
          this, m_wszFileName, cbRead, m_oOverlapped.Offset);
  #endif // DEBUG_LOAD

  if (m_hFile == INVALID_HANDLE_VALUE) {
    auto const nError = openForLoad();
    if (nError != 0) {
      finishIo(nError);
      return;
    }

    if (associate(m_hFile)) {
      requestRead();
    } // if

    return;
  } // if

  // Do we reach EOF?
  if (cbRead == 0) {
    finishIo(0);
    return;
  } // if

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

  if (m_CharsetDecoder == nullptr) {
    if (auto const nCodePage = m_CharsetDetector.Detect(runner, end)) {
      m_nCodePage = static_cast<uint>(nCodePage);
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

  if (m_CharsetDecoder == nullptr) {
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
      DEBUG_PRINTF("ReadFile: %p %ls %u\n", this, m_wszFileName, dwError);
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
    } // switch
  } // if
} // LoadRequest::requestRead

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
        DEBUG_PRINTF("SetFilePointerEx: %p %ls %u\n",
            this, m_wszFileName, dwError);
        finishIo(dwError);
        return;
      } // if

      if (!::SetEndOfFile(m_hFile)) {
        auto const dwError = ::GetLastError();

        DEBUG_PRINTF("SetEndOfFile: %p %ls %u\n",
            this, m_wszFileName, dwError);
        finishIo(dwError);
        return;
      } // if
    }

    ::CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;

    {
      auto const fSucceeded = ::MoveFileEx(
          m_wszTempName,
          m_wszFileName,
          MOVEFILE_REPLACE_EXISTING);

      if (!fSucceeded) {
        auto const dwError = ::GetLastError();
        DEBUG_PRINTF("MoveFileEx: %p %ls %u %ls\n",
            this, m_wszFileName, dwError, m_wszTempName);
        finishIo(dwError);
        return;
      } // if
    }

    openForLoad();
  } // if

  ::DeleteFile(m_wszTempName);

  IoManager::FinishSave(
      m_pBuffer,
      m_wszFileName,
      nError,
      NewlineMode_Detect,
      m_nFileAttrs,
      &m_ftLastWrite);

  delete this;
} // SaveRequest::finishIo

void SaveRequest::onEvent(uint const cbWritten) {
  #if DEBUG_SAVE
    DEBUG_PRINTF("%p %ls cb=%u @%u\n",
        this, m_wszFileName, cbWritten, m_oOverlapped.Offset);
  #endif // DEBUG_SAVE

  if (m_hFile == INVALID_HANDLE_VALUE) {
    {
      char16 wszTempDir[MAX_PATH + 1];
      char16* pwszFile;
      auto const cwchFull = ::GetFullPathName(
          m_wszFileName,
          lengthof(wszTempDir),
          wszTempDir,
          &pwszFile);
      if (cwchFull == 0) {
        auto const dwError = ::GetLastError();
        DEBUG_PRINTF("GetFullPathName: %p %ls %u\n",
            this, m_wszFileName, dwError);
        finishIo(dwError);
        return;
      } // if

      *pwszFile = 0;
      auto const cwchTemp = ::GetTempFileName(
          wszTempDir,
          L"ed",
          0,
          m_wszTempName);
      if (cwchTemp == 0) {
        auto const dwError = ::GetLastError();
        DEBUG_PRINTF("GetTempName: %p %ls %u %ls\n",
            this, m_wszFileName, dwError, wszTempDir);
        finishIo(dwError);
        return;
      } // if
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
      DEBUG_PRINTF("CreateFile: %p %ls %u %ls\n",
          this, m_wszFileName, dwError, m_wszTempName);
      finishIo(dwError);
      return;
    } // if

    if (m_lPosn == m_lEnd) {
      finishIo(0);
    } else if (associate(m_hFile)) {
      retrieve();
    } // if

    return;
  } // if

  // Do we reach EOF?
  if (m_lPosn == m_lEnd) {
    finishIo(0);
    return;
  } // if

  // Advance to next read position.
  m_oOverlapped.Offset += cbWritten;

  retrieve();
} // SaveRequest::onEvent

void SaveRequest::requestWrite(uint const cbWrite) {
  #if DEBUG_SAVE
    DEBUG_PRINTF("%p %ls cb=%u @%u\n",
        this, m_wszFileName, cbWrite, m_oOverlapped.Offset);
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

  DEBUG_PRINTF("WriteFile: %p %ls %u\n",
    this, m_wszFileName, dwError);
  finishIo(dwError);
} // SaveRequest::requestWrite

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
        } // switch code page

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
        } // if

        DEBUG_PRINTF("WideCharToMultiByte: %p %ls %u\n",
            this, m_wszFileName, dwError);
        finishIo(dwError);
        return;
      } // if
      pch += cch;
    } // if
  } // for

  m_lPosn += static_cast<int>(pwch - pwchStart);
  requestWrite(static_cast<uint>(pch - m_rgchIoBuffer));
} // SaveRequest::retrieve
