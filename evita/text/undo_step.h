// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_undo_step_h)
#define INCLUDE_evita_text_undo_step_h

#include "base/basictypes.h"
#include "common/castable.h"

namespace text {

class UndoStack;

//////////////////////////////////////////////////////////////////////
//
// UndoStep
//
class UndoStep : common::Castable {
  DECLARE_CASTABLE_CLASS(UndoStep, Castable);

  friend class UndoStack;

  public: void* operator new(size_t size, UndoStack* undo_stack);
  private: void operator delete(void* undo_step);

  protected: UndoStep* m_pNext;
  protected: UndoStep* m_pPrev;

  protected: UndoStep();
  public: virtual ~UndoStep();

  public: virtual void Discard(UndoStack*);
  public: virtual Posn GetAfterRedo() const = 0;
  public: virtual Posn GetAfterUndo() const = 0;
  public: virtual Posn GetBeforeRedo() const = 0;
  public: virtual Posn GetBeforeUndo() const = 0;
  public: virtual void Redo(Buffer*);
  public: virtual void Undo(Buffer*) = 0;

  DISALLOW_COPY_AND_ASSIGN(UndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// TextUndoStep
//
class TextUndoStep : public UndoStep {
  DECLARE_CASTABLE_CLASS(TextUndoStep, UndoStep);

  protected: Posn m_lEnd;
  protected: Posn m_lStart;

  protected: TextUndoStep(Posn lStart, Posn lEnd);
  public: virtual ~TextUndoStep();

  DISALLOW_COPY_AND_ASSIGN(TextUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// BeginUndoStep
//
class BeginUndoStep : public UndoStep {
  DECLARE_CASTABLE_CLASS(BeginUndoStep, UndoStep);

  private: const base::string16 name_;

  public: BeginUndoStep(const base::string16& name);
  public: virtual ~BeginUndoStep();

  // UndoStep
  private: virtual Posn GetAfterRedo() const override;
  private: virtual Posn GetAfterUndo() const override;
  private: virtual Posn GetBeforeRedo() const override;
  private: virtual Posn GetBeforeUndo() const override;
  private: virtual void Undo(Buffer* pBuffer) override;

  DISALLOW_COPY_AND_ASSIGN(BeginUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// DeleteUndoStep
//
class DeleteUndoStep : public TextUndoStep {
  DECLARE_CASTABLE_CLASS(DeleteUndoStep, TextUndoStep);

  private: class Chars;
  private: class EnumChars;

  private: Chars* m_pFirst;
  private: Chars* m_pLast;

  public: DeleteUndoStep(UndoStack* pUndo, Posn lStart, Posn lEnd);
  public: virtual ~DeleteUndoStep();

  private: Chars* createChars(UndoStack* pUndo, Posn lStart, Posn lEnd);
  private: void insertChars(Buffer* pBuffer) const;
  // Merges new delete information into the last delete UndoStep if
  // o New delete doesn't start with newline.
  // o New delete doesn't end with newline.
  public: bool Merge(UndoStack* pUndo, Posn lStart, Posn lEnd);

  // Recrod
  private: virtual void Discard(UndoStack* pUndo) override;
  private: virtual Posn GetAfterRedo() const override;
  private: virtual Posn GetAfterUndo() const override;
  private: virtual Posn GetBeforeRedo() const override;
  private: virtual Posn GetBeforeUndo() const override;
  private: virtual void Redo(Buffer* pBuffer) override;
  private: virtual void Undo(Buffer* pBuffer) override;

  DISALLOW_COPY_AND_ASSIGN(DeleteUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// EndUndoStep
//
class EndUndoStep : public UndoStep {
  DECLARE_CASTABLE_CLASS(EndUndoStep, UndoStep);

  private: const base::string16 name_;

  public: EndUndoStep(const base::string16& name);
  public: virtual ~EndUndoStep();

  public: bool CanMerge(const base::string16& name) const;

  private: virtual Posn GetAfterRedo() const override;
  private: virtual Posn GetAfterUndo() const override;
  private: virtual Posn GetBeforeRedo() const override;
  private: virtual Posn GetBeforeUndo() const override;
  private: virtual void Undo(Buffer* pBuffer) override;

  DISALLOW_COPY_AND_ASSIGN(EndUndoStep);
};

//////////////////////////////////////////////////////////////////////
//
// InsertUndoStep
//
class InsertUndoStep : public TextUndoStep {
  DECLARE_CASTABLE_CLASS(InsertUndoStep, TextUndoStep);

  public: InsertUndoStep(Posn lStart, Posn lEnd);
  public: virtual ~InsertUndoStep();

  // Merge
  // Merge "Insert" UndoStep if
  // o [last][new]
  // o [new][last]
  // and there is no newline between last and new.
  public: bool Merge(Buffer* pBuffer, Posn lStart, Posn lEnd);

  // UndoStep
  private: virtual Posn GetAfterRedo() const override;
  private: virtual Posn GetAfterUndo() const override;
  private: virtual Posn GetBeforeRedo() const override;
  private: virtual Posn GetBeforeUndo() const override;
  private: virtual void Redo(Buffer* pBuffer) override;;
  private: virtual void Undo(Buffer* pBuffer) override;;

  DISALLOW_COPY_AND_ASSIGN(InsertUndoStep);
};


}  // namespace text

#endif // !defined(INCLUDE_evita_text_undo_step_h)
