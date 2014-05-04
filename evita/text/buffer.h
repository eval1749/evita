// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_text_buffer_h)
#define INCLUDE_evita_text_buffer_h

#include <memory>

#include "base/basictypes.h"
#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/observer_list.h"
#pragma warning(pop)
#include "base/strings/string16.h"
#include "base/time/time.h"
#include "evita/li_util.h"

#include "evita/IStringCursor.h"
#include "evita/ed_BufferCore.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/marker_set_observer.h"

namespace css {
class Style;
class StyleResolver;
}

namespace text {

class Buffer;
class Interval;
class IntervalSet;
class LineNumberCache;
class Range;
class RangeSet;
class Marker;
class MarkerSet;
class UndoStack;

//////////////////////////////////////////////////////////////////////
//
// LineAndColumn
// This structure contains line number and column number information.
//
struct LineAndColumn {
  int column;
  int line_number;
};

//////////////////////////////////////////////////////////////////////
//
// Buffer
//
class Buffer : public BufferCore,
               public BufferMutationObservee,
               public MarkerSetObserver {

  private: ObserverList<BufferMutationObserver> observers_;
  private: std::unique_ptr<IntervalSet> intervals_;
  private: std::unique_ptr<LineNumberCache> line_number_cache_;
  private: std::unique_ptr<MarkerSet> spelling_markers_;
  private: std::unique_ptr<MarkerSet> syntax_markers_;
  private: std::unique_ptr<RangeSet> ranges_;
  private: std::unique_ptr<css::StyleResolver> style_resolver_;
  private: std::unique_ptr<UndoStack> undo_stack_;

  private: bool m_fReadOnly;

  // Modified?
  private: int m_nCharTick;
  private: int m_nSaveTick;

  // ctor/dtor
  public: Buffer();
  public: virtual ~Buffer();

  public: RangeSet* ranges() const { return ranges_.get(); }
  public: MarkerSet* spelling_markers() const {
    return spelling_markers_.get();
  }
  public: MarkerSet* syntax_markers() const {
    return syntax_markers_.get();
  }

  public: const css::StyleResolver* style_resolver() const {
    return style_resolver_.get();
  }

  // [C]
  public: bool CanRedo() const;
  public: bool CanUndo() const;
  public: void ClearUndo();
  public: Posn ComputeEndOfLine(Posn offset) const;
  public: Posn ComputeStartOfLine(Posn offset) const;

  // [D]
  public: Count Delete(Posn, Posn);

  // [E]
  public: void EndUndoGroup(const base::string16& name);

  // [G]
  public: const css::Style& GetDefaultStyle() const;
  public: Interval* GetIntervalAt(Posn) const;
  public: LineAndColumn GetLineAndColumn(Posn offset) const;
  public: Posn GetStart() const { return 0; }
  public: const css::Style& GetStyleAt(Posn) const;
  public: UndoStack* GetUndo() const { return undo_stack_.get(); }

  // [I]
  public: Count IncCharTick(int n) { return m_nCharTick += n; }
  public: Count Insert(Posn, const char16*, Count);
  public: bool IsModified() const { return m_nCharTick != m_nSaveTick; }
  public: bool IsNotReady() const;
  public: bool IsReadOnly() const { return m_fReadOnly; }

  public: void Insert(Posn lPosn, const char16* pwsz) {
    Insert(lPosn, pwsz, ::lstrlenW(pwsz));
  }

  public: void InsertBefore(Posn position, const base::string16& text);

  // [R]
  public: Posn Redo(Posn, Count = 1);

  // [S]
  public: void SetNotModifiedForTesting() {
    m_nSaveTick = m_nCharTick;
  }
  public: void SetModified(bool new_modifier);
  public: bool SetReadOnly(bool f) { return m_fReadOnly = f; }
  public: void SetStyle(Posn, Posn, const css::Style& style_values);
  public: void StartUndoGroup(const base::string16& name);

  // [U]
  public: Posn Undo(Posn, Count = 1);
  private: void UpdateChangeTick();

  // BufferMutationObservee
  public: virtual void AddObserver(
      BufferMutationObserver* observer) override;
  public: virtual void RemoveObserver(
      BufferMutationObserver* observer) override;

  // MarkerSetObserver
  private: virtual void DidChangeMarker(Posn start, Posn end) override;

  /// <summary>
  /// Buffer character enumerator
  /// </summary>
  public: class EnumChar {
    private: Posn m_lEnd;
    private: Posn m_lPosn;
    private: Buffer* m_pBuffer;

    public: struct Arg {
      Posn m_lEnd;
      Posn m_lPosn;
      Buffer* m_pBuffer;

      Arg(Buffer* pBuffer, Posn lPosn, Posn lEnd)
          : m_lEnd(lEnd),
            m_lPosn(lPosn),
            m_pBuffer(pBuffer) {
      }

      Arg(Buffer* pBuffer, Posn lPosn)
          : m_lEnd(pBuffer->GetEnd()),
            m_lPosn(lPosn),
            m_pBuffer(pBuffer) {
      }
    };

    public: EnumChar(Buffer* pBuffer)
        : m_lEnd(pBuffer->GetEnd()),
          m_lPosn(0),
          m_pBuffer(pBuffer) {
      ASSERT(m_pBuffer->IsValidRange(m_lPosn, m_lEnd));
    }

    public: EnumChar(Arg oArg)
        : m_lEnd(oArg.m_lEnd),
          m_lPosn(oArg.m_lPosn),
          m_pBuffer(oArg.m_pBuffer) {
      ASSERT(m_pBuffer->IsValidRange(m_lPosn, m_lEnd));
    }

    public: EnumChar(const Range*);

    public: bool AtEnd() const { return m_lPosn >= m_lEnd; }

    public: char16 Get() const {
      ASSERT(!AtEnd());
      return m_pBuffer->GetCharAt(m_lPosn);
    }

    public: Posn GetPosn() const { return m_lPosn; }

    public: const css::Style& GetStyle() const {
      ASSERT(!AtEnd());
      return m_pBuffer->GetStyleAt(m_lPosn);
    }

    public: Posn GoTo(Posn lPosn) { return m_lPosn = lPosn; }
    public: void Next() { ASSERT(!AtEnd()); m_lPosn += 1; }
    public: void Prev() { m_lPosn -= 1; }

    public: void SyncEnd() { m_lEnd = m_pBuffer->GetEnd(); }
  };

  /// <summary>
  /// Reverse character enumerator
  /// </summary>
  public: class EnumCharRev {
    private: Posn m_lPosn;
    private: Posn m_lStart;
    private: const Buffer* m_pBuffer;

    public: struct Arg {
      Posn m_lPosn;
      Posn m_lStart;
      Buffer* m_pBuffer;
      Arg(Buffer* pBuffer, Posn lPosn, Posn lStart = 0)
          : m_lPosn(lPosn), m_lStart(lStart), m_pBuffer(pBuffer) {
      }
    };

    public: EnumCharRev(Arg oArg)
        : m_lPosn(oArg.m_lPosn),
          m_lStart(oArg.m_lStart),
          m_pBuffer(oArg.m_pBuffer) {
    }

    public: EnumCharRev(const Range*);

    public: bool AtEnd() const { return m_lPosn <= m_lStart; }

    public: char16 Get() const {
      ASSERT(!AtEnd());
      return m_pBuffer->GetCharAt(m_lPosn - 1);
    }

    public: Posn GetPosn() const { ASSERT(!AtEnd()); return m_lPosn; }

    public: const css::Style& GetStyle() const {
      ASSERT(!AtEnd());
      return m_pBuffer->GetStyleAt(m_lPosn - 1);
    }

    public: void Next() { ASSERT(!AtEnd()); --m_lPosn; }
    public: void Prev() { ++m_lPosn; }
  };

  DISALLOW_COPY_AND_ASSIGN(Buffer);
};

/// <summary>
/// Utility class for inserting begin/end undo operation. Instances are
/// created on stack instead of heap.
/// </summary>
class ScopedUndoGroup {
  private: Buffer* buffer_;
  private: const base::string16 name_;
  public: ScopedUndoGroup(Range* range, const base::string16& name);
  public: ScopedUndoGroup(Buffer* buffer, const base::string16& name);
  public: ~ScopedUndoGroup();

  DISALLOW_COPY_AND_ASSIGN(ScopedUndoGroup);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_buffer_h)
