// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_range_h)
#define INCLUDE_evita_text_range_h

#include "base/strings/string16.h"

namespace text {

class RangeSet;

class Range {
  friend class RangeSet;

  public: struct Information {
      bool m_fLineNum;
      bool m_fColumn;
      Count m_lLineNum;
      Count m_lColumn;
  }; // Information

  private: Posn m_lStart;
  private: Posn m_lEnd;
  private: Buffer* m_pBuffer;

  public: Range(Buffer*, Posn, Posn);
  public: ~Range();

  // [C]
  public: void Collapse(CollapseWhich = Collapse_Start);
  public: Count Copy();

  // [E]
  private: Posn ensurePosn(Posn) const;

  // [F]
  public: Posn FindFirstChar(char16) const;

  // [G]
  public: Buffer* GetBuffer() const { return m_pBuffer; }
  public: Posn GetEnd() const { return m_lEnd; }
  public: void GetInformation(Information*, Count = Count_Max) const;
  public: Posn GetStart() const { return m_lStart; }
  public: base::string16 GetText() const;

  // [P]
  public: void Paste();

  // [S]
  public: Posn SetEnd(Posn);
  public: void SetRange(Posn start, Posn end);
  public: Posn SetStart(Posn);
  public: void SetText(const base::string16& text);

  DISALLOW_COPY_AND_ASSIGN(Range);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_range_h)
