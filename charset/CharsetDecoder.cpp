// Charset Decoder
// Copyright (C) 1996-2011 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_FileIo.cpp#5 $
//
#include "./CharsetDecoder.h"

namespace Charset {

namespace {

template<typename T> class ScopedArray_ {
  private: T* const array_;
  public: ScopedArray_(int const size) 
    : array_(new T[static_cast<size_t>(size)]) {
  }
  public: ~ScopedArray_() { delete[] array_; }
  public: operator T*() const { return array_; }
  DISALLOW_COPY_AND_ASSIGN(ScopedArray_);
};

// Double Byte Charset decoder.
// Simply call MBCS2WCS.
class DecodeHelper {
  protected: char bytes_[4];
  private: CharsetDecoder::Callback* const callback_;
  protected: CodePage const code_page_;

  protected: DecodeHelper::DecodeHelper(
      CodePage const code_page,
      CharsetDecoder::Callback* const callback)
      : callback_(callback),
        code_page_(code_page) {
  }

  protected: bool BulkDecode(
      const char* const start,
      const char* const end) {
    ASSERT(start != nullptr);
    ASSERT(start <= end);

    auto const cch = static_cast<uint>(end - start);
    ScopedArray_<char16> buf(static_cast<int>(cch));

    auto const cwch = ::MultiByteToWideChar(
        code_page_,
        MB_ERR_INVALID_CHARS,
        start,
        static_cast<int>(cch),
        buf,
        static_cast<int>(cch));

    if (cch == static_cast<uint>(cwch)) {
      callback_->DecoderOutput(buf, buf + cwch);
      return true;
    }

    return false;
  }

  protected: static void ConvertJisToShiftJis(char* in) {
    uint8* buf = reinterpret_cast<uint8*>(in);
    auto const iRow = buf[0] < 95 ? 112 : 176;
    auto const iCell = buf[0] & 1 ? (buf[1] > 95 ? 32 : 31) : 126;
    buf[0] = static_cast<uint8>(((buf[0] + 1) >> 1) + iRow);
    buf[1] = static_cast<uint8>(buf[1] + iCell);
  }

  protected: void Decode(int const buflen) {
    ASSERT(buflen == 1 || buflen == 2);
    char16 buf[2];
    auto const cwch = ::MultiByteToWideChar(
      code_page_,
      MB_ERR_INVALID_CHARS,
      bytes_,
      buflen,
      buf,
      1);
    if (cwch == 1) {
      callback_->DecoderOutput(buf, buf + 1);
    } else {
      buf[0] = static_cast<char16>(bytes_[0]);
      buf[1] = static_cast<char16>(bytes_[1]);
      callback_->DecoderOutput(buf, buf + buflen);
    }
  }

  protected: void SendBytes(const char* const start, const char* const end) {
    if (start == nullptr) return;
    ASSERT(start <= end);
    auto const len = static_cast<int>(end - start);
    if (len == 0) return;
    ScopedArray_<char16> chars(len);
    char16* dest = chars;
    for (auto p = start; p < end; p++) {
      *dest++ = static_cast<uint8>(*p);
    }
    callback_->DecoderOutput(chars, chars + len);
  }

  protected: void SendChar(char16 wch) {
    callback_->DecoderOutput(&wch, &wch + 1);
  }

  DISALLOW_COPY_AND_ASSIGN(DecodeHelper);
}; // DecodeHelper


// Double Byte Charset decoder.
// Simply call MBCS2WCS.
class DbcsDecoder : public CharsetDecoder, public DecodeHelper {
  private: enum State {
    StateAscii,
    StateLeadByte,
  };

  private: State state_;

  public: DbcsDecoder::DbcsDecoder(
      CodePage const code_page,
      Callback* const callback)
      : DecodeHelper(code_page, callback),
        state_(StateAscii) {
  }

  public: virtual ~DbcsDecoder() {
    ASSERT(state_ == StateAscii);
  }

  public: virtual void Feed(
      const char* const start,
      const char* const end) override {
    ASSERT(start != nullptr);
    ASSERT(start <= end);
    if (BulkDecode(start, end)) {
      return;
    }

    const char* ascii_start = nullptr;
    for (auto runner = start; runner < end; runner++) {
      auto const ch = static_cast<uint8>(*runner);

      switch (state_) {
        case StateAscii:
          if (ch <= 0x7F) {
            if (ascii_start == nullptr) {
              ascii_start = runner;
            }

          } else if (::IsDBCSLeadByteEx(code_page_, ch)) {
            SendBytes(ascii_start, runner);
            ascii_start = nullptr;

            bytes_[0] = static_cast<char>(ch);
            state_ = StateLeadByte;

          } else if (ch > 0x7F && code_page_ == CP_SHIFT_JIS) {
            SendBytes(ascii_start, runner);
            ascii_start = nullptr;

            bytes_[0] = static_cast<char>(ch);
            Decode(1);

          } else {
            if (ascii_start == nullptr) {
              ascii_start = runner;
            }
          }
          break;

        case StateLeadByte:
          ASSERT(ascii_start == nullptr);
          bytes_[1] = static_cast<char>(ch);
          Decode(2);
          state_ = StateAscii;
          break;

        default:
          CAN_NOT_HAPPEN();
      } // switch state_
    } // for

    if (ascii_start != nullptr) {
      SendBytes(ascii_start, end);
    }
  }

  public: virtual void Finish() override {
    switch (state_) {
      case StateAscii:
        break;

      case StateLeadByte:
        SendBytes(bytes_, bytes_ + 1);
        state_ = StateAscii;
        break;

      default:
        CAN_NOT_HAPPEN();
    }
  }

  DISALLOW_COPY_AND_ASSIGN(DbcsDecoder);
}; // DbcsDecoder

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
class EucJpDecoder : public CharsetDecoder, public DecodeHelper {
  private: enum {
    EUC_SS2 = 0x8E,
    EUC_SS3 = 0x8F
  };

  private: enum State {
    STATE_EUC_0,
    STATE_EUC_1_2,
    STATE_EUC_2_2,
    STATE_EUC_3_2,
    STATE_EUC_3_3
  };

  private: State state_;

  public: EucJpDecoder::EucJpDecoder(
      Callback* const callback)
      : DecodeHelper(CP_SHIFT_JIS, callback),
        state_(STATE_EUC_0) {
  }

  public: virtual ~EucJpDecoder() {
    ASSERT(state_ == STATE_EUC_0);
  }

  public: virtual void Feed(
      const char* const start,
      const char* const end) override {
    ASSERT(start != nullptr);
    ASSERT(start <= end);

    const char* ascii_start = nullptr;
    for (auto runner = start; runner < end; runner++) {
      auto const ch = static_cast<uint8>(*runner);

      switch (state_) {
        case STATE_EUC_0:
          if (ch <= 0x7F) {
            if (ascii_start == nullptr) {
              ascii_start = runner;
            }

          } else if (ch >= 0xA1 && ch <= 0xFE) {
            SendBytes(ascii_start, runner);
            ascii_start = nullptr;

            bytes_[0] = ch & 0x7F;
            state_ = STATE_EUC_1_2;

          } else if (ch == EUC_SS2) {
            SendBytes(ascii_start, runner);
            ascii_start = nullptr;

            state_ = STATE_EUC_2_2;

          } else if (ch == EUC_SS3) {
            SendBytes(ascii_start, runner);
            ascii_start = nullptr;

            state_ = STATE_EUC_3_2;

          } else {
            if (ascii_start == nullptr) {
              ascii_start = runner;
            }
          }
          break;

        case STATE_EUC_1_2:
          ASSERT(ascii_start == nullptr);
          if (ch >= 0xA1 && ch <= 0xFE) {
            bytes_[1] = ch & 0x7F;
            ConvertJisToShiftJis(bytes_);
            Decode(2);
          } else {
            bytes_[1] = static_cast<char>(ch);
            SendBytes(bytes_, bytes_ + 2);
          }
          state_ = STATE_EUC_0;
          break;

        case STATE_EUC_2_2:
          ASSERT(ascii_start == nullptr);
          // Half-width katakana
          if (ch >= 0xA1 || ch<= 0xFE) {
            bytes_[0] = static_cast<char>(ch);
            Decode(1);
          } else {
            bytes_[0] = static_cast<uint8>(EUC_SS2);
            bytes_[1] = static_cast<char>(ch);
            SendBytes(bytes_, bytes_ + 2);
          }
          state_ = STATE_EUC_0;
          break;

          case STATE_EUC_3_2:
            // Windows doesn't support EUC-JP code set 3.
            bytes_[0] = static_cast<uint8>(EUC_SS3);
            bytes_[1] = static_cast<char>(ch);
            SendBytes(bytes_, bytes_ + 2);
            state_ = STATE_EUC_0;
            break;

          default:
            CAN_NOT_HAPPEN();
      } // switch state_
    } // for

    if (ascii_start != nullptr) {
      SendBytes(ascii_start, end);
    }
  }

  public: virtual void Finish() override {
    switch (state_) {
      case STATE_EUC_0:
        break;

      case STATE_EUC_1_2:
        SendBytes(bytes_, bytes_ + 1);
        state_ = STATE_EUC_0;
        break;

      case STATE_EUC_2_2:
        bytes_[0] = static_cast<uint8>(EUC_SS2);
        SendBytes(bytes_, bytes_ + 1);
        state_ = STATE_EUC_0;
        break;

      case STATE_EUC_3_2:
        bytes_[0] = static_cast<uint8>(EUC_SS3);
        SendBytes(bytes_, bytes_ + 1);
        state_ = STATE_EUC_0;
        break;

      default:
        CAN_NOT_HAPPEN();
    }
  }

  DISALLOW_COPY_AND_ASSIGN(EucJpDecoder);
}; // EucJpDecoder

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
class IsoJpDecoder : public CharsetDecoder, public DecodeHelper {
  enum {
    ISO2022_SI = 0x0F,        // Ctrl+O
    ISO2022_SO = 0x0E,        // Ctrl+N
    ISO2022_ESC = 0x1B
  };

  enum State {
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
  };

  private: State state_;

  public: IsoJpDecoder::IsoJpDecoder(
      Callback* const callback)
      : DecodeHelper(CP_SHIFT_JIS, callback),
        state_(STATE_SBC) {
  }

  public: virtual ~IsoJpDecoder() {
    ASSERT(state_ == STATE_SBC);
  }

  public: virtual void Feed(
      const char* const start,
      const char* const end) override {
    ASSERT(start != nullptr);
    ASSERT(start <= end);

    const char* ascii_start = nullptr;
    CodePage code_page = CP_SHIFT_JIS;
    for (auto runner = start; runner < end; runner++) {
      auto const ch = static_cast<uint8>(*runner);

      switch (state_) {
        case STATE_SBC:
          switch (ch) {
            case ISO2022_ESC:
              SendBytes(ascii_start, runner);
              ascii_start = nullptr;

              state_ = STATE_ESC;
              break;

            case ISO2022_SO:
              SendBytes(ascii_start, runner);
              ascii_start = nullptr;

              state_ = STATE_SBC_SO;
              break;

            default:
              if (ascii_start == nullptr) {
                ascii_start = runner;
              }
              break;
           } // switch ch
           break;

        case STATE_SBC_G1:
        case STATE_SBC_SI:
        case STATE_SBC_SO:    // FIXME(yosi) 1999-07-22: SBC_SO @
          if (ch == ISO2022_ESC) {
            state_ = STATE_ESC;

          } else if (ch == ISO2022_SI && state_ == STATE_SBC_SO) {
            state_ = STATE_SBC;

          } else {
            // Note: We accept G1 characters. Some programs emit
            // half-width katakana as G1
            bytes_[0] = ch | 0x80;
            Decode(1);
            break;
          }
          break;

        case STATE_DBC_1:
          if (ch == ISO2022_ESC) {
            state_ = STATE_ESC;

          } else if (ch >= 0x21 && ch <= 0x7E) {
            bytes_[0] = static_cast<char>(ch);
            state_ = STATE_DBC_2;

          } else {
            bytes_[0] = static_cast<char>(ch);
            SendBytes(bytes_, bytes_ + 1);
            state_ = STATE_SBC;
          }
          break;

        case STATE_DBC_2:
          bytes_[1] = static_cast<char>(ch);
          if (ch >= 0x21 && ch <= 0x7E) {
            ConvertJisToShiftJis(bytes_);
            Decode(2);
          } else {
            SendBytes(bytes_, bytes_ + 2);
          }
          state_ = STATE_DBC_1;
          break;

        case STATE_ESC:
          switch (ch) {
            case '$':  // ESC $
              state_ = STATE_ESC_4F;
              break;

            case '(':  // ESC (
              state_ = STATE_ESC_8F;
              break;

            case ISO2022_ESC:  // ESC ESC
              bytes_[0] = static_cast<char>(ch);
              SendBytes(bytes_, bytes_ + 1);
              break;

            default:
              bytes_[0] = static_cast<uint8>(ISO2022_ESC);
              bytes_[1] = static_cast<char>(ch);
              SendBytes(bytes_, bytes_ + 2);
              state_ = STATE_SBC;
              break;
          }
          break;

        case STATE_ESC_4F:    // ESC $
          switch (ch) {
            case '@':  // ESC $ @ = JIS X 0208-1978
            case 'B':  // ESC $ B = JIS X 0208-1983
              code_page = CP_SHIFT_JIS;
              state_ = STATE_DBC_1;
              break;

            case 'A':  // ESC $ A = GB 2312-1980
              code_page = CP_GBK;
              state_ = STATE_DBC_1;
              break;

            case '(':  // ESC $ (
              state_ = STATE_ESC_4F_8F;
              break;

            default:
              bytes_[0] = static_cast<char>(ISO2022_ESC);
              bytes_[1] = '$';
              bytes_[2] = static_cast<char>(ch);
              SendBytes(bytes_, bytes_ + 3);
              state_ = STATE_SBC;
              break;
          } // switch ch
          break;

        case STATE_ESC_4F_8F:  // ESC $ (
          switch (ch) {
            case '@':  // ESC $ ( @ = JIS X 0208-1978
            case 'B':  // ESC $ ( @ = JIS X 0208-1983
              code_page = CP_SHIFT_JIS;
              state_ = STATE_DBC_1;
              break;

            case 'A':  // ESC $ ( A = GB 2312-1980
               code_page = CP_GBK;
               state_ = STATE_DBC_1;
               break;

            case 'C':  // ESC $ ( C = KCS5601-1987
               code_page = CP_KOREA;
               state_ = STATE_DBC_1;
               break;

            default:
              bytes_[0] = ISO2022_ESC;
              bytes_[1] = '$';
              bytes_[2] = '(';
              bytes_[3] = static_cast<char>(ch);
              SendBytes(bytes_, bytes_ + 4);
              state_ = STATE_SBC;
              break;
            } // switch ch
            break;

        case STATE_ESC_8F:    // ESC (
          switch (ch) {
            case 'B': // ESC ( B = ASCII
            case 'J': // ESC ( J = JIS X201-1976:G0
              state_ = STATE_SBC;
              code_page = CP_SHIFT_JIS;
              break;

            case 'I':  // ESC ( I = JIS X202-1984:G1
              state_ = STATE_SBC_G1;
              code_page = CP_SHIFT_JIS;
              break;

            default:
              bytes_[0] = ISO2022_ESC;
              bytes_[1] = '$';
              bytes_[2] = static_cast<char>(ch);
              SendBytes(bytes_, bytes_ + 3);
              state_ = STATE_SBC;
              break;
            } // switch ch
            break;

        default:
          CAN_NOT_HAPPEN();
      } // switch state_
    } // for

    if (ascii_start != nullptr) {
      SendBytes(ascii_start, end);
    }
  } // Feed

  public: virtual void Finish() override {
    switch (state_) {
      case STATE_SBC:
        break;

      case STATE_SBC_G1:
      case STATE_SBC_SI:
      case STATE_SBC_SO:
      case STATE_ESC:
      case STATE_ESC_4F:
      case STATE_ESC_8F:
      case STATE_ESC_4F_8F:
      case STATE_DBC_1:
        state_ = STATE_SBC;
        break;

      case STATE_DBC_2:
        SendBytes(bytes_, bytes_ + 1);
        state_ = STATE_SBC;
        break;

      default:
        CAN_NOT_HAPPEN();
    }
  }

  DISALLOW_COPY_AND_ASSIGN(IsoJpDecoder);
}; // IsoJpDecoder

// Single Byte Charset decoder.
// Simply call MBCS2WCS.
class SbcsDecoder : public CharsetDecoder, public DecodeHelper {
  public: SbcsDecoder::SbcsDecoder(
      CodePage const code_page,
      Callback* const callback)
      : DecodeHelper(code_page, callback) {
  }

  public: virtual void Feed(
      const char* const start,
      const char* const end) override {
    ASSERT(start != nullptr);
    ASSERT(start <= end);
    if (BulkDecode(start, end)) {
      return;
    }

    for (auto runner = start; runner < end; runner++) {
      bytes_[0] = *runner;
      Decode(1);
    }
  } 

  DISALLOW_COPY_AND_ASSIGN(SbcsDecoder);
}; // SbcsDecoder

// UTF-8 Charset decoder.
// U+0000..U+007F 0xxxxxxx
// U+0080..U+07FF 110xxxxx 10xxxxxx
// U+0800..U+FFFF 1110xxxx 10xxxxxx 10xxxxxx
class Utf8Decoder : public CharsetDecoder, public DecodeHelper {
  private: enum State {
    Utf8_0,
    Utf8_2,
    Utf8_3,
    Utf8_3_2,
  };

  private: State state_;
  private: char16 char16_;

  public: Utf8Decoder::Utf8Decoder(Callback* const callback)
      : DecodeHelper(CP_UTF_8, callback),
        state_(Utf8_0) {
  }

  public: virtual void Feed(
      const char* const start,
      const char* const end) override {
    ASSERT(start != nullptr);
    ASSERT(start <= end);

    const char* ascii_start = nullptr;
    for (auto runner = start; runner < end; runner++) {
      auto const ch = static_cast<uint8>(*runner);

      switch (state_) {
        case Utf8_0:
          if (ch <= 0x7E) {
            if (ascii_start == nullptr) {
              ascii_start = runner;
            }

          } else if (ch >= 0xC0 && ch <= 0xDF) {
            SendBytes(ascii_start, runner);
            ascii_start = nullptr;

            bytes_[0] = static_cast<char>(ch);
            char16_ = static_cast<char16>((ch & 0x1F) << 6);
            state_ = Utf8_2;

          } else if (ch >= 0xE0 && ch <= 0xEF) {
            SendBytes(ascii_start, runner);
            ascii_start = nullptr;

            bytes_[0] = static_cast<char>(ch);
            char16_ = static_cast<char16>((ch & 0x0F) << 12);
            state_ = Utf8_3;

          } else {
            if (ascii_start == nullptr) {
              ascii_start = runner;
            }
          }
          break;

        case Utf8_2:
          ASSERT(ascii_start == nullptr);
          if (ch >= 0x80 && ch <= 0xBF) {
            char16_ |= ch & 0x3F;
            SendChar(char16_);
          } else {
            bytes_[1] = static_cast<char>(ch);
            SendBytes(bytes_, bytes_ + 2);
          }
          state_ = Utf8_0;
          break;

        case Utf8_3:
          ASSERT(ascii_start == nullptr);
          bytes_[1] = static_cast<char>(ch);
          if (ch >= 0x80 && ch <= 0xBF) {
            char16_ |= (ch & 0x3f) << 6;
            state_ = Utf8_3_2;

          } else {
            SendBytes(bytes_, bytes_ + 2);
            state_ = Utf8_0;
          }
          break;

        case Utf8_3_2:
          ASSERT(ascii_start == nullptr);
          if (ch >= 0x80 && ch <= 0xBF) {
            char16_ |= ch & 0x3f;
            SendChar(char16_);
          } else {
            bytes_[2] = static_cast<char>(ch);
            SendBytes(bytes_, bytes_ + 3);
          }
          state_ = Utf8_0;
          break;

        default:
          CAN_NOT_HAPPEN();
      } // swtich state_
    } // for

    if (ascii_start != nullptr) {
      SendBytes(ascii_start, end);
    }
  } // Feed

  public: virtual void Finish() override {
    switch (state_) {
      case Utf8_0:
        break;

      case Utf8_2:
        SendBytes(bytes_, bytes_ + 1);
        state_ = Utf8_0;
        break;

      case Utf8_3:
        SendBytes(bytes_, bytes_ + 1);
        state_ = Utf8_0;
        break;

      case Utf8_3_2:
        SendBytes(bytes_, bytes_ + 2);
        state_ = Utf8_0;
        break;

      default:
        CAN_NOT_HAPPEN();
    }
  } // Finish

  DISALLOW_COPY_AND_ASSIGN(Utf8Decoder);
}; // Utf8Decoder

} // namespace

CharsetDecoder* CharsetDecoder::Create(
    CodePage const code_page,
    Callback* const callback) {
  ASSERT(callback != nullptr);
  switch (code_page) {
    case CP_BIG5:
    case CP_GBK:
    case CP_JOHAB:
    case CP_KOREA:
    case CP_SHIFT_JIS:
      return new DbcsDecoder(code_page, callback);

    case CP_EUC_JP:
    case CP_EUC_JP_OLD:
      return new EucJpDecoder(callback);

    case CP_ISO_2022_JP:
      return new IsoJpDecoder(callback);

    case CP_UTF_8:
      return new Utf8Decoder(callback);

    default:
      return new SbcsDecoder(code_page, callback);
  }
}

} // Charset
