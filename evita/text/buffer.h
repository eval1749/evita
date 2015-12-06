// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_TEXT_BUFFER_H_
#define EVITA_TEXT_BUFFER_H_

#include <memory>

#include "base/logging.h"
#include "base/basictypes.h"
#include "base/observer_list.h"
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
 public:
  // ctor/dtor
  Buffer();
  virtual ~Buffer();

  RangeSet* ranges() const { return ranges_.get(); }
  MarkerSet* spelling_markers() const { return spelling_markers_.get(); }
  MarkerSet* syntax_markers() const { return syntax_markers_.get(); }

  const css::StyleResolver* style_resolver() const {
    return style_resolver_.get();
  }

  // [C]
  bool CanRedo() const;
  bool CanUndo() const;
  void ClearUndo();
  Posn ComputeEndOfLine(Posn offset) const;
  Posn ComputeStartOfLine(Posn offset) const;

  // [D]
  Count Delete(Posn, Posn);

  // [E]
  void EndUndoGroup(const base::string16& name);

  // [G]
  const css::Style& GetDefaultStyle() const;
  Interval* GetIntervalAt(Posn) const;
  LineAndColumn GetLineAndColumn(Posn offset) const;
  Posn GetStart() const { return 0; }
  const css::Style& GetStyleAt(Posn) const;
  UndoStack* GetUndo() const { return undo_stack_.get(); }

  // [I]
  Count IncCharTick(int n) { return m_nCharTick += n; }
  Count Insert(Posn, const base::char16*, Count);
  bool IsModified() const { return m_nCharTick != m_nSaveTick; }
  bool IsNotReady() const;
  bool IsReadOnly() const { return m_fReadOnly; }

  void Insert(Posn lPosn, const base::char16* pwsz) {
    Insert(lPosn, pwsz, ::lstrlenW(pwsz));
  }

  void InsertBefore(Posn position, const base::string16& text);

  // [R]
  Posn Redo(Posn, Count = 1);

  // [S]
  void SetNotModifiedForTesting() { m_nSaveTick = m_nCharTick; }
  void SetModified(bool new_modifier);
  bool SetReadOnly(bool f) { return m_fReadOnly = f; }
  void SetStyle(Posn, Posn, const css::Style& style_values);
  void StartUndoGroup(const base::string16& name);

  // [U]
  Posn Undo(Posn, Count = 1);

  // BufferMutationObservee
  void AddObserver(BufferMutationObserver* observer) final;
  void RemoveObserver(BufferMutationObserver* observer) final;

  /// <summary>
  /// Buffer character enumerator
  /// </summary>
  class EnumChar final {
   public:
    struct Arg final {
      Posn m_lEnd;
      Posn m_lPosn;
      Buffer* m_pBuffer;

      Arg(Buffer* pBuffer, Posn lPosn, Posn lEnd)
          : m_lEnd(lEnd), m_lPosn(lPosn), m_pBuffer(pBuffer) {}

      Arg(Buffer* pBuffer, Posn lPosn)
          : m_lEnd(pBuffer->GetEnd()), m_lPosn(lPosn), m_pBuffer(pBuffer) {}
    };

    explicit EnumChar(Buffer* pBuffer)
        : m_lEnd(pBuffer->GetEnd()), m_lPosn(0), m_pBuffer(pBuffer) {
      DCHECK(m_pBuffer->IsValidRange(m_lPosn, m_lEnd));
    }

    explicit EnumChar(Arg oArg)
        : m_lEnd(oArg.m_lEnd),
          m_lPosn(oArg.m_lPosn),
          m_pBuffer(oArg.m_pBuffer) {
      DCHECK(m_pBuffer->IsValidRange(m_lPosn, m_lEnd));
    }

    explicit EnumChar(const Range*);

    bool AtEnd() const { return m_lPosn >= m_lEnd; }

    base::char16 Get() const {
      DCHECK(!AtEnd());
      return m_pBuffer->GetCharAt(m_lPosn);
    }

    Posn GetPosn() const { return m_lPosn; }

    const css::Style& GetStyle() const {
      DCHECK(!AtEnd());
      return m_pBuffer->GetStyleAt(m_lPosn);
    }

    Posn GoTo(Posn lPosn) { return m_lPosn = lPosn; }
    void Next() {
      DCHECK(!AtEnd());
      m_lPosn += 1;
    }
    void Prev() { m_lPosn -= 1; }

    void SyncEnd() { m_lEnd = m_pBuffer->GetEnd(); }

   private:
    Posn m_lEnd;
    Posn m_lPosn;
    Buffer* m_pBuffer;

    DISALLOW_COPY_AND_ASSIGN(EnumChar);
  };

  /// <summary>
  /// Reverse character enumerator
  /// </summary>
  class EnumCharRev final {
   public:
    struct Arg {
      Posn m_lPosn;
      Posn m_lStart;
      Buffer* m_pBuffer;
      Arg(Buffer* pBuffer, Posn lPosn, Posn lStart = 0)
          : m_lPosn(lPosn), m_lStart(lStart), m_pBuffer(pBuffer) {}
    };

    explicit EnumCharRev(Arg oArg)
        : m_lPosn(oArg.m_lPosn),
          m_lStart(oArg.m_lStart),
          m_pBuffer(oArg.m_pBuffer) {}

    explicit EnumCharRev(const Range*);

    bool AtEnd() const { return m_lPosn <= m_lStart; }

    base::char16 Get() const {
      DCHECK(!AtEnd());
      return m_pBuffer->GetCharAt(m_lPosn - 1);
    }

    Posn GetPosn() const {
      DCHECK(!AtEnd());
      return m_lPosn;
    }

    const css::Style& GetStyle() const {
      DCHECK(!AtEnd());
      return m_pBuffer->GetStyleAt(m_lPosn - 1);
    }

    void Next() {
      DCHECK(!AtEnd());
      --m_lPosn;
    }
    void Prev() { ++m_lPosn; }

   private:
    Posn m_lPosn;
    Posn m_lStart;
    const Buffer* m_pBuffer;

    DISALLOW_COPY_AND_ASSIGN(EnumCharRev);
  };

 private:
  void UpdateChangeTick();

  base::ObserverList<BufferMutationObserver> observers_;
  std::unique_ptr<IntervalSet> intervals_;
  std::unique_ptr<LineNumberCache> line_number_cache_;
  std::unique_ptr<MarkerSet> spelling_markers_;
  std::unique_ptr<MarkerSet> syntax_markers_;
  std::unique_ptr<RangeSet> ranges_;
  std::unique_ptr<css::StyleResolver> style_resolver_;
  std::unique_ptr<UndoStack> undo_stack_;

  // MarkerSetObserver
  void DidChangeMarker(Posn start, Posn end) override;

  int m_nCharTick;
  int m_nSaveTick;
  bool m_fReadOnly;

  DISALLOW_COPY_AND_ASSIGN(Buffer);
};

}  // namespace text

#endif  // EVITA_TEXT_BUFFER_H_
