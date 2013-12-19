#include "precomp.h"
// Charset Detector
// Copyright (C) 1996-2011 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_FileIo.cpp#5 $
//
#include "./CharsetDetector.h"

#include "./Charset.h"

namespace Charset {

CharsetDetector::CharsetDetector()
    : m_cCandidates(4),
      m_cPossibles(0),
      m_eIsoState(IsoAscii),
      m_eEucState(EucJpAscii),
      m_eShiftJisState(ShiftJisAscii),
      m_eUtf8State(Utf8_0),
      m_fClean(true),
      m_rgfCandidate(EUC_JP | ISO_2022_JP | SHIFT_JIS | UTF8),
      m_rgfPossible(0) {
}

void CharsetDetector::CanBe(Encoding const eEncoding) {
  if ((m_rgfPossible & eEncoding) == 0) {
    m_rgfPossible |= eEncoding;
    m_cPossibles++;
  }
}

void CharsetDetector::CanNotBe(Encoding const eEncoding) {
  if ((m_rgfCandidate & eEncoding) != 0) {
    m_rgfCandidate &= ~eEncoding;
    m_cCandidates--;
  }

  if ((m_rgfPossible & eEncoding) != 0) {
    m_rgfPossible &= ~eEncoding;
    m_cPossibles--;
  }
}

int CharsetDetector::ComputeCodePage(int const rgfMask) {
  switch (rgfMask) {
    case EUC_JP:
      return CP_EUC_JP;

    case ISO_2022_JP:
      // Note: 50222 "Japanese (JIS-Allow 1 byte Kana - SO/SI)
      return CP_ISO_2022_JP;

    case SHIFT_JIS:
      return CP_SHIFT_JIS;

    case UTF8:
      return CP_UTF_8;

    default:
      return 0;
  }
}

int CharsetDetector::Detect(
    const char* const pchStart,
    const char* const pchEnd) {
  ASSERT(pchStart <= pchEnd);

  if (m_cCandidates == 0) {
    return 0;
  }

  for (auto pch = pchStart; pch < pchEnd; pch++) {
    auto const ch = *pch & 0xFF;

    if (ch == 0 || ch == 0xFF) {
      // This should be binary stream rather than chracter stream.
      m_cCandidates = 0;
      return 0;
    }

    if (m_fClean) {
      if (ch >= 0x80) {
        m_fClean = false;
      }
    }

    switch (m_eEucState) {
      case EucJpNo:
          break;

      case EucJpAscii:
        if (ch <= 0x7E) {
          m_eEucState =  EucJpAscii;
        } else if (ch >= 0xA1 && ch <= 0xFE) {
          m_eEucState = EucJpFirst;
        } else if (ch == 0x8E) {
          m_eEucState = EucJp8E;
        } else if (ch == 0x8F) {
          m_eEucState = EucJp8F;
        } else {
          CanNotBe(EUC_JP);
          m_eEucState = EucJpNo;
        }
        break;

      case EucJpFirst:
        if (ch >= 0xA1 && ch <= 0xFE) {
          CanBe(EUC_JP);
          m_eEucState = EucJpAscii;
        } else {
          CanNotBe(EUC_JP);
          m_eEucState = EucJpNo;
        }
        break;

      case EucJp8E:
        if (ch >= 0xA1 && ch <= 0xDF) {
          CanBe(EUC_JP);
          m_eEucState = EucJpAscii;
        } else {
          CanNotBe(EUC_JP);
          m_eEucState = EucJpNo;
        }
        break;

      case EucJp8F:
        if (ch >= 0xA1 && ch <= 0xFE) {
          m_eEucState = EucJp8FSecond;
        } else {
          CanNotBe(EUC_JP);
          m_eEucState = EucJpNo;
        }
        break;

      case EucJp8FSecond:
        if (ch >= 0xA1 && ch <= 0xFE) {
          CanBe(EUC_JP);
          m_eEucState = EucJpAscii;
        } else {
          CanNotBe(EUC_JP);
          m_eEucState = EucJpNo;
        }
        break;

      default:
        CAN_NOT_HAPPEN();
    } // swtich m_eEucState

    switch (m_eIsoState) {
      case IsoNo:
        break;

      case IsoAscii:
        if (ch == 0x1B) {
          m_eIsoState = IsoEsc;

        } else if (ch >= 0x80) {
          CanNotBe(ISO_2022_JP);
          m_eIsoState= IsoNo;
        }
        break;

      case IsoEsc:
        switch (ch) {
        case '$':
          m_eIsoState = IsoEscDollar;
          break;

        case 0x1B: // ESC
          break;

        default:
          if (ch < 0x80) {
            m_eIsoState = IsoAscii;
          } else {
            CanNotBe(ISO_2022_JP);
            m_eIsoState= IsoNo;
          }
          break;
        }
        break;

      case IsoEscDollar:
        switch (ch) {
          case 'B':  // ESC $ B -- JIS X 0208-1983
          case '@':  // ESC $ @ -- JIS C 6226-1978
            CanBe(ISO_2022_JP);
            m_eIsoState = IsoAscii;
            break;

          default:
            CanNotBe(ISO_2022_JP);
            m_eIsoState = IsoNo;
            break;
        }
        break;

      default:
        CAN_NOT_HAPPEN();
    } // switch m_eIsoState

    switch (m_eShiftJisState) {
      case ShiftJisNo:
        break;

      case ShiftJisAscii:
        if (ch <= 0x7E) {
          m_eShiftJisState = ShiftJisAscii;

        } else if (ch >= 0xA1 && ch <= 0xDF) {
          CanBe(SHIFT_JIS);

        } else if (ch >= 0x81 && ch <= 0x9F) {
          m_eShiftJisState = ShiftJisFirst;

        } else if (ch >= 0xE0 && ch <= 0xEF) {
          m_eShiftJisState = ShiftJisFirst;

        } else {
          CanNotBe(SHIFT_JIS);
        }
        break;

      case ShiftJisFirst:
        if (ch <= 0x7E) {
          CanBe(SHIFT_JIS);

        } else if (ch >= 0x80 && ch <= 0xFC) {
          CanBe(SHIFT_JIS);

        } else {
          CanNotBe(SHIFT_JIS);
          m_eShiftJisState = ShiftJisNo;
        }
        break;

      default:
        CAN_NOT_HAPPEN();
    } // switch m_eShiftJis

    switch (m_eUtf8State) {
      case Utf8No:
        break;

      case Utf8_0:
        if (ch <= 0x7E) {
          m_eUtf8State = Utf8_0;
        } else if (ch >= 0xC0 && ch <= 0xDF) {
          m_eUtf8State = Utf8_2;
        } else if (ch >= 0xE0 && ch <= 0xEF) {
          m_eUtf8State = Utf8_3;
        } else {
          CanNotBe(UTF8);
          m_eUtf8State = Utf8No;
        }
        break;

      case Utf8_2:
        if (ch >= 0x80 && ch <= 0xBF) {
          CanBe(UTF8);
          m_eUtf8State = Utf8_0;
        } else {
          CanNotBe(UTF8);
          m_eUtf8State = Utf8No;
        }
        break;

      case Utf8_3:
        if (ch >= 0x80 && ch <= 0xBF) {
          m_eUtf8State = Utf8_3_2;
        } else {
          CanNotBe(UTF8);
          m_eUtf8State = Utf8No;
        }
        break;

      case Utf8_3_2:
        if (ch >= 0x80 && ch <= 0xBF) {
          CanBe(UTF8);
          m_eUtf8State = Utf8_0;
        } else {
          CanNotBe(UTF8);
          m_eUtf8State = Utf8No;
        }
        break;

      default:
        CAN_NOT_HAPPEN();
    } // swtich m_eUtf8State

    if (m_cCandidates == 0) {
      return 0;
    }
  } // for pch

  if (m_cCandidates == 1 && m_rgfCandidate == m_rgfPossible) {
    ASSERT(m_cPossibles == 1);
    return ComputeCodePage(m_rgfCandidate);
  }

  return 0;
} // Detect

int CharsetDetector::Finish() {
  if (m_cCandidates == 0) {
    return 0;
  }

  if (m_cPossibles == 1) {
    return ComputeCodePage(m_rgfPossible);
  }

  if (m_fClean) {
    return 0;
  }

  if (m_rgfPossible & UTF8) {
    return CP_UTF8;
  }

  if (m_rgfPossible & SHIFT_JIS) {
    return 932;
  }

  if (m_rgfPossible & EUC_JP) {
    return 50932;
  }

  if (m_rgfPossible & ISO_2022_JP) {
    return 50222;
  }

  return 0;
}

bool CharsetDetector::IsBinary() const {
  return m_cCandidates == 0;
}

} // Charset
