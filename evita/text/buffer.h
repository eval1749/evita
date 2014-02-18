// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_text_buffer_h)
#define INCLUDE_evita_text_buffer_h

#include <unordered_set>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "base/time/time.h"
#include "evita/ed_BinTree.h"
#include "evita/li_util.h"

#include "evita/IStringCursor.h"
#include "evita/ed_BufferCore.h"
#include "evita/ed_Style.h"

namespace text {

class Buffer;
class Interval;
class Mode;
class Range;
class UndoManager;

/// <summary>
// Represents file related properties.
/// </summary>
class FileFeatures {
  protected: NewlineMode m_eNewline;
  protected: bool m_fNoSave;
  protected: base::Time last_write_time_;
  protected: uint m_nCodePage;
  protected: base::string16 filename_;

  // ctor
  protected: FileFeatures()
      : m_eNewline(NewlineMode_Detect),
        m_fNoSave(false),
        m_nCodePage(CP_UTF8) {
  }

  // [G]
  public: uint GetCodePage() const { return m_nCodePage; }
  public: const base::string16& GetFileName() const { return filename_; }
  public: base::Time GetLastWriteTime() const { return last_write_time_; }

  public: NewlineMode GetNewline() const { return m_eNewline; }
  public: bool GetNoSave() const { return m_fNoSave; }

  // [S]
  public: uint SetCodePage(uint const nCodePage) {
    return m_nCodePage = nCodePage;
  }

  public: NewlineMode SetNewline(NewlineMode eNewline) {
    return m_eNewline = eNewline;
  }

  public: bool SetNoSave(bool f) { return m_fNoSave = f; }
};

/// <summary>
/// Represents text buffer.
/// </summary>
class Buffer : public BufferCore, public FileFeatures {
  protected: typedef DoubleLinkedList_<Interval> Intervals;
  protected: typedef BinaryTree<Interval> IntervalTree;

  private: HANDLE m_hObjHeap;
  private: std::unordered_set<Range*> ranges_;
  private: Mode* m_pMode;

  protected: bool m_fUndo;
  protected: UndoManager* m_pUndo;

  private: bool m_fReadOnly;

  // Redisplay
  private: int m_nModfTick;

  // Modified?
  private: int m_nCharTick;
  private: int m_nSaveTick;

  // Interval
  private: Intervals m_oIntervals;
  private: IntervalTree m_oIntervalTree;

  // Asynchronous Operation Support
  public: enum State {
    State_Save,
    State_Ready,
    State_Load,
  };

  protected: State m_eState;
  protected: base::string16 name_;

  // ctor/dtor
  public: Buffer(const base::string16& name, Mode* mode);
  public: virtual ~Buffer();

  public: bool operator==(const Buffer& other) const {
    return this == &other;
  }

  public: bool operator==(const Buffer* other) const {
    return this == other;
  }

  public: bool operator!=(const Buffer& other) const {
    return this != &other;
  }

  public: bool operator!=(const Buffer* other) const {
    return this != other;
  }

  public: const base::string16& name() const { return name_; }

  // [C]
  public: bool CanRedo() const;
  public: bool CanUndo() const;
  public: void ClearUndo();
  public: Posn ComputeEndOfLine(Posn offset) const;
  public: Posn ComputeStartOfLine(Posn offset) const;

  // [D]
  private: void destroyObject(void* pv) { ::HeapFree(m_hObjHeap, 0, pv); }
  public: Count Delete(Posn, Posn);

  // [E]
  public: void EndUndoGroup(const base::string16& name);

  // [G]
  public: const StyleValues* GetDefaultStyle() const;
  public: HANDLE GetHeap() const { return m_hObjHeap; }
  public: Interval* GetIntervalAt(Posn) const;
  public: Mode* GetMode() const { return m_pMode; }
  public: Count GetModfTick() const { return m_nModfTick; }
  public: const base::string16& GetName() const { return name_; }
  public: Posn GetStart() const { return 0; }
  public: const StyleValues* GetStyleAt(Posn) const;
  public: UndoManager* GetUndo() const { return m_pUndo; }

  // [I]
  public: Count IncCharTick(int n) { return m_nCharTick += n; }
  public: Count Insert(Posn, const char16*, Count);
  public: Range* InternalAddRange(Range*);
  public: void InternalDelete(Posn, Posn);
  public: void InternalInsert(Posn, const char16*, Count);
  public: void InternalRemoveRange(Range*);
  public: bool IsModified() const { return m_nCharTick != m_nSaveTick; }
  public: bool IsNotReady() const;
  public: bool IsReadOnly() const { return m_fReadOnly; }

  public: void Insert(Posn lPosn, const char16* pwsz) {
    Insert(lPosn, pwsz, ::lstrlenW(pwsz));
  }

  public: void InsertBefore(Posn position, const base::string16& text);

  // [N]
  public: bool NeedSave () const {
    return !m_fNoSave && IsModified();
  }

  private: Interval* newInterval(Posn, Posn);

  // [R]
  public: Posn Redo(Posn, Count = 1);

  // [S]
  public: void SetFile(const base::string16& filename,
                       base::Time last_write_time);

  public: void SetMode(Mode*);

  public: void SetName(const base::string16& new_name) {
    name_ = new_name;
  }
  public: bool SetReadOnly(bool f) { return m_fReadOnly = f; }
  public: void SetStyle(Posn, Posn, const StyleValues*);
  public: void StartUndoGroup(const base::string16& name);

  // [T]
  private: Interval* tryMergeInterval(Interval*);

  // [U]
  public: Posn Undo(Posn, Count = 1);

  // Buffer management methods
  private: void onChange();
  private: void relocate(Posn, Count);

  // ChangeTracker
  public: class ChangeTracker :  public DoubleLinkedNode_<ChangeTracker> {
    friend class Buffer;

    private: Posn m_lStart;
    private: Posn m_lEnd;

    public: ChangeTracker() { Reset(); }

    public: Posn GetStart() const { return m_lStart; }
    public: Posn GetEnd() const { return m_lEnd; }

    public: void Reset() {
      m_lStart = Posn_Max;
      m_lEnd = 0;
    }
  };

  private: typedef DoubleLinkedList_<ChangeTracker> ChangeTrackers;
  private: ChangeTrackers m_oChangeTrackers;

  public: void RegisterChangeTracker(ChangeTracker* p) {
    m_oChangeTrackers.Append(p);
  }

  public: void UnregisterChangeTracker(ChangeTracker* p) {
    m_oChangeTrackers.Delete(p);
  }

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

    public: const StyleValues* GetStyle() const {
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

    public: const StyleValues* GetStyle() const {
      ASSERT(!AtEnd());
      return m_pBuffer->GetStyleAt(m_lPosn - 1);
    }

    public: void Next() { ASSERT(!AtEnd()); --m_lPosn; }
    public: void Prev() { ++m_lPosn; }
  };

  // EnumInterval
  public: class EnumInterval : public Intervals::Enum {
    public: EnumInterval(const Buffer* pBuffer) :
      Intervals::Enum(&pBuffer->m_oIntervals) {}
  };
};

/// <summary>
/// Utility class for inserting begin/end undo operation. Instances are
/// created on stack instead of heap.
/// </summary>
class UndoBlock {
  private: Buffer* buffer_;
  private: const base::string16& name_;
  public: UndoBlock(Range* range, const base::string16& name);
  public: UndoBlock(Buffer* buffer, const base::string16& name);
  public: ~UndoBlock();

  DISALLOW_COPY_AND_ASSIGN(UndoBlock);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_buffer_h)
