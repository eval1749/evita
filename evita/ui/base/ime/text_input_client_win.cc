// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/base/ime/text_input_client_win.h"

#include <imm.h>
#pragma comment(lib, "imm32.lib")

#include <vector>

#include "base/logging.h"
#include "common/win/win32_verify.h"
#include "evita/gfx/rect.h"
#include "evita/ui/base/ime/text_composition.h"
#include "evita/ui/base/ime/text_input_delegate.h"
#include "evita/ui/widget.h"

namespace ui {

namespace {

//////////////////////////////////////////////////////////////////////
//
// InputMethodContext
//
class InputMethodContext final {
 public:
  explicit InputMethodContext(TextInputDelegate* delegate);
  ~InputMethodContext();

  operator HIMC() const { return handle_; }

  std::vector<TextComposition::Span> GetSpans();
  int GetCursorOffset();
  base::string16 GetText(int index);
  bool SetComposition(int index, uint8_t* bytes, size_t num_bytes);

 private:
  explicit InputMethodContext(HWND hwnd);
  HIMC handle_;
  HWND hwnd_;

  DISALLOW_COPY_AND_ASSIGN(InputMethodContext);
};

InputMethodContext::InputMethodContext(TextInputDelegate* delegate)
    : InputMethodContext(delegate->GetClientWindow()->AssociatedHwnd()) {}

InputMethodContext::InputMethodContext(HWND hwnd)
    : handle_(::ImmGetContext(hwnd)), hwnd_(hwnd) {}

InputMethodContext::~InputMethodContext() {
  if (handle_)
    ::ImmReleaseContext(hwnd_, handle_);
}

std::vector<TextComposition::Span> InputMethodContext::GetSpans() {
  auto const num_bytes =
      ::ImmGetCompositionString(handle_, GCS_COMPATTR, nullptr, 0);
  if (num_bytes < 0) {
    DVLOG(0) << "ImmGetCompositionString GCS_COMPATTR" << num_bytes;
    return std::vector<TextComposition::Span>();
  }
  if (!num_bytes)
    return std::vector<TextComposition::Span>();
  std::vector<uint8_t> attributes(static_cast<size_t>(num_bytes), 0);
  auto const result = ::ImmGetCompositionString(
      handle_, GCS_COMPATTR, &attributes[0], static_cast<DWORD>(num_bytes));
  if (result < num_bytes)
    return std::vector<TextComposition::Span>();
  std::vector<TextComposition::Span> spans;
  auto offset = 0;
  for (auto const attribute : attributes) {
    auto const type = static_cast<TextComposition::Span::Type>(attribute);
    if (spans.empty() || spans.back().type != type) {
      TextComposition::Span span;
      span.start = offset;
      span.type = type;
      spans.push_back(span);
    }
    ++offset;
    spans.back().end = offset;
  }
  return std::move(spans);
}

int InputMethodContext::GetCursorOffset() {
  return ::ImmGetCompositionString(handle_, GCS_CURSORPOS, nullptr, 0);
}

base::string16 InputMethodContext::GetText(int index) {
  auto const num_bytes =
      ::ImmGetCompositionString(handle_, static_cast<DWORD>(index), nullptr, 0);
  if (num_bytes < 0) {
    DVLOG(0) << "ImmGetCompositionString " << index << " " << num_bytes;
    return base::string16();
  }
  if (!num_bytes)
    return base::string16();
  base::string16 text(static_cast<size_t>(num_bytes / sizeof(base::char16)), 0);
  auto const result =
      ::ImmGetCompositionString(handle_, static_cast<DWORD>(index), &text[0],
                                static_cast<DWORD>(num_bytes));
  if (result != num_bytes) {
    DVLOG(0) << "ImmGetCompositionString " << result;
    return base::string16();
  }
  return text;
}

bool InputMethodContext::SetComposition(int index,
                                        uint8_t* bytes,
                                        size_t num_bytes) {
  if (!handle_)
    return false;
  auto const succeeded =
      ::ImmSetCompositionString(handle_, static_cast<DWORD>(index), bytes,
                                static_cast<DWORD>(num_bytes), nullptr, 0);
  if (!succeeded) {
    DVLOG(0) << "ImmSetCompositionString " << index << " failed.";
    return false;
  }
  return true;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextInputClientWin
//
TextInputClientWin::TextInputClientWin() {}

TextInputClientWin::~TextInputClientWin() {}

void TextInputClientWin::DidChangeDelegate(TextInputDelegate* old_delegate) {
  if (!delegate()) {
    if (old_delegate) {
      WIN32_VERIFY(::ImmAssociateContextEx(
          old_delegate->GetClientWindow()->AssociatedHwnd(), nullptr,
          IACE_IGNORENOCONTEXT));
    }
    return;
  }
  InputMethodContext imc(delegate());
  WIN32_VERIFY(::ImmAssociateContextEx(GetHwnd(), imc, IACE_DEFAULT));
}

void TextInputClientWin::DidUpdateComposition(LPARAM lParam) {
  DCHECK(delegate());

  InputMethodContext imc(delegate());
  if (!imc)
    return;

  if (!lParam) {
    // Text composition is canceled.
    set_has_composition(false);
    TextComposition composition;
    delegate()->DidCommitComposition(composition);
    return;
  }

  if (lParam & GCS_RESULTSTR) {
    // Text composition is finished.
    set_has_composition(false);
    TextComposition composition;
    composition.set_text(imc.GetText(GCS_RESULTSTR));
    delegate()->DidCommitComposition(composition);
    return;
  }

  set_has_composition(true);
  TextComposition composition;
  composition.set_caret(imc.GetCursorOffset());
  composition.set_spans(imc.GetSpans());
  composition.set_text(imc.GetText(GCS_COMPSTR));
  delegate()->DidUpdateComposition(composition);
}

HWND TextInputClientWin::GetHwnd() const {
  DCHECK(delegate());
  return delegate()->GetClientWindow()->AssociatedHwnd();
}

std::pair<LRESULT, bool> TextInputClientWin::OnImeMessage(uint32_t message,
                                                          WPARAM wParam,
                                                          LPARAM lParam) {
  if (!delegate())
    return std::make_pair(0, false);

  switch (message) {
    case WM_IME_COMPOSITION:
      DidUpdateComposition(lParam);
      return std::make_pair(0, true);

    case WM_IME_ENDCOMPOSITION:
      delegate()->DidFinishComposition();
      return std::make_pair(0, true);

    case WM_IME_REQUEST:
      return std::make_pair(0, false);

    case WM_IME_SETCONTEXT: {
      // We draw composition string instead of IME. So, we don't
      // need default composition window.
      lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
      return std::make_pair(::DefWindowProc(GetHwnd(), message, wParam, lParam),
                            true);
    }

    case WM_IME_STARTCOMPOSITION:
      delegate()->DidStartComposition();
      return std::make_pair(0, true);
  }
  return std::make_pair(0, false);
}

void TextInputClientWin::Start() {}

// ui::TextInputClient
void TextInputClientWin::CancelComposition(TextInputDelegate* requester) {
  if (delegate() != requester || !has_composition())
    return;
  InputMethodContext imc(delegate());
  if (!imc)
    return;
  set_has_composition(false);
  WIN32_VERIFY(::ImmNotifyIME(imc, NI_COMPOSITIONSTR, CPS_CANCEL, 0));
}

void TextInputClientWin::CommitComposition(TextInputDelegate* requester) {
  if (delegate() != requester || !has_composition())
    return;
  InputMethodContext imc(delegate());
  if (!imc)
    return;
  WIN32_VERIFY(::ImmNotifyIME(imc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0));
}

void TextInputClientWin::DidChangeCaretBounds() {
  if (!delegate() || !has_composition())
    return;
  InputMethodContext imc(delegate());
  if (!imc)
    return;

  // Set left top coordinate of IME candidate window.
  CANDIDATEFORM param;
  param.dwIndex = 0;
  param.dwStyle = CFS_EXCLUDE;
  param.ptCurrentPos.x = static_cast<int>(caret_bounds().left);
  param.ptCurrentPos.y = static_cast<int>(caret_bounds().bottom);
  param.rcArea.left = param.ptCurrentPos.x;
  param.rcArea.top = static_cast<int>(caret_bounds().top);
  param.rcArea.right = static_cast<int>(caret_bounds().right);
  param.rcArea.bottom = param.ptCurrentPos.y;
  WIN32_VERIFY(::ImmSetCandidateWindow(imc, &param));
}

// Note:
// o IME2000 ignores string after newline.
// o We should limit number of characters to be reconverted.
void TextInputClientWin::Reconvert(TextInputDelegate* requester,
                                   const base::string16& text) {
  if (requester != delegate() || text.empty())
    return;

  auto const num_text_bytes = sizeof(base::char16) * text.length();
  std::vector<uint8_t> buffer(sizeof(RECONVERTSTRING) + num_text_bytes);
  auto const reconvert = reinterpret_cast<RECONVERTSTRING*>(&buffer[0]);
  reconvert->dwSize = static_cast<DWORD>(buffer.size());
  reconvert->dwVersion = 0;
  reconvert->dwStrLen = static_cast<DWORD>(text.length());
  reconvert->dwStrOffset = sizeof(RECONVERTSTRING);
  reconvert->dwCompStrLen = static_cast<DWORD>(text.length());
  reconvert->dwCompStrOffset = 0;
  reconvert->dwTargetStrLen = reconvert->dwCompStrLen;
  reconvert->dwTargetStrOffset = reconvert->dwCompStrOffset;
  auto reconvert_string =
      reinterpret_cast<base::char16*>(&buffer[reconvert->dwStrOffset]);
  ::memcpy(reconvert_string, text.data(), num_text_bytes);

  InputMethodContext imc(delegate());
  if (!imc.SetComposition(SCS_QUERYRECONVERTSTRING, &buffer[0], buffer.size()))
    return;
  // We'll have WM_IME_STARTCOMPOSITION and WM_IME_COMPOSITION if
  // |SetComposition()| succeeded.
  imc.SetComposition(SCS_SETRECONVERTSTRING, &buffer[0], buffer.size());
}

}  // namespace ui
