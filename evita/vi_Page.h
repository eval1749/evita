//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Page object
// listener/winapp/vi_page.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Page.h#1 $
//
#if !defined(INCLUDE_listener_winapp_visual_formatter_h)
#define INCLUDE_listener_winapp_visual_formatter_h

#include "./vi_defs.h"
#include "./vi_style.h"
#include "gfx_base.h"

class Font;
class Selection;
class Style;

namespace dom {
class Buffer;
}

using Buffer = dom::Buffer;

namespace PageInternal {
  class Cell;
  class DisplayBuffer;
  class Formatter;
}; // PageInternal

//////////////////////////////////////////////////////////////////////
//
// Page
//
class Page {
  friend class PageInternal::Formatter;

  private: typedef PageInternal::Cell Cell;
  private: class DisplayBuffer;
  public: class Line;

  private: class DisplayBuffer {
    private: typedef DoubleLinkedList_<Line, DisplayBuffer> Lines;
    private: float m_cy;
    private: HANDLE m_hObjHeap;
    private: Lines lines_;
    private: gfx::RectF rect_;

    public: DisplayBuffer();
    public: DisplayBuffer(const DisplayBuffer&);
    public: ~DisplayBuffer();

    public: float bottom() const { return rect_.bottom; }
    public: float height() const { return rect_.height(); }
    public: float left() const { return rect_.left; }
    public: const Lines& lines() const { return lines_; }
    public: const gfx::RectF& rect() const { return rect_; }
    public: float right() const { return rect_.right; }
    public: float top() const { return rect_.top; }
    public: float width() const { return rect_.width(); }

    public: void Append(Line*);
    public: void* Alloc(size_t);
    public: Line* GetFirst() const { return lines_.GetFirst(); }
    public: HANDLE GetHeap() const { return m_hObjHeap; }
    public: float GetHeight() const { return m_cy; }
    public: Line* GetLast() const { return lines_.GetLast(); }
    public: Line* NewLine();
    public: void Prepend(Line*);
    public: HANDLE Reset(const gfx::RectF& page_rect);
    public: Line* ScrollDown();
    public: Line* ScrollUp();
  };

  // Line
  public: class Line : public DoubleLinkedNode_<Line, DisplayBuffer>,
                       public ObjectInHeap {
    private: typedef DoubleLinkedList_<Cell, Line> Cells;
    friend class Page;
    friend class DisplayBuffer;
    friend class PageInternal::Formatter;

    private: Cells cells_;
    private: uint m_cwch;
    private: mutable uint m_nHash;
    private: HANDLE m_hObjHeap;
    private: float m_iHeight;
    private: float m_iWidth;
    private: Posn m_lStart;
    private: Posn m_lEnd;
    private: char16* m_pwch;

    public: explicit Line(HANDLE hHeap);
    private: Line(const Line& other, HANDLE hHeap);
    private: ~Line() {}

    public: const Cells& cells() const { return cells_; }
    public: Cells& cells() { return cells_; }

    public: void Discard();
    public: Line* Copy(HANDLE hHeap) const;
    public: bool Equal(const Line*) const;
    public: void Fix(float dscent);
    public: Cell* GetCell() const { return cells_.GetFirst(); }
    public: Posn GetEnd() const { return m_lEnd; }
    public: float GetHeight() const { return m_iHeight; }
    public: Posn GetStart() const { return m_lStart; }
    public: float GetWidth() const { return m_iWidth; }
    public: uint Hash() const;
    public: Posn MapXToPosn(const gfx::Graphics&, float) const;
    public: void Render(const gfx::Graphics&,
                         const gfx::PointF& left_top) const;
    public: void Reset();
  };

  // Buffer
  public: text::Buffer* m_pBuffer;
  private: Count m_nModfTick;

  // Selection
  public: Posn m_lSelStart;;
  public: Posn m_lSelEnd;
  public: Color m_crSelFg;
  public: Color m_crSelBg;

  public: Color m_crBackground;

  private: Posn m_lStart;
  private: Posn m_lEnd;

  private: DisplayBuffer m_oFormatBuf;
  private: DisplayBuffer m_oScreenBuf;

  public: Page();
  public: ~Page();

  // [A]
  private: void allocHeap();

  // [F]
  private: void fillBottom(const gfx::Graphics&, float top) const;
  private: void fillRight(const gfx::Graphics&, const Line*, float) const;
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
  public: bool IsDirty(const Rect& page_rect, const Selection&,
                       bool is_selection_active = false) const;
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
};

#endif //!defined(INCLUDE_listener_winapp_visual_formatter_h)
