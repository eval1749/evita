// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
class Offset;
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
  int revision() const { return revision_; }
  MarkerSet* spelling_markers() const { return spelling_markers_.get(); }
  MarkerSet* syntax_markers() const { return syntax_markers_.get(); }

  const css::StyleResolver* style_resolver() const {
    return style_resolver_.get();
  }

  // [C]
  bool CanRedo() const;
  bool CanUndo() const;
  void ClearUndo();
  Offset ComputeEndOfLine(Offset offset) const;
  Offset ComputeStartOfLine(Offset offset) const;

  // [D]
  int Delete(Offset, Offset);

  // [E]
  void EndUndoGroup(const base::string16& name);

  // [G]
  const css::Style& GetDefaultStyle() const;
  Interval* GetIntervalAt(Offset) const;
  LineAndColumn GetLineAndColumn(Offset offset) const;
  Offset GetStart() const;
  const css::Style& GetStyleAt(Offset) const;
  UndoStack* GetUndo() const { return undo_stack_.get(); }

  // [I]
  int IncCharTick(int n) { return revision_ += n; }
  int Insert(Offset, const base::char16*, int);
  bool IsReadOnly() const { return m_fReadOnly; }

  void Insert(Offset lOffset, const base::char16* pwsz) {
    Insert(lOffset, pwsz, ::lstrlenW(pwsz));
  }

  void InsertBefore(Offset position, const base::string16& text);

  // Does redo last undo operation if it starts at |offset| and returns
  // |offset|, otherwise returns starting offset of the last undo operation.
  Offset Redo(Offset offset);

  // [S]
  bool SetReadOnly(bool f) { return m_fReadOnly = f; }
  void SetStyle(Offset, Offset, const css::Style& style_values);
  void StartUndoGroup(const base::string16& name);

  // Does undo last modification if it starts at |offset| and returns
  // |offset|, otherwise returns starting offset of the last modification.
  Offset Undo(Offset offset);

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
  void DidChangeMarker(Offset start, Offset end) override;

  // |revision_| holds buffer revision, which incremented by one at each
  // deletion and insertion. Undo operation restores resets buffer revision
  // at revision before roll backed operation executed.
  int revision_ = 0;
  bool m_fReadOnly;

  DISALLOW_COPY_AND_ASSIGN(Buffer);
};

}  // namespace text

#endif  // EVITA_TEXT_BUFFER_H_
