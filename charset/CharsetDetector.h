//////////////////////////////////////////////////////////////////////////////
//
// Charset Detector
//
// Copyright (C) 1996-2012 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_Buffer.h#9 $
//
#if !defined(INCLUDE_charset_CharsetDetector_h)
#define INCLUDE_charset_CharsetDetector_h

namespace Charset {

// See below link for code pages in Windows:
// http://msdn.microsoft.com/en-us/goglobal/bb964654
class CharsetDetector {
  public: enum Encoding {
    UNKNOWN,
    ASCII = 1 << 0,

    EUC_JP = 1 << 1,
      // Code Set 0 [21..7E]
      // Code Set 1 [A1..FE] [A1..FE]
      // Code Set 2 8E [A1..DF]
      // Code Set 3 8F [A1..FE] [A1..FE]
    ISO_2022_JP = 1 << 2,

    SHIFT_JIS = 1 << 3,
      // ASCII or JIS-Roman [21..7E]
      // Half-Width Katakana [A1..DF]
      // JIS X 208 1997 [81..9F,E0..EF] [40..7E, 80..FC]

    UTF8 = 1 << 4,
      // U+0000..U+007F 0xxxxxxxx
      // U+0080..U+07FF 110xxxxxx 10xxxxxx
      // U+0800..U+FFFF 1110xxxxx 10xxxxxx 10xxxxxx
  };

  private: enum EucState {
    EucJpAscii,
    EucJpFirst,
    EucJp8E,
    EucJp8F,
    EucJp8FSecond,
    EucJpNo,
  } m_eEucState;

  private: enum IsoState {
    IsoAscii,
    IsoEsc,
    IsoEscDollar,
    IsoNo,
  } m_eIsoState;

  private: enum ShiftJisState {
    ShiftJisAscii,
    ShiftJisFirst,
    ShiftJisNo,
  } m_eShiftJisState;

  private: enum Utf8State {
    Utf8_0,  // 0xxxxxxx
    Utf8_2,      // 110xxxxx 10xxxxxx
    Utf8_3,      // 1110xxxx 10xxxxxx 10xxxxxx
    Utf8_3_2,
    Utf8No,
  } m_eUtf8State;

  private: bool m_fClean;
  private: int m_cCandidates;
  private: int m_cPossibles;
  private: int m_rgfCandidate;
  private: int m_rgfPossible;

  // ctor
  public: CharsetDetector();

  // [C]
  private: void CanBe(Encoding);
  private: void CanNotBe(Encoding);
  private: int ComputeCodePage(int);

  // [D]
  public: int Detect(const char* const, const char* const);

  // [F]
  public: int Finish();

  // [I]
  public: bool IsBinary() const;

  DISALLOW_COPY_AND_ASSIGN(CharsetDetector);
}; // CharsetDetector

} // Charset

#endif // !defined(INCLUDE_charset_CharsetDetector_h)
