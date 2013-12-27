#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - Editor - Application Main
// listener/winapp/ap_main.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ap_main.cpp#3 $
//
// Example options:
//  -dll vanilla.dll -image evcl3.image -multiple
//
#define DEBUG_BUSY 0
#define DEBUG_IDLE 0

#include "base/at_exit.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/location.h"
#include "base/logging.h"
#pragma warning(push)
#pragma warning(disable: 4100)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/run_loop.h"
#include "common/win/native_window.h"

#if USE_LISTENER
    #include "./ap_listener_buffer.h"
    #define SINGLE_INSTANCE_NAME    L"3C9C7EC2-0DE7-461e-8F09-14F3B830413E" 
#else // USE_LISTENER
    #include "./vi_buffer.h"
    #define SINGLE_INSTANCE_NAME    L"D47A7677-9F8E-467c-BABE-8ABDE8D58476" 
#endif // USE_LISTENER

#include "./ctrl_TabBand.h"

#include "evita/editor/application.h"
#include "./vi_IoManager.h"
#include "./vi_Style.h"
#include "./vi_TextEditWindow.h"

#if _WIN64
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

const char16* k_pwszTitle = L"Evita Common Lisp Listner";

namespace {

//////////////////////////////////////////////////////////////////////
//
// EnumArg
//  Enumerates command line arguments.
//
// FIXME 2007-08-07 yosi@msn.com We should share EnumArg with console
// and listener.
//
class EnumArg {
  private: enum { MAX_WORD_LEN = MAX_PATH };
  private: const char16* runner_;
  private: char16 m_wsz[MAX_WORD_LEN];

    public: EnumArg(const char16* pwsz)
      : runner_(pwsz) {
    // skip command name
    next();
    if (!AtEnd())
      next();
  }

  public: bool AtEnd() const {
    return !*runner_ && !*m_wsz;
  }

  public: LPCWSTR Get() const { ASSERT(!AtEnd()); return m_wsz; }
  public: void Next() { ASSERT(!AtEnd()); next(); }
  static bool isspace(char16 wch) { return ' ' == wch || '\t' == wch; }

    void next() {
      while (isspace(*runner_)) {
        ++runner_;
      }
      auto pwsz = m_wsz;
      if (*runner_ != 0x22) {
          while (*runner_) {
            if (isspace(*runner_))
              break;
            *pwsz++ = *runner_;
            ++runner_;
          }
      } else {
          ++runner_;
          while (*runner_) {
            if (*runner_ == 0x22) {
              ++runner_;
              break;
            }
            *pwsz++ = *++runner_;
          }
      }
      *pwsz = 0;
  }
};

} // namespace

extern HINSTANCE   g_hInstance;
extern HINSTANCE   g_hResource;
extern HWND        g_hwndActiveDialog;
extern uint        g_TabBand__TabDragMsg;

extern StyleValues g_DefaultStyle;

static void NoReturn fatalExit(const char16*);


static int callRunningApp(EnumArg* pEnumArg) {
  Handle shShared = ::OpenFileMapping(
      FILE_MAP_READ | FILE_MAP_WRITE,
      FALSE,
      k_wszFileMapping);
  if (!shShared)
      fatalExit(L"OpenFileMapping");

  auto const * p = reinterpret_cast<SharedArea*>(
      ::MapViewOfFile(shShared,
                      FILE_MAP_READ | FILE_MAP_WRITE,
                      0,      // dwFileOffsetHigh
                      0,      // dwFileOffsetLow
                      k_cbFileMapping));
  if (!p)
    fatalExit(L"MapViewOfFile");

  while (!pEnumArg->AtEnd()) {
    auto const pwszArg = pEnumArg->Get();
    char16 wsz[MAX_PATH];
    char16* pwszFile;
    auto const cwch = ::GetFullPathName(pwszArg, lengthof(wsz), wsz,
                                        &pwszFile);
    pEnumArg->Next();
    if (!cwch || cwch > lengthof(wsz))
     continue;

    COPYDATASTRUCT oData;
    oData.dwData = 1;
    oData.cbData = sizeof(char16) * (cwch + 1);
    oData.lpData = wsz;

    ::SendMessage(p->m_hwnd, WM_COPYDATA, 0,
                  reinterpret_cast<LPARAM>(&oData));
  }
  return 0;
}

static void NoReturn fatalExit(const char16* pwsz) {
  char16 wsz[100];
  ::wsprintf(wsz, L"Evita Text Editor can't start (%s).", pwsz);
  ::FatalAppExit(0, wsz);
}

static void DoIdle() {
  static int idle_count;
  #if DEBUG_IDLE
    DVLOG(4) << "idle_count=" << idle_count << " running=" <<
        base::MessageLoop::current()->is_running();
  #endif
  if (Application::Get()->OnIdle(idle_count))
    ++idle_count;
  else
    idle_count= 0;
  if (!Application::instance().is_quit())
    base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(DoIdle));
}

static int MainLoop(EnumArg* pEnumArg) {
  // Initialize Default Style
  // This initialize must be before creating edit buffers.
  {
      g_DefaultStyle.m_rgfMask =
          StyleValues::Mask_Background |
          StyleValues::Mask_Color |
          StyleValues::Mask_Decoration |
          StyleValues::Mask_FontFamily |
          StyleValues::Mask_FontStyle |
          StyleValues::Mask_FontWeight |
          StyleValues::Mask_Marker |
          StyleValues::Mask_Syntax;

        #if 0
          //g_DefaultStyle->SetBackground(Color(0xF0, 0xF0, 0xF0));
          g_DefaultStyle->SetBackground(Color(247, 247, 239));
          g_DefaultStyle->SetColor(Color(0x00, 0x00, 0x00));
          g_DefaultStyle->SetMarker(Color(0x00, 0x66, 0x00));
      #else
          g_DefaultStyle.m_crBackground = Color(255, 255, 255);
          g_DefaultStyle.m_crColor      = Color(0x00, 0x00, 0x00);
          g_DefaultStyle.m_crMarker     = Color(0x00, 0x99, 0x00);
      #endif

        //#define BaseFont L"Lucida Console"
      //#define BaseFont L"Courier New"
      #define BaseFont L"Consolas, MS Gothic"

        {
          //FontSet* pFontSet = new FontSet;
          //pFontSet->Add(Font::Create(BaseFont, nFontSize, ANSI_CHARSET));
          //pFontSet->Add(Font::Create(L"Courier New", nFontSize, ANSI_CHARSET));
          //pFontSet->Add(Font::Create(L"MS Gothic", nFontSize, SHIFTJIS_CHARSET));
          g_DefaultStyle.m_pwszFontFamily = BaseFont;
          g_DefaultStyle.m_nFontSize      = 10;
      }
  }

  auto& frame = *Application::Get()->CreateFrame();
  while (!pEnumArg->AtEnd()) {
    auto const  pwszArg = pEnumArg->Get();
    auto const buffer = Application::Get()->Load(pwszArg);
    frame.AddWindow(new TextEditWindow(buffer));
    pEnumArg->Next();
  }

  // When there is no filename argument, we start lisp.
  if (!frame.GetFirstPane()) {
    #if USE_LISTENER && _NDEBUG
      auto const buffer = new ListenerBuffer();
      buffer->Start();
    #else // USE_LISTENER
      auto const buffer = new Buffer(L"*scratch*");
    #endif // USE_LISTENER

    Application::Get()->InternalAddBuffer(buffer);
    frame.AddWindow(new TextEditWindow(buffer));
  }
  frame.Realize();

  base::MessageLoop message_loop(base::MessageLoop::TYPE_UI);
  DoIdle();
  base::RunLoop run_loop;
  run_loop.Run();
  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
  base::AtExitManager at_exit;
  CommandLine::Init(0, nullptr);
  {
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    logging::InitLogging(settings);
  }
  common::win::NativeWindow::Init(hInstance);
  g_hInstance = hInstance;
  g_hResource = hInstance;

    EnumArg oEnumArg(::GetCommandLine());
  while (!oEnumArg.AtEnd()) {
    auto const pwszArg = oEnumArg.Get();
    if (*pwszArg != '-')
      break;

      if (!::lstrcmpW(pwszArg, L"-multiple")) {
      g_fMultiple = true;
    } else if (!::lstrcmpW(pwszArg, L"-dll")) {
      oEnumArg.Next();
      if (oEnumArg.AtEnd())
        break;
    } else if (!::lstrcmpW(pwszArg, L"-image")) {
      oEnumArg.Next();
      if (oEnumArg.AtEnd())
        break;
    }

      oEnumArg.Next();
  }

    if (!g_fMultiple) {
    g_hEvent = ::CreateEventW(nullptr,   // lpEventAttrs
                              TRUE,   // fManualReset
                              FALSE,  // fInitialState
                              L"Local\\" SINGLE_INSTANCE_NAME);

      if (!g_hEvent)
      fatalExit(L"CreateEvent");

      if (::GetLastError() == ERROR_ALREADY_EXISTS) {
      auto const nWait = ::WaitForSingleObject(g_hEvent, 30 * 100);
      switch (nWait) {
        case WAIT_OBJECT_0:
          return callRunningApp(&oEnumArg);

          default:
          ::MessageBox(nullptr, L"WaitForSignleObject", k_pwszTitle,
                       MB_APPLMODAL | MB_ICONERROR);
          break;
      }
    }
  }

    // Common Control
  {
      INITCOMMONCONTROLSEX oInit;
      oInit.dwSize = sizeof(oInit);
      oInit.dwICC  = ICC_BAR_CLASSES;
      if (!::InitCommonControlsEx(&oInit)) {
          ::MessageBox(
              nullptr,
              L"InitCommonControlsEx",
              k_pwszTitle,
              MB_APPLMODAL | MB_ICONERROR);
          return 1;
      }
  }

    ::TabBand__Init(g_hInstance);

    g_TabBand__TabDragMsg = ::RegisterWindowMessage(TabBand__TabDragMsgStr);

  return MainLoop(&oEnumArg);
}
