#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - Text Edit Window
// listener/winapp/vi_TextEditWindow.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_TextEditWindow.cpp#3 $
//
#include "evita/vi_TextEditWindow.h"

#include <algorithm>
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/bind.h"
#pragma warning(pop)
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "common/timer/timer.h"
#include "evita/gfx_base.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/dom/public/text_composition_data.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/text/document.h"
#include "evita/dom/text/range.h"
#include "evita/dom/windows/selection.h"
#include "evita/dom/windows/text_window.h"
#include "evita/metrics/time_scope.h"
#include "evita/text/buffer.h"
#include "evita/text/marker_set.h"
#include "evita/text/range.h"
#include "evita/ui/base/ime/text_composition.h"
#include "evita/ui/base/ime/text_input_client.h"
#include "evita/ui/controls/scroll_bar.h"
#include "evita/views/frame_list.h"
#include "evita/views/icon_cache.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_text_line.h"
#include "evita/views/text/text_renderer.h"
#include "evita/vi_Caret.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Frame.h"
#include "evita/vi_Selection.h"

using views::rendering::TextSelectionModel;

namespace {

bool IsPopupWindow(HWND hwnd) {
  while (hwnd) {
    auto const dwStyle = static_cast<DWORD>(::GetWindowLong(hwnd, GWL_STYLE));
    if (dwStyle & WS_POPUP)
      return true;
    if (!(dwStyle & WS_CHILD))
      return false;
    hwnd = ::GetParent(hwnd);
  }
  return false;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextEditWindow
//
TextEditWindow::TextEditWindow(const dom::TextWindow& text_window)
    : ContentWindow(text_window.window_id()),
      caret_(new Caret()),
      m_gfx(nullptr),
      m_lCaretPosn(-1),
      text_renderer_(new TextRenderer(text_window.document()->buffer())),
      selection_(text_window.view_selection()),
      vertical_scroll_bar_(new ui::ScrollBar(ui::ScrollBar::Type::Vertical,
                                             this)),
      view_start_(0), zoom_(1.0f) {
  AppendChild(vertical_scroll_bar_);
  buffer()->AddObserver(this);
}

TextEditWindow::~TextEditWindow() {
  buffer()->RemoveObserver(this);
}

text::Buffer* TextEditWindow::buffer() const {
  return text_renderer_->GetBuffer();
}

bool TextEditWindow::is_selection_active() const {
  if (has_focus())
    return true;

  if (!IsPopupWindow(::GetFocus()))
    return false;

  auto const edit_pane = views::FrameList::instance()->active_frame()->
    GetActivePane()->as<EditPane>();
  return edit_pane && edit_pane->GetActiveWindow() == this;
}

void TextEditWindow::set_zoom(float new_zoom) {
  DCHECK_GT(new_zoom, 0.0f);
  if (zoom_ == new_zoom)
    return;
  zoom_ = new_zoom;
}

Posn TextEditWindow::computeGoalX(float xGoal, Posn lGoal) {
  if (xGoal < 0)
    return lGoal;

  if (!text_renderer_->ShouldFormat(zoom_)) {
    if (auto const line = text_renderer_->FindLine(lGoal))
      return line->MapXToPosn(*m_gfx, xGoal);
  }

  auto lStart = buffer()->ComputeStartOfLine(lGoal);
  for (;;) {
    auto const pLine = text_renderer_->FormatLine(lStart);
    auto const lEnd = pLine->GetEnd();
    if (lGoal < lEnd)
      return pLine->MapXToPosn(*m_gfx, xGoal);
    lStart = lEnd;
  }
}

text::Posn TextEditWindow::ComputeMotion(
    Unit eUnit, Count n, const gfx::PointF& pt, text::Posn lPosn) {
  UI_ASSERT_DOM_LOCKED();
  switch (eUnit) {
    case Unit_WindowLine:
      if (n > 0) {
        auto const lBufEnd = buffer()->GetEnd();
        auto lGoal = lPosn;
        auto k = 0;
        for (k = 0; k < n; ++k) {
          lGoal = EndOfLine(lGoal);
          if (lGoal >= lBufEnd)
            break;
          ++lGoal;
        }
        return computeGoalX(pt.x, std::min(lGoal, lBufEnd));
      }
      if (n < 0) {
        n = -n;
        auto const lBufStart = buffer()->GetStart();
        auto lStart = lPosn;
        auto k = 0;
        for (k = 0; k < n; ++k) {
          lStart = StartOfLine(lStart);
          if (lStart <= lBufStart)
            break;
          --lStart;
        }

        return computeGoalX(pt.x, std::max(lStart, lBufStart));
      }
      return lPosn;

    case Unit_Screen: {
      auto k = LargeScroll(0, n, false);
      if (k > 0) {
        auto const lStart = text_renderer_->GetStart();
        view_start_ = lStart;
        return MapPointToPosn(pt);
      }
      if (n > 0)
        return std::min(GetEnd(), buffer()->GetEnd());
      if (n < 0)
        return GetStart();
      return lPosn;
    }

    case Unit_Window:
      if (n > 0)
        return std::min(GetEnd(), buffer()->GetEnd());
      if (n < 0)
        return GetStart();
      return lPosn;
  }

  LOG(ERROR) << "Unsupported unit " << eUnit;
  return lPosn;
}

void TextEditWindow::DidChangeHierarchy() {
  ParentClass::DidChangeHierarchy();
  m_gfx = &frame().gfx();
  text_renderer_->SetGraphics(m_gfx);
}

void TextEditWindow::DidHide() {
  // Note: It is OK that hidden window have focus.
  vertical_scroll_bar_->Hide();
  text_renderer_->Reset();
}

void TextEditWindow::DidKillFocus(ui::Widget* focused_widget) {
  ParentClass::DidKillFocus(focused_widget);
  caret_->Give(this, m_gfx);
  ui::TextInputClient::Get()->CommitComposition(this);
  ui::TextInputClient::Get()->CancelComposition(this);
  ui::TextInputClient::Get()->set_delegate(nullptr);
}

void TextEditWindow::DidRealize() {
  ParentClass::DidRealize();
  auto const frame = Frame::FindFrame(*this);
  ASSERT(frame);
  m_gfx = &frame->gfx();
  text_renderer_->SetGraphics(m_gfx);
}

void TextEditWindow::DidSetFocus(ui::Widget* last_focused) {
  ASSERT(has_focus());
  // Note: It is OK to set focus to hidden window.
  caret_->Take(this);
  ui::TextInputClient::Get()->set_delegate(this);
  ParentClass::DidSetFocus(last_focused);
}

void TextEditWindow::DidShow() {
  vertical_scroll_bar_->Show();
}

Posn TextEditWindow::EndOfLine(Posn lPosn) {
  UI_ASSERT_DOM_LOCKED();
  if (!text_renderer_->ShouldFormat(zoom_)) {
    auto const pLine = text_renderer_->FindLine(lPosn);
    if (pLine)
      return pLine->GetEnd() - 1;
  }

  auto const lBufEnd = buffer()->GetEnd();
  if (lPosn >= lBufEnd)
    return lBufEnd;

  auto lStart = buffer()->ComputeStartOfLine(lPosn);
  for (;;) {
    auto const pLine = text_renderer_->FormatLine(lStart);
    lStart = pLine->GetEnd();
    if (lPosn < lStart)
      return lStart - 1;
  }
}

HCURSOR TextEditWindow::GetCursorAt(const Point& point) const {
  if (vertical_scroll_bar_->bounds().Contains(point))
    return ::LoadCursor(nullptr, IDC_ARROW);
  return ::LoadCursor(nullptr, IDC_IBEAM);
}

// For Selection.MoveDown Screen
Posn TextEditWindow::GetEnd() {
  UI_ASSERT_DOM_LOCKED();
  updateScreen();
  return text_renderer_->GetEnd();
}

//For Selection.MoveUp Screen
Posn TextEditWindow::GetStart() {
  UI_ASSERT_DOM_LOCKED();
  updateScreen();
  return text_renderer_->GetStart();
}
// Description:
// Maps position specified buffer position and returns height
// of caret, If specified buffer position isn't in window, this function
// returns 0.
gfx::RectF TextEditWindow::HitTestTextPosition(Posn lPosn) {
  DCHECK_GE(lPosn, 0);
  UI_ASSERT_DOM_LOCKED();
  updateScreen();
  for (;;) {
    if (auto rect = text_renderer_->HitTestTextPosition(lPosn))
      return rect;
    text_renderer_->ScrollToPosn(lPosn);
  }
}

int TextEditWindow::LargeScroll(int, int iDy, bool fRender) {
  UI_ASSERT_DOM_LOCKED();
  updateScreen();

  auto k = 0;
  if (iDy < 0) {
    // Scroll Down -- place top line out of window.
    iDy = -iDy;

    auto const lBufStart = buffer()->GetStart();
    for (k = 0; k < iDy; ++k) {
      auto const lStart = text_renderer_->GetStart();
      if (lStart == lBufStart)
        break;

      // Scroll down until page start goes out to page.
      do {
        if (!text_renderer_->ScrollDown())
          break;
      } while (text_renderer_->GetEnd() != lStart);
    }
  } else if (iDy > 0) {
    // Scroll Up -- format page from page end.
    const Posn lBufEnd = buffer()->GetEnd();
    for (k = 0; k < iDy; ++k) {
      auto const lStart = text_renderer_->GetEnd();
      if (lStart >= lBufEnd)
        break;
      text_renderer_->Format(lStart);
    }
  }

  if (fRender && k > 0)
    Render();
  return k;
}

void TextEditWindow::MakeSelectionVisible() {
  m_lCaretPosn = -1;
  Redraw();
}

Posn TextEditWindow::MapPointToPosn(const gfx::PointF pt) {
  updateScreen();
  return std::min(text_renderer_->MapPointToPosn(pt), buffer()->GetEnd());
}

void TextEditWindow::OnDraw(gfx::Canvas*) {
  UI_ASSERT_DOM_LOCKED();
  text_renderer_->Reset();
  Redraw();
}

void TextEditWindow::Redraw() {
  if (!is_shown())
    return;

  auto const selection_is_active = is_selection_active();

  UI_ASSERT_DOM_LOCKED();

  auto const lSelStart = selection_->GetStart();
  auto const lSelEnd = selection_->GetEnd();

  TextSelectionModel selection(lSelStart, lSelEnd, selection_is_active);

  Posn lCaretPosn;
  if (selection_is_active) {
    lCaretPosn = selection_->IsStartActive() ? lSelStart : lSelEnd;
  } else {
    lCaretPosn = m_lCaretPosn == -1 ? lSelStart : m_lCaretPosn;

    Posn lEnd = buffer()->GetEnd();
    if (lSelStart == lEnd && lSelEnd == lEnd)
      lCaretPosn = lEnd;
  }

  DCHECK_GE(lCaretPosn, 0);

  if (text_renderer_->ShouldFormat(zoom_)) {
    text_renderer_->Prepare(selection, zoom_);
    text_renderer_->Format(StartOfLine(view_start_));

    if (m_lCaretPosn != lCaretPosn) {
      // FIXME 2007-05-12 Fill the page with lines.
      text_renderer_->ScrollToPosn(lCaretPosn);
      m_lCaretPosn = lCaretPosn;
    }
    Render();
    return;
  }

  if (m_lCaretPosn != lCaretPosn) {
    m_lCaretPosn = lCaretPosn;
    const auto char_rect = text_renderer_->HitTestTextPosition(lCaretPosn);
    if (!char_rect.empty()) {
      if (text_renderer_->ShouldRender()) {
        Render();
        return;
      }
      caret_->Hide(m_gfx);
      text_renderer_->RenderSelectionIfNeeded(selection);
      UpdateCaretBounds(char_rect);
      return;
    }
    text_renderer_->Prepare(selection, zoom_);
    text_renderer_->ScrollToPosn(lCaretPosn);
    Render();
    return;
  }

  if (text_renderer_->GetStart() != view_start_) {
    text_renderer_->Prepare(selection, zoom_);
    text_renderer_->Format(StartOfLine(view_start_));
    Render();
    return;
  }

  if (text_renderer_->ShouldRender()) {
    Render();
    return;
  }

  // The screen is clean.
  if (selection.is_range()) {
    Caret::HideScope hide_scope(caret_.get(), m_gfx);
    text_renderer_->RenderSelectionIfNeeded(selection);
  }
  caret_->Blink(m_gfx);
}

void TextEditWindow::Render() {
  if (!is_shown())
    return;

  gfx::Canvas::DrawingScope drawing_scope(*m_gfx);
  m_gfx->set_dirty_rect(gfx::Rect(
      bounds().left_top(),
      gfx::Size(bounds().width() - vertical_scroll_bar_->bounds().width(),
                bounds().height())));
  text_renderer_->Render();

  view_start_ = text_renderer_->GetStart();
  updateScrollBar();
  static_cast<Widget*>(vertical_scroll_bar_)->OnDraw(m_gfx);

  const auto char_rect = text_renderer_->HitTestTextPosition(m_lCaretPosn);
  if (char_rect.empty()) {
    Caret::Updater caret_updater(caret_.get());
    caret_updater.Clear();
    return;
  }

  UpdateCaretBounds(char_rect);
}

int TextEditWindow::SmallScroll(int, int iDy) {
  UI_ASSERT_DOM_LOCKED();
  updateScreen();

  if (iDy < 0) {
    iDy = -iDy;

    auto const lBufStart = buffer()->GetStart();
    auto lStart = text_renderer_->GetStart();
    int k;
    for (k = 0; k < iDy; ++k) {
      if (lStart == lBufStart)
        break;
        lStart = StartOfLine(lStart - 1);
    }

    if (k > 0) {
      text_renderer_->Format(lStart);
      Render();
    }
    return k;
  }

  if (iDy > 0) {
    auto const lBufEnd = buffer()->GetEnd();
    int k;
    for (k = 0; k < iDy; ++k) {
      if (text_renderer_->GetEnd() >= lBufEnd) {
          // Make sure whole line of buffer end is visible.
          text_renderer_->ScrollToPosn(lBufEnd);
          ++k;
          break;
      }

      if (!text_renderer_->ScrollUp())
        break;
    }

    if (k > 0)
        Render();
    return k;
  }

  return 0;
}

Posn TextEditWindow::StartOfLine(Posn lPosn) {
  UI_ASSERT_DOM_LOCKED();
  if (lPosn <= 0 )
    return 0;

  if (!text_renderer_->ShouldFormat(zoom_)) {
    auto const pLine = text_renderer_->FindLine(lPosn);
    if (pLine)
      return pLine->GetStart();
  }

  auto lStart = buffer()->ComputeStartOfLine(lPosn);
  if (!lStart)
    return 0;

  for (;;) {
    auto const pLine = text_renderer_->FormatLine(lStart);
    auto const lEnd = pLine->GetEnd();
    if (lPosn < lEnd)
      return pLine->GetStart();
    lStart = lEnd;
  }
}

void TextEditWindow::UpdateCaretBounds(const gfx::RectF& char_rect) {
  DCHECK(!char_rect.empty());
  auto const caret_width = std::max(::GetSystemMetrics(SM_CXBORDER), 2);
  gfx::RectF caret_bounds(char_rect.left, char_rect.top,
                          std::min(char_rect.left + caret_width,
                                   static_cast<float>(bounds().right)),
                          std::min(char_rect.bottom,
                                   static_cast<float>(bounds().bottom)));

  ui::TextInputClient::Get()->set_caret_bounds(caret_bounds);
  Caret::Updater caret_updater(caret_.get());
  caret_updater.Update(m_gfx, caret_bounds);
}

void TextEditWindow::updateScreen() {
  UI_ASSERT_DOM_LOCKED();
  TextSelectionModel selection(selection_->GetStart(), selection_->GetEnd(),
                               is_selection_active());
  if (!text_renderer_->ShouldFormat(zoom_)) {
    text_renderer_->RenderSelectionIfNeeded(selection);
    return;
  }
  text_renderer_->Prepare(selection, zoom_);
  auto const line_start_ = StartOfLine(view_start_);
  text_renderer_->Format(line_start_);
}

void TextEditWindow::updateScrollBar() {
  auto const lBufEnd = buffer()->GetEnd() + 1;
  ui::ScrollBar::Data data;
  data.minimum = 0;
  data.thumb_size = text_renderer_->GetVisibleEnd() -
        text_renderer_->GetStart();
  data.thumb_value = text_renderer_->GetStart();
  // Current screen shows entire buffer. We disable scroll bar.
  data.maximum = data.thumb_size < lBufEnd ? lBufEnd : 0;
  vertical_scroll_bar_->SetData(data);
}

// text::BufferMutationObserver
void TextEditWindow::DidDeleteAt(text::Posn offset, size_t length) {
  if (view_start_ <= offset)
    return;
  view_start_ = std::max(static_cast<text::Posn>(view_start_ - length),
                         offset);
}

void TextEditWindow::DidInsertAt(text::Posn offset, size_t length) {
  if (view_start_ <= offset)
    return;
  view_start_ += length;
}

// ui::ScrollBarObserver
void TextEditWindow::DidClickLineDown() {
  UI_DOM_AUTO_LOCK_SCOPE();
  SmallScroll(0, 1);
}

void TextEditWindow::DidClickLineUp() {
  UI_DOM_AUTO_LOCK_SCOPE();
  SmallScroll(0, -1);
}

void TextEditWindow::DidClickPageDown() {
  UI_DOM_AUTO_LOCK_SCOPE();
  LargeScroll(0, 1);
}

void TextEditWindow::DidClickPageUp() {
  UI_DOM_AUTO_LOCK_SCOPE();
  LargeScroll(0, -1);
}

void TextEditWindow::DidMoveThumb(int value) {
  UI_DOM_AUTO_LOCK_SCOPE();
  auto const start = StartOfLine(value);
  text_renderer_->Format(start);
  Render();
}

// ui::TextInputDelegate
void TextEditWindow::DidCommitComposition(
    const ui::TextComposition& composition) {
  DispatchTxetCompositionEvent(domapi::EventType::TextCompositionCommit,
                               composition);
}

void TextEditWindow::DidFinishComposition() {
  DispatchTxetCompositionEvent(domapi::EventType::TextCompositionEnd,
                               ui::TextComposition());
}
 
void TextEditWindow::DidStartComposition() {
  DispatchTxetCompositionEvent(domapi::EventType::TextCompositionStart,
                               ui::TextComposition());
}
 
void TextEditWindow::DidUpdateComposition(
    const ui::TextComposition& composition) {
  DispatchTxetCompositionEvent(domapi::EventType::TextCompositionUpdate,
                               composition);
}

ui::Widget* TextEditWindow::GetClientWindow() {
   return this;
}

// ui::Widget
void TextEditWindow::DidResize() {
  views::ContentWindow::DidResize();
  UI_DOM_AUTO_LOCK_SCOPE();
  auto const scroll_bar_width = ::GetSystemMetrics(SM_CXVSCROLL);
  auto text_block_rect = bounds();
  text_block_rect.right -= scroll_bar_width;
  text_renderer_->SetBounds(text_block_rect);

  auto scroll_bar_rect = bounds();
  scroll_bar_rect.left = text_block_rect.right;
  vertical_scroll_bar_->SetBounds(scroll_bar_rect);
}

// views::Window
bool TextEditWindow::OnIdle(int) {
  if (!is_shown())
    return false;
  Redraw();
  return false;
}
