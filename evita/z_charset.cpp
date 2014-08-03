#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - Editor - Charset
// listener/winapp/z_charset.cpp
//
// Copyright (C) 1996-2011 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_FileIo.cpp#5 $
//

enum {
  CP_EUC_JP      = 51932,
  CP_ISO_2022_JP = 50222,
  CP_SJIS        =   932,
};

class WideCharSink {
  private: wchar_t* m_pwch;
  private: wchar_t* const m_pwchEnd;
  private: wchar_t* const m_pwchStart;

  public: WideCharSink(wchar_t* pwch, int ccwch)
      : m_pwchStart(pwch), m_pwchEnd(pwch + ccwch), m_pwch(pwch) {
  }

  public: uint Add(wchar_t const wch) {
    if (m_pwchStart == nullptr) {
      m_pwch++;
      return 0;
    }

    if (m_pwch >= m_pwchEnd) {
      return ERROR_INSUFFICIENT_BUFFER;
    }

    *m_pwch++ = wch;
    return 0;
  }

  public: uint Add(wchar_t const wch1, wchar_t const wch2) {
    if (m_pwchStart == nullptr) {
      m_pwch += 2;
      return 0;
    }

    if (m_pwch + 2 >= m_pwchEnd) {
      return ERROR_INSUFFICIENT_BUFFER;
    }

    *m_pwch++ = wch1;
    *m_pwch++ = wch2;
    return 0;
  }

  public: uint Add(
      wchar_t const wch1,
      wchar_t const wch2,
      wchar_t const wch3) {
    if (m_pwchStart == nullptr) {
      m_pwch += 3;
      return 0;
    }

    if (m_pwch + 3 >= m_pwchEnd) {
      return ERROR_INSUFFICIENT_BUFFER;
    }

    *m_pwch++ = wch1;
    *m_pwch++ = wch2;
    *m_pwch++ = wch3;
    return 0;
  }

  public: uint Add(
      wchar_t const wch1,
      wchar_t const wch2,
      wchar_t const wch3,
      wchar_t const wch4) {
    if (m_pwchStart == nullptr) {
      m_pwch += 4;
      return 0;
    }

    if (m_pwch + 4 >= m_pwchEnd) {
      return ERROR_INSUFFICIENT_BUFFER;
    }

    *m_pwch++ = wch1;
    *m_pwch++ = wch2;
    *m_pwch++ = wch3;
    *m_pwch++ = wch4;
    return 0;
  }

  public: uint32 GetLength() const {
    return static_cast<uint32>(m_pwch - m_pwchStart);
  }

  DISALLOW_COPY_AND_ASSIGN(WideCharSink);
};

class Converter : public WideCharSink  {
  private: uint const m_dwFlags;

  public: Converter(
      uint const dwFlags,
      wchar_t* const pwch,
      int const cwch)
      : WideCharSink(pwch, cwch),
        m_dwFlags(dwFlags) {
  }

  public: uint Convert(uint const nCodePage, char const ch) {
    char rgch[1];
    rgch[0] = ch;
    return Convert(nCodePage, rgch, 1);
  }

  public: uint Convert(uint const nCodePage, const char* const prgch) {
    char rgch[2];
    if (nCodePage == CP_SJIS) {
      auto const iRow = prgch[0] < 95 ? 112 : 176;
      auto const iCell = prgch[0] & 1 ? (prgch[1] > 95 ? 32 : 31) : 126;
      rgch[0] = static_cast<char>(((prgch[0] + 1) >> 1) + iRow);
      rgch[1] = static_cast<char>(prgch[1] + iCell);

    } else {
      rgch[0] = static_cast<char>(prgch[0] | 0x80);
      rgch[1] = static_cast<char>(prgch[1] | 0x80);
    }

    return Convert(nCodePage, rgch, 2);
  }

  private: uint Convert(
      uint const nCodePage,
      const char* const prgch,
      int const cch) {
    ASSERT(nCodePage != 0);
    ASSERT(prgch != nullptr);
    ASSERT(cch >= 1);
    wchar_t wchChar;
    auto const cwchChar = ::MultiByteToWideChar(
      nCodePage,
      m_dwFlags,
      prgch, cch,
      &wchChar, 1);
    if (cwchChar != 1) {
      return ERROR_NO_UNICODE_TRANSLATION;
    }

    return Add(wchChar);
  }

  DISALLOW_COPY_AND_ASSIGN(Converter);
};

// EUC-JP
//  CS0: 0x21-0x7E                  (ASCII or JIS Roman)
//  CS1: 0xA1..0xFE 0xA1..0xFE      (JIS-X0208-1990)
//  CS2: 0x8E 0xA1..0xFE            (half-width katakana)
//  CS3: 0x8F 0xA1..0xFE 0xA1..0xFE (JIS X0212-1990)
//
// EUC-CN
//  CS0: 0x21-0x7E                  (ASCII or GB Roman)
//  CS1: 0xA1..0xFE 0xA1..0xFE      (GB 2312-80)
//  CS2: 0x8E 0xA1..0xFE            unused
//  CS3: 0x8F 0xA1..0xFE 0xA1..0xFE unused
//
// EUC-KR
//  CS0: 0x21-0x7E                  (ASCII or KS-Roman)
//  CS1: 0xA1..0xFE 0xA1..0xFE      (KS X 1001:1992)
//  CS2: 0x8E 0xA1..0xFE            unused
//  CS3: 0x8F 0xA1..0xFE 0xA1..0xFE unused
//
// EUC-TW
//  CS0: 0x21-0x7E                  (ASCII or CNS Roman)
//  CS1: 0xA1..0xFE 0xA1..0xFE      (CNS 11643-1992 Plane 1)
//  CS2: 0x8E 0xA1..0xFE            (CNS 11643-1992 Plane 1-7)
//  CS3: 0x8F 0xA1..0xFE 0xA1..0xFE unused
static int EucToWideChar(
    UINT dwFlags,
    LPCSTR pchMBCS,
    int* inout_cchMBCS,
    LPWSTR pwchWCS,
    int* inout_cwchWCS ) {
  ASSERT(pchMBCS != nullptr);

  enum {
    EUC_SS2 = 0x8E,
    EUC_SS3 = 0x8F
  };

  enum {
    STATE_EUC_0,
    STATE_EUC_1_2,
    STATE_EUC_2_2,
    STATE_EUC_3_2,
    STATE_EUC_3_3
  } eState = STATE_EUC_0;

  Converter oConverter(dwFlags, pwchWCS, *inout_cwchWCS);
  const char* const pchScanEnd = pchMBCS + *inout_cchMBCS;
  int dwError = 0;
  char rgch[2];
  const char* pchScan;
  for (pchScan = pchMBCS; pchScan < pchScanEnd; pchScan++) {
    unsigned char const ch = *pchScan;
    switch (eState) {
      case STATE_EUC_0:
        if (ch <= 0x7F) {
          dwError = oConverter.Add(ch);
          if (dwError != 0) goto exit;
          break;
        }

        if (ch >= 0xA1 && ch <= 0xFE) {
          rgch[0] = ch & 0x7F;
          eState = STATE_EUC_1_2;
          break;
        }

        if (ch == EUC_SS2) {
          eState = STATE_EUC_2_2;
          break;
        }

        if (ch == EUC_SS3) {
          eState = STATE_EUC_3_2;
          break;
        }

        --pchScan;
        goto errorIllegalChar;

      case STATE_EUC_1_2: {
        if (ch < 0xA1 || ch > 0xFE) {
          --pchScan;
          goto errorIllegalChar;
        }

        rgch[1] = ch & 0x7F;
        dwError = oConverter.Convert(CP_SJIS, rgch);
        if (dwError != 0) goto exit;
        eState = STATE_EUC_0;
        break;
      } // case STATE_EUC_1_2

      case STATE_EUC_2_2:
        // Half-width katakana
        if (ch < 0xA1 || ch > 0xFE) {
          pchScan -= 2;
          goto errorIllegalChar;
        }

        dwError = oConverter.Convert(CP_SJIS, ch);
        if (dwError != 0) goto exit;

        eState = STATE_EUC_0;
        break;

      case STATE_EUC_3_2:
      case STATE_EUC_3_3:
        // Windows doesn't support EUC-JP code set 3.
        goto errorIllegalChar;

      default:
        return ERROR_INVALID_PARAMETER;
    }
  } // while

  if (eState != STATE_EUC_0) {
    goto errorTruncate;
  }

  goto exit;

 errorTruncate:
  dwError = ERROR_NO_UNICODE_TRANSLATION;
  goto exit;

 errorIllegalChar:
  dwError = ERROR_NO_UNICODE_TRANSLATION;
  goto exit;

 exit:
  *inout_cchMBCS = pchScan - pchMBCS;
  *inout_cwchWCS = oConverter.GetLength();
  return dwError;
}

// ISO-2022-1986 = JIS X 0202-1991
// ESC $ (      G0-DESIGNATE MULTIBYTE 94-SET
// ESC $ )      G1-DESIGNATE MULTIBYTE 94-SET
// ESC $ *      G2-DESIGNATE MULTIBYTE 94-SET
// ESC $ +      G3-DESIGNATE MULTIBYTE 94-SET
// ESC $ -      G0-DESIGNATE MULTIBYTE 96-SET
// ESC $ .      G1-DESIGNATE MULTIBYTE 96-SET
// ESC $ /      G2-DESIGNATE MULTIBYTE 96-SET
// ESC (        G0-DESIGNATE 94-SET
// ESC )        G1-DESIGNATE 94-SET
// ESC *        G2-DESIGNATE 94-SET
// ESC +        G3-DESIGNATE 94-SET
// ESC -        G1-DESIGNATE 96-SET
// ESC .        G2-DESIGNATE 96-SET
// ESC /        G3-DESIGNATE 96-SET
//
// Double Byte
// ESC $ ( @    JIS X 0208-1978 (ESC $ @)
// ESC $ ( A    GB2312-1980 (ESC $ A)
// ESC $ ( B    JIS X 0208-Roman (ESC $ B)
// ESC $ ( C    KCS5601-1987
// ESC $ ( D    JIS X 0212-1990
//
// Single Byte
// ESC ( B      ASCII
// ESC ( H      some implementation use this instead of ESC ( J
// ESC ( I      JIS X0202-1984 G1
// ESC ( J      JIS X0201-1976 G0
// ESC . A      ISO-8859-1 G2
// ESC . F      ISO-8859-7 G2 (Greek)
//

// See: ISO-2022 or ECMA-035 document
//  (Character Code Structure and Extension Techniques)
// 4F and 8F are Escape Sqeunce of types nF instead of hexadecimal
//
static int IsoToWideChar(
    DWORD   dwFlags,
    LPCSTR  pchMBCS,
    int*    inout_cchMBCS,
    LPWSTR  pwchWCS,
    int*    inout_cwchWCS) {
  enum {
    ISO2022_SI = 0x0F,        // Ctrl+O
    ISO2022_SO = 0x0E,        // Ctrl+N
    ISO2022_ESC = 0x1B
  };

  enum {
    STATE_SBC,
    STATE_SBC_G1,
    STATE_SBC_SI,
    STATE_SBC_SO,
    STATE_DBC_1,
    STATE_DBC_2,
    STATE_ESC,
    STATE_ESC_4F,
    STATE_ESC_8F,
    STATE_ESC_4F_8F,
  } eState = STATE_SBC;

  Converter oConverter(dwFlags, pwchWCS, *inout_cwchWCS);
  auto const pchScanEnd = pchMBCS + *inout_cchMBCS;
  int nDesignate = 0;
  DWORD dwError = 0;
  char rgch[2];
  const char* pchScan;
  for (pchScan = pchMBCS; pchScan < pchScanEnd; pchScan++) {
    unsigned char const ch = *pchScan;
    switch (eState) {
      case STATE_SBC:
        switch (ch) {
          case ISO2022_ESC:
            eState = STATE_ESC;
            break;

          case ISO2022_SO:
            eState = STATE_SBC_SO;
            break;

          default:
            if (ch <= ' ' || ch >= 0x7F) {
              dwError = oConverter.Add(ch);
            } else {
              dwError = oConverter.Convert(CP_SJIS, ch);
            }
            break;
         } // switch ch
         break;

      case STATE_SBC_G1:
      case STATE_SBC_SI:
      case STATE_SBC_SO:    // FIXME(yosi) 1999-07-22: SBC_SO @
        if (ch == ISO2022_ESC) {
          eState = STATE_ESC;

        } else if (ch == ISO2022_SI && eState == STATE_SBC_SO) {
          eState = STATE_SBC;

        } else {
          // Note: We accept G1 characters. Some programs emit
          // half-width katakana as G1
          dwError = oConverter.Convert(
              CP_SJIS,
              static_cast<char>(ch | 0x80));
        }
        break;

      case STATE_DBC_1:
        if (ch == ISO2022_ESC) {
          eState = STATE_ESC;

        } else if (ch >= 0x21 && ch <= 0x7E) {
          rgch[0] = ch;
          eState = STATE_DBC_2;

        } else {
          dwError = oConverter.Add(ch);
          eState = STATE_SBC;
        }
        break;

      case STATE_DBC_2:
        if (ch < 0x21 || ch > 0x7E) {
           goto errorIllegalChar;
        }

        rgch[1] = ch;
        dwError = oConverter.Convert(CP_SJIS, rgch);
        eState = STATE_DBC_1;
        break;

      case STATE_ESC:
        switch (ch) {
          case '$':  // ESC $
            eState = STATE_ESC_4F;
            break;

          case '(':  // ESC (
            eState = STATE_ESC_8F;
            break;

          case ISO2022_ESC:  // ESC ESC
            dwError = oConverter.Add(ch);
            break;

          default:
            dwError = oConverter.Add(ISO2022_ESC, ch);
            eState = STATE_SBC;
            break;
        }
        break;

      case STATE_ESC_4F:    // ESC $
        switch (ch) {
          case '@':  // ESC $ @ = JIS X 0208-1978
          case 'B':  // ESC $ B = JIS X 0208-1983
            eState = STATE_DBC_1;
            nDesignate = CP_SJIS;
            break;

          case 'A':  // ESC $ A = GB 2312-1980
            eState = STATE_DBC_1;
            nDesignate = 936;   // GB2312 Simplied Chinese GBK
            break;

          case '(':  // ESC $ (
            eState = STATE_ESC_4F_8F;
            break;

          default:
            dwError = oConverter.Add(ISO2022_ESC, '$', ch);
            eState = STATE_SBC;
            break;
        } // switch ch
        break;

      case STATE_ESC_4F_8F:  // ESC $ (
        switch (ch) {
          case '@':  // ESC $ ( @ = JIS X 0208-1978
          case 'B':  // ESC $ ( @ = JIS X 0208-1983
            eState = STATE_DBC_1;
            nDesignate = CP_SJIS; // Shift_JIS
            break;

          case 'A':  // ESC $ ( A = GB 2312-1980
             eState = STATE_DBC_1;
             nDesignate = 936;   // GB2312 Simplied Chinese GBK
             break;

          case 'C':  // ESC $ ( C = KCS5601-1987
             eState = STATE_DBC_1;
             nDesignate = 949;  // Korean
             break;

          default:
            dwError = oConverter.Add(ISO2022_ESC, '$', '(', ch);
            eState = STATE_SBC;
            break;
          } // switch ch
          break;

      case STATE_ESC_8F:    // ESC (
        switch (ch) {
          case 'B': // ESC ( B = ASCII
          case 'J': // ESC ( J = JIS X201-1976:G0
            eState = STATE_SBC;
            nDesignate = CP_SJIS;
            break;

          case 'I':  // ESC ( I = JIS X202-1984:G1
            eState = STATE_SBC_G1;
            nDesignate = CP_SJIS;
            break;

          default:
            dwError = oConverter.Add(ISO2022_ESC, '(', ch);
            eState = STATE_SBC;
            break;
          } // switch ch
          break;

      default:
        NOTREACHED();
      } // switch eState

      if (dwError != 0) {
        break;
      }
    } // for

    switch (eState) {
      case STATE_DBC_2:
      case STATE_ESC_4F:
      case STATE_ESC_8F:
      case STATE_ESC_4F_8F:
        goto errorTruncate;
    } // switch eState

    goto exit;

 errorTruncate:
   dwError = ERROR_NO_UNICODE_TRANSLATION;
   goto exit;

 errorIllegalChar:
   dwError = ERROR_NO_UNICODE_TRANSLATION;
   goto exit;

 exit:
  *inout_cchMBCS = pchScan - pchMBCS;
  *inout_cwchWCS = oConverter.GetLength();

  return dwError;
}

int MyMultiByteToWideChar(
    __in   UINT nCodePage,
    __in   DWORD dwFlags,
    __in   LPCSTR lpMultiByteStr,
    __in   int cbMultiByte,
    __out  LPWSTR lpWideCharStr,
    __in   int cchWideChar) {

  if (cbMultiByte == -1) {
    cbMultiByte = ::lstrlenA(lpMultiByteStr);
  } else if (cbMultiByte < 0) {
    ::SetLastError(ERROR_INVALID_PARAMETER);
    return -1;
  }

  if (lpWideCharStr == nullptr && cchWideChar != 0) {
    ::SetLastError(ERROR_INVALID_PARAMETER);
    return -1;
  }

  switch (nCodePage) {
    case 0: {
      if (cbMultiByte > cchWideChar) {
        ::SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return -1;
      }

      auto const pchEnd = lpMultiByteStr + cbMultiByte;
      auto pwch = lpWideCharStr;
      for (auto pch = lpMultiByteStr; pch < pchEnd; pch++) {
        *pwch++ = *pch;
      }
      return cbMultiByte;
    }

    case CP_ISO_2022_JP: {
      auto const nError = IsoToWideChar(
          dwFlags,
          lpMultiByteStr,
          &cbMultiByte,
          lpWideCharStr,
          &cchWideChar);
      if (nError != 0) {
        ::SetLastError(nError);
        return -1;
      }
      return cchWideChar;
    }

    case CP_EUC_JP: {
      auto const nError = EucToWideChar(
          dwFlags,
          lpMultiByteStr,
          &cbMultiByte,
          lpWideCharStr,
          &cchWideChar);
      if (nError != 0) {
        ::SetLastError(nError);
        return -1;
      }

      return cchWideChar;
    }
  } // switch nCodePage

  return ::MultiByteToWideChar(
    nCodePage,
    dwFlags,
    lpMultiByteStr,
    cbMultiByte,
    lpWideCharStr,
    cchWideChar);
}
