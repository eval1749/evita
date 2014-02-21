// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_text_renderer_h)
#define INCLUDE_evita_views_text_text_renderer_h

#include "evita/gfx_base.h"
#include "evita/vi_style.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/views/text/render_text_block.h"

class Font;
class Selection;
class Style;

namespace views {

namespace rendering {
  class Cell;
  class TextBlock;
  class TextFormatter;
  class TextLine;
};

//////////////////////////////////////////////////////////////////////
//
// TextRenderer
//
class TextRenderer : public text::BufferMutationObserver {
  // TODO(yosi) Once we make |TextFormatter| using |TextBlock|, we should not
  // make |TextFormatter| as friend class of |TextRenderer|.
  friend class rendering::TextFormatter;

  private: typedef common::win::Rect Rect;
  private: typedef rendering::Cell Cell;
  public: typedef rendering::TextBlock TextBlock;
  public: typedef rendering::TextLine Line;

  // Buffer
  public: text::Buffer* const m_pBuffer;

  // Selection
  public: Posn m_lSelStart;;
  public: Posn m_lSelEnd;
  public: Color m_crSelFg;
  public: Color m_crSelBg;

  public: Color m_crBackground;

  private: Posn m_lStart;
  private: Posn m_lEnd;

  private: TextBlock m_oFormatBuf;
  private: TextBlock m_oScreenBuf;

  public: TextRenderer(text::Buffer* buffer);
  public: ~TextRenderer();

  // [F]
  private: void fillBottom(const gfx::Graphics&) const;
  private: void fillRight(const gfx::Graphics&, const Line*) const;
  private: void formatAux(const gfx::Graphics&, const gfx::RectF, Posn);
  public: Line* FindLine(Posn) const;
  public: void Format(const gfx::Graphics&, gfx::RectF, const Selection&,
                      Posn);
  public: Line* FormatLine(const gfx::Graphics& gfx,
                           const gfx::RectF& page_rect,
                           const Selection&, Posn start);

  // [G]
  public: text::Buffer* GetBuffer() const { return m_pBuffer; }
  public: Line* GetFirstLine() const { return m_oFormatBuf.GetFirst(); }
  public: Line* GetLastLine() const { return m_oFormatBuf.GetLast(); }
  public: Posn GetStart() const { return m_lStart; }
  public: Posn GetEnd() const { return m_lEnd; }

  // [I]
  private: bool isPosnVisible(Posn) const;

  // [M]
  public: void MakePosnVisible(Posn);
  public: Posn MapPointToPosn(const gfx::Graphics&, gfx::PointF) const;
  public: gfx::RectF MapPosnToPoint(const gfx::Graphics&, Posn) const;

  // [P]
  private: int pageLines(const gfx::Graphics&) const;
  private: void Prepare(const Selection&);

  // [R]
  public: bool Render(const gfx::Graphics&);
  public: void Reset();

  // [S]
  public: bool ScrollDown(const gfx::Graphics&);
  public: bool ScrollToPosn(const gfx::Graphics&, Posn target_position);
  public: bool ScrollUp(const gfx::Graphics&);
  public: void SetBufferDirtyOffset(Posn offset);
  public: bool ShouldFormat(const Rect& page_rect, const Selection&,
                          bool is_selection_active = false) const;
  public: bool ShouldRender() const;

  // text::BufferMutationObserver
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;

  DISALLOW_COPY_AND_ASSIGN(TextRenderer);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_text_text_renderer_h)
