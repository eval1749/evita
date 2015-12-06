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
#include "evita/IStringCursor.h"
#include "evita/text/buffer_core.h"
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
