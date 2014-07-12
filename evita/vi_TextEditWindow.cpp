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
#include "evita/dom/lock.h"
#include "evita/gfx/canvas.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/dom/public/text_composition_data.h"
#include "evita/dom/public/view_event.h"
#include "evita/metrics/time_scope.h"
#include "evita/text/buffer.h"
#include "evita/text/marker_set.h"
#include "evita/text/range.h"
#include "evita/text/selection.h"
#include "evita/ui/base/ime/text_composition.h"
#include "evita/ui/base/ime/text_input_client.h"
#include "evita/ui/controls/scroll_bar.h"
#include "evita/views/frame_list.h"
#include "evita/views/icon_cache.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_text_line.h"
#include "evita/views/text/text_renderer.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Frame.h"

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

TextSelectionModel::State GetTextSelectionState(TextEditWindow* window) {
  if (window->has_focus())
    return TextSelectionModel::State::HasFocus;

  if (!IsPopupWindow(::GetFocus()))
    return TextSelectionModel::State::Disabled;

  auto const edit_pane = views::FrameList::instance()->active_frame()->
    GetActivePane()->as<EditPane>();
  if (edit_pane && edit_pane->GetActiveWindow() == window)
    return TextSelectionModel::State::Highlight;
  return TextSelectionModel::State::Disabled;
}

TextSelectionModel GetTextSelectionModel(
    TextEditWindow* window, const text::Selection& selection) {
  return TextSelectionModel(
      GetTextSelectionState(window),
      selection.anchor_offset(), selection.focus_offset());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextEditWindow
//
TextEditWindow::TextEditWindow(views::WindowId window_id,
                               text::Selection* selection)
    : ContentWindow(window_id),
      canvas_(nullptr),
      m_lCaretPosn(-1),
      text_renderer_(new TextRenderer(selection->buffer())),
      selection_(selection),
      vertical_scroll_bar_(new ui::ScrollBar(ui::ScrollBar::Type::Vertical,
                                             this)),
      view_start_(0) {
  AppendChild(vertical_scroll_bar_);
  buffer()->AddObserver(this);
}

TextEditWindow::~TextEditWindow() {
  buffer()->RemoveObserver(this);
}

text::Buffer* TextEditWindow::buffer() const {
  return text_renderer_->GetBuffer();
}

Posn TextEditWindow::computeGoalX(float xGoal, Posn lGoal) {
  if (xGoal < 0)
    return lGoal;

  if (!text_renderer_->ShouldFormat()) {
    if (auto const line = text_renderer_->FindLine(lGoal))
      return line->MapXToPosn(*canvas_, xGoal);
  }

  auto lStart = buffer()->ComputeStartOfLine(lGoal);
  for (;;) {
    auto const pLine = text_renderer_->FormatLine(lStart);
    auto const lEnd = pLine->GetEnd();
    if (lGoal < lEnd)
      return pLine->MapXToPosn(*canvas_, xGoal);
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
      if (LargeScroll(0, n)) {
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
        return std::max(std::min(GetEnd() - 1, buffer()->GetEnd()),
                        GetStart());
      if (n < 0)
        return GetStart();
      return lPosn;
  }

  LOG(ERROR) << "Unsupported unit " << eUnit;
  return lPosn;
}

Posn TextEditWindow::EndOfLine(Posn lPosn) {
  UI_ASSERT_DOM_LOCKED();
  if (!text_renderer_->ShouldFormat()) {
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

void TextEditWindow::FormatTextBlockIfNeeded() {
  UI_ASSERT_DOM_LOCKED();
  if (!text_renderer_->ShouldFormat())
    return;
  auto const line_start_ = StartOfLine(view_start_);
  text_renderer_->Format(line_start_);
}

// For Selection.MoveDown Screen
Posn TextEditWindow::GetEnd() {
  UI_ASSERT_DOM_LOCKED();
  FormatTextBlockIfNeeded();
  return text_renderer_->GetVisibleEnd();
}

//For Selection.MoveUp Screen
Posn TextEditWindow::GetStart() {
  UI_ASSERT_DOM_LOCKED();
  FormatTextBlockIfNeeded();
  return text_renderer_->GetStart();
}
// Description:
// Maps position specified buffer position and returns height
// of caret, If specified buffer position isn't in window, this function
// returns 0.
gfx::RectF TextEditWindow::HitTestTextPosition(Posn lPosn) {
  DCHECK_GE(lPosn, 0);
  UI_ASSERT_DOM_LOCKED();
  FormatTextBlockIfNeeded();
  for (;;) {
    if (auto rect = text_renderer_->HitTestTextPosition(lPosn))
      return rect;
    text_renderer_->ScrollToPosn(lPosn);
  }
}

bool TextEditWindow::LargeScroll(int, int iDy) {
  UI_ASSERT_DOM_LOCKED();
  FormatTextBlockIfNeeded();

  auto scrolled = false;
  if (iDy < 0) {
    // Scroll Down -- place top line out of window.
    iDy = -iDy;

    auto const lBufStart = buffer()->GetStart();
    for (auto k = 0; k < iDy; ++k) {
      auto const lStart = text_renderer_->GetStart();
      if (lStart == lBufStart)
        break;

      // Scroll down until page start goes out to page.
      do {
        if (!text_renderer_->ScrollDown())
          break;
        scrolled = true;
      } while (text_renderer_->GetEnd() != lStart);
    }
  } else if (iDy > 0) {
    // Scroll Up -- format page from page end.
    const Posn lBufEnd = buffer()->GetEnd();
    for (auto k = 0; k < iDy; ++k) {
      auto const lStart = text_renderer_->GetEnd();
      if (lStart >= lBufEnd)
        break;
      text_renderer_->Format(lStart);
      scrolled = true;
    }
  }
  return scrolled;
}

Posn TextEditWindow::MapPointToPosn(const gfx::PointF pt) {
  FormatTextBlockIfNeeded();
  return std::min(text_renderer_->MapPointToPosn(pt), buffer()->GetEnd());
}

void TextEditWindow::Render(const TextSelectionModel& selection) {
  UI_ASSERT_DOM_LOCKED();
  if (!is_shown())
    return;

  gfx::Canvas::DrawingScope drawing_scope(*canvas_);
  canvas_->set_dirty_rect(gfx::Rect(
      bounds().left_top(),
      gfx::Size(bounds().width() - vertical_scroll_bar_->bounds().width(),
                bounds().height())));
  text_renderer_->Render(selection);

  view_start_ = text_renderer_->GetStart();
  updateScrollBar();
  static_cast<Widget*>(vertical_scroll_bar_)->OnDraw(canvas_);
}

void TextEditWindow::Render() {
  auto const selection = GetTextSelectionModel(this, *selection_);
  Render(selection);
}

void TextEditWindow::SetZoom(float new_zoom) {
  text_renderer_->set_zoom(new_zoom);
}

bool TextEditWindow::SmallScroll(int, int y_count) {
  UI_ASSERT_DOM_LOCKED();
  FormatTextBlockIfNeeded();

  bool scrolled = false;
  if (y_count < 0) {
    for (auto k = y_count; k; ++k) {
      if (!text_renderer_->ScrollDown())
        break;
      scrolled = true;
    }
  } else if (y_count > 0) {
    for (auto k = 0; k < y_count; ++k) {
      if (!text_renderer_->ScrollUp())
        break;
      scrolled = true;
    }
 }

  return scrolled;
}

Posn TextEditWindow::StartOfLine(Posn lPosn) {
  UI_ASSERT_DOM_LOCKED();
  if (lPosn <= 0 )
    return 0;

  if (!text_renderer_->ShouldFormat()) {
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
  ASSERT_DOM_LOCKED();
  if (view_start_ <= offset)
    return;
  view_start_ = std::max(static_cast<text::Posn>(view_start_ - length),
                         offset);
}

void TextEditWindow::DidInsertAt(text::Posn offset, size_t length) {
  ASSERT_DOM_LOCKED();
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
void TextEditWindow::DidChangeHierarchy() {
  ParentClass::DidChangeHierarchy();
  canvas_ = &frame().gfx();
  text_renderer_->SetCanvas(canvas_);
}

void TextEditWindow::DidHide() {
  // Note: It is OK that hidden window have focus.
  vertical_scroll_bar_->Hide();
  text_renderer_->Reset();
}

void TextEditWindow::DidKillFocus(ui::Widget* focused_widget) {
  ParentClass::DidKillFocus(focused_widget);
  text_renderer_->DidKillFocus();
  ui::TextInputClient::Get()->CommitComposition(this);
  ui::TextInputClient::Get()->CancelComposition(this);
  ui::TextInputClient::Get()->set_delegate(nullptr);
}

void TextEditWindow::DidRealize() {
  ParentClass::DidRealize();
  auto const frame = Frame::FindFrame(*this);
  ASSERT(frame);
  canvas_ = &frame->gfx();
  text_renderer_->SetCanvas(canvas_);
}

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

void TextEditWindow::DidSetFocus(ui::Widget* last_focused) {
  ASSERT(has_focus());
  // Note: It is OK to set focus to hidden window.
  text_renderer_->DidSetFocus();
  ui::TextInputClient::Get()->set_delegate(this);
  ParentClass::DidSetFocus(last_focused);
}

void TextEditWindow::DidShow() {
  vertical_scroll_bar_->Show();
}

HCURSOR TextEditWindow::GetCursorAt(const Point& point) const {
  if (vertical_scroll_bar_->bounds().Contains(point))
    return ::LoadCursor(nullptr, IDC_ARROW);
  return ::LoadCursor(nullptr, IDC_IBEAM);
}

void TextEditWindow::OnDraw(gfx::Canvas*) {
  UI_ASSERT_DOM_LOCKED();
  text_renderer_->Reset();
  Redraw();
}

// views::ContentWindow
void TextEditWindow::MakeSelectionVisible() {
  m_lCaretPosn = -1;
}

void TextEditWindow::Redraw() {
  UI_ASSERT_DOM_LOCKED();

  if (!is_shown())
    return;

  auto const selection = GetTextSelectionModel(this, *selection_);
  Posn lCaretPosn;
  if (selection.disabled()) {
    auto const max_offset = buffer()->GetEnd();
    if (selection.start() == max_offset && selection.end() == max_offset)
      lCaretPosn = max_offset;
    else
      lCaretPosn = m_lCaretPosn == -1 ? selection.focus_offset() : m_lCaretPosn;
  } else {
    lCaretPosn = selection.focus_offset();
  }

  DCHECK_GE(lCaretPosn, 0);

  if (text_renderer_->ShouldFormat()) {
    text_renderer_->Format(StartOfLine(view_start_));

    if (m_lCaretPosn != lCaretPosn) {
      // FIXME 2007-05-12 Fill the page with lines.
      text_renderer_->ScrollToPosn(lCaretPosn);
      m_lCaretPosn = lCaretPosn;
    }
    Render(selection);
    return;
  }

  if (m_lCaretPosn != lCaretPosn) {
    m_lCaretPosn = lCaretPosn;
    if (text_renderer_->IsPositionFullyVisible(lCaretPosn)) {
      if (text_renderer_->ShouldRender())
        Render(selection);
      else
        text_renderer_->RenderSelectionIfNeeded(selection);
      return;
    }
    text_renderer_->ScrollToPosn(lCaretPosn);
    Render(selection);
    return;
  }

  if (text_renderer_->ShouldRender()) {
    Render(selection);
    return;
  }

  // The screen is clean.
  text_renderer_->RenderSelectionIfNeeded(selection);
}

// views::Window
bool TextEditWindow::OnIdle(int) {
  if (!is_shown())
    return false;
  Redraw();
  return false;
}
