// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <memory>

#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "base/macros.h"
#include "evita/ginx/converter.h"
#include "third_party/icu/source/common/unicode/uchar.h"
#include "third_party/icu/source/common/unicode/uscript.h"
#include "third_party/icu/source/common/unicode/utypes.h"

namespace dom {
namespace internal {

namespace {
// Note: The order of |kCategoryNames| must be matched to |UCharCategory|.
const char* kCategoryNames[] = {
    "Cn", "Lu", "Ll", "Lt", "Lm", "Lo", "Mn", "Me", "Mc", "Nd",
    "Nl", "No", "Zs", "Zl", "Zp", "Cc", "Cf", "Co", "Cs", "Pd",
    "Ps", "Pe", "Pc", "Po", "Sm", "Sc", "Sk", "So", "Pi", "Pf",
};

static_assert(sizeof(kCategoryNames) / sizeof(*kCategoryNames) ==
                  U_CHAR_CATEGORY_COUNT,
              "kCategoryNames size mismatch");

#define FOR_EACH_SCRIPT(V)                    \
  V(COMMON, 0, "Zyyy")                        \
  V(INHERITED, 1, "Zinh")                     \
  V(ARABIC, 2, "Arab")                        \
  V(ARMENIAN, 3, "Armn")                      \
  V(BENGALI, 4, "Beng")                       \
  V(BOPOMOFO, 5, "Bopo")                      \
  V(CHEROKEE, 6, "Cher")                      \
  V(COPTIC, 7, "Copt")                        \
  V(CYRILLIC, 8, "Cyrl")                      \
  V(DESERET, 9, "Dsrt")                       \
  V(DEVANAGARI, 10, "Deva")                   \
  V(ETHIOPIC, 11, "Ethi")                     \
  V(GEORGIAN, 12, "Geor")                     \
  V(GOTHIC, 13, "Goth")                       \
  V(GREEK, 14, "Grek")                        \
  V(GUJARATI, 15, "Gujr")                     \
  V(GURMUKHI, 16, "Guru")                     \
  V(HAN, 17, "Hani")                          \
  V(HANGUL, 18, "Hang")                       \
  V(HEBREW, 19, "Hebr")                       \
  V(HIRAGANA, 20, "Hira")                     \
  V(KANNADA, 21, "Knda")                      \
  V(KATAKANA, 22, "Kana")                     \
  V(KHMER, 23, "Khmr")                        \
  V(LAO, 24, "Laoo")                          \
  V(LATIN, 25, "Latn")                        \
  V(MALAYALAM, 26, "Mlym")                    \
  V(MONGOLIAN, 27, "Mong")                    \
  V(MYANMAR, 28, "Mymr")                      \
  V(OGHAM, 29, "Ogam")                        \
  V(OLD_ITALIC, 30, "Ital")                   \
  V(ORIYA, 31, "Orya")                        \
  V(RUNIC, 32, "Runr")                        \
  V(SINHALA, 33, "Sinh")                      \
  V(SYRIAC, 34, "Syrc")                       \
  V(TAMIL, 35, "Taml")                        \
  V(TELUGU, 36, "Telu")                       \
  V(THAANA, 37, "Thaa")                       \
  V(THAI, 38, "Thai")                         \
  V(TIBETAN, 39, "Tibt")                      \
  V(CANADIAN_ABORIGINAL, 40, "Cans")          \
  V(YI, 41, "Yiii")                           \
  V(TAGALOG, 42, "Tglg")                      \
  V(HANUNOO, 43, "Hano")                      \
  V(BUHID, 44, "Buhd")                        \
  V(TAGBANWA, 45, "Tagb")                     \
  V(BRAILLE, 46, "Brai")                      \
  V(CYPRIOT, 47, "Cprt")                      \
  V(LIMBU, 48, "Limb")                        \
  V(LINEAR_B, 49, "Linb")                     \
  V(OSMANYA, 50, "Osma")                      \
  V(SHAVIAN, 51, "Shaw")                      \
  V(TAI_LE, 52, "Tale")                       \
  V(UGARITIC, 53, "Ugar")                     \
  V(KATAKANA_OR_HIRAGANA, 54, "Hrkt")         \
  V(BUGINESE, 55, "Bugi")                     \
  V(GLAGOLITIC, 56, "Glag")                   \
  V(KHAROSHTHI, 57, "Khar")                   \
  V(SYLOTI_NAGRI, 58, "Sylo")                 \
  V(NEW_TAI_LUE, 59, "Talu")                  \
  V(TIFINAGH, 60, "Tfng")                     \
  V(OLD_PERSIAN, 61, "Xpeo")                  \
  V(BALINESE, 62, "Bali")                     \
  V(BATAK, 63, "Batk")                        \
  V(BLISSYMBOLS, 64, "Blis")                  \
  V(BRAHMI, 65, "Brah")                       \
  V(CHAM, 66, "Cham")                         \
  V(CIRTH, 67, "Cirt")                        \
  V(OLD_CHURCH_SLAVONIC_CYRILLIC, 68, "Cyrs") \
  V(DEMOTIC_EGYPTIAN, 69, "Egyd")             \
  V(HIERATIC_EGYPTIAN, 70, "Egyh")            \
  V(EGYPTIAN_HIEROGLYPHS, 71, "Egyp")         \
  V(KHUTSURI, 72, "Geok")                     \
  V(SIMPLIFIED_HAN, 73, "Hans")               \
  V(TRADITIONAL_HAN, 74, "Hant")              \
  V(PAHAWH_HMONG, 75, "Hmng")                 \
  V(OLD_HUNGARIAN, 76, "Hung")                \
  V(HARAPPAN_INDUS, 77, "Inds")               \
  V(JAVANESE, 78, "Java")                     \
  V(KAYAH_LI, 79, "Kali")                     \
  V(LATIN_FRAKTUR, 80, "Latf")                \
  V(LATIN_GAELIC, 81, "Latg")                 \
  V(LEPCHA, 82, "Lepc")                       \
  V(LINEAR_A, 83, "Lina")                     \
  V(MANDAIC, 84, "Mand")                      \
  V(MAYAN_HIEROGLYPHS, 85, "Maya")            \
  V(MEROITIC_HIEROGLYPHS, 86, "Mero")         \
  V(NKO, 87, "Nkoo")                          \
  V(ORKHON, 88, "Orkh")                       \
  V(OLD_PERMIC, 89, "Perm")                   \
  V(PHAGS_PA, 90, "Phag")                     \
  V(PHOENICIAN, 91, "Phnx")                   \
  V(MIAO, 92, "Plrd")                         \
  V(RONGORONGO, 93, "Roro")                   \
  V(SARATI, 94, "Sara")                       \
  V(ESTRANGELO_SYRIAC, 95, "Syre")            \
  V(WESTERN_SYRIAC, 96, "Syrj")               \
  V(EASTERN_SYRIAC, 97, "Syrn")               \
  V(TENGWAR, 98, "Teng")                      \
  V(VAI, 99, "Vaii")                          \
  V(VISIBLE_SPEECH, 100, "Visp")              \
  V(CUNEIFORM, 101, "Xsux")                   \
  V(UNWRITTEN_LANGUAGES, 102, "Zxxx")         \
  V(UNKNOWN, 103, "Zzzz")                     \
  V(CARIAN, 104, "Cari")                      \
  V(JAPANESE, 105, "Jpan")                    \
  V(LANNA, 106, "Lana")                       \
  V(LYCIAN, 107, "Lyci")                      \
  V(LYDIAN, 108, "Lydi")                      \
  V(OL_CHIKI, 109, "Olck")                    \
  V(REJANG, 110, "Rjng")                      \
  V(SAURASHTRA, 111, "Saur")                  \
  V(SIGN_WRITING, 112, "Sgnw")                \
  V(SUNDANESE, 113, "Sund")                   \
  V(MOON, 114, "Moon")                        \
  V(MEITEI_MAYEK, 115, "Mtei")                \
  V(IMPERIAL_ARAMAIC, 116, "Armi")            \
  V(AVESTAN, 117, "Avst")                     \
  V(CHAKMA, 118, "Cakm")                      \
  V(KOREAN, 119, "Kore")                      \
  V(KAITHI, 120, "Kthi")                      \
  V(MANICHAEAN, 121, "Mani")                  \
  V(INSCRIPTIONAL_PAHLAVI, 122, "Phli")       \
  V(PSALTER_PAHLAVI, 123, "Phlp")             \
  V(BOOK_PAHLAVI, 124, "Phlv")                \
  V(INSCRIPTIONAL_PARTHIAN, 125, "Prti")      \
  V(SAMARITAN, 126, "Samr")                   \
  V(TAI_VIET, 127, "Tavt")                    \
  V(MATHEMATICAL_NOTATION, 128, "Zmth")       \
  V(SYMBOLS, 129, "Zsym")                     \
  V(BAMUM, 130, "Bamu")                       \
  V(LISU, 131, "Lisu")                        \
  V(NAKHI_GEBA, 132, "Nkgb")                  \
  V(OLD_SOUTH_ARABIAN, 133, "Sarb")           \
  V(BASSA_VAH, 134, "Bass")                   \
  V(DUPLOYAN, 135, "Dupl")                    \
  V(ELBASAN, 136, "Elba")                     \
  V(GRANTHA, 137, "Gran")                     \
  V(KPELLE, 138, "Kpel")                      \
  V(LOMA, 139, "Loma")                        \
  V(MENDE, 140, "Mend")                       \
  V(MEROITIC_CURSIVE, 141, "Merc")            \
  V(OLD_NORTH_ARABIAN, 142, "Narb")           \
  V(NABATAEAN, 143, "Nbat")                   \
  V(PALMYRENE, 144, "Palm")                   \
  V(KHUDAWADI, 145, "Sind")                   \
  V(WARANG_CITI, 146, "Wara")                 \
  V(AFAKA, 147, "Afak")                       \
  V(JURCHEN, 148, "Jurc")                     \
  V(MRO, 149, "Mroo")                         \
  V(NUSHU, 150, "Nshu")                       \
  V(SHARADA, 151, "Shrd")                     \
  V(SORA_SOMPENG, 152, "Sora")                \
  V(TAKRI, 153, "Takr")                       \
  V(TANGUT, 154, "Tang")                      \
  V(WOLEAI, 155, "Wole")                      \
  V(ANATOLIAN_HIEROGLYPHS, 156, "Hluw")       \
  V(KHOJKI, 157, "Khoj")                      \
  V(TIRHUTA, 158, "Tirh")                     \
  V(CAUCASIAN_ALBANIAN, 159, "Aghb")          \
  V(MAHAJANI, 160, "Mahj")                    \
  V(AHOM, 161, "Ahom")                        \
  V(HATRAN, 162, "Hatr")                      \
  V(MODI, 163, "Modi")                        \
  V(MULTANI, 164, "Mult")                     \
  V(PAU_CIN_HAU, 165, "Pauc")                 \
  V(SIDDHAM, 166, "Sidd")

#define USCRIPT_TAI_THAM USCRIPT_LANNA
#define USCRIPT_MENDE_KIKAKUI USCRIPT_MENDE
#define USCRIPT_MEETEI_MAYEK USCRIPT_MEITEI_MAYEK
#define USCRIPT_OLD_TURKIC USCRIPT_ORKHON

#define DCHECK_EQ_CHAR_1(vector, index, name) \
  DCHECK((vector)[index][0] == (name)[0] && (vector)[index][1] == (name)[1])

#define DCHECK_EQ_CHAR_2(vector, index, name)                                  \
  DCHECK((vector)[index][0] == (name)[0] && (vector)[index][1] == (name)[1] && \
         (vector)[index][2] == (name)[2])

#define DCHECK_EQ_CHAR_3(vector, index, name)                                  \
  DCHECK((vector)[index][0] == (name)[0] && (vector)[index][1] == (name)[1] && \
         (vector)[index][2] == (name)[2] && (vector)[index][3] == (name)[3])

// Unicode.CATEGORY_SHOT_NAMES : Array.<string>
// Unicode.SCRIPT_NAMES : Array.<string>
// Unicode.Category : enum
// Unicode.Script: enum
// Unicode.UCD : Array.<{
//  category: Unicode.Category,
//  script: Unicode.Script,
// }>
v8::Local<v8::Object> CreateUnicode(v8::Isolate* isolate) {
  DCHECK_EQ_CHAR_2(kCategoryNames, U_UNASSIGNED, "Cn");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_UPPERCASE_LETTER, "Lu");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_LOWERCASE_LETTER, "Ll");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_TITLECASE_LETTER, "Lt");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_MODIFIER_LETTER, "Lm");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_OTHER_LETTER, "Lo");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_NON_SPACING_MARK, "Mn");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_ENCLOSING_MARK, "Me");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_COMBINING_SPACING_MARK, "Mc");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_DECIMAL_DIGIT_NUMBER, "Nd");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_LETTER_NUMBER, "Nl");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_OTHER_NUMBER, "No");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_SPACE_SEPARATOR, "Zs");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_LINE_SEPARATOR, "Zl");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_PARAGRAPH_SEPARATOR, "Zp");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_CONTROL_CHAR, "Cc");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_FORMAT_CHAR, "Cf");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_PRIVATE_USE_CHAR, "Co");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_SURROGATE, "Cs");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_DASH_PUNCTUATION, "Pd");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_START_PUNCTUATION, "Ps");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_END_PUNCTUATION, "Pe");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_CONNECTOR_PUNCTUATION, "Pc");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_OTHER_PUNCTUATION, "Po");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_MATH_SYMBOL, "Sm");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_CURRENCY_SYMBOL, "Sc");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_MODIFIER_SYMBOL, "Sk");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_OTHER_SYMBOL, "So");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_INITIAL_PUNCTUATION, "Pi");
  DCHECK_EQ_CHAR_2(kCategoryNames, U_FINAL_PUNCTUATION, "Pf");

  std::array<v8::Local<v8::Symbol>, USCRIPT_CODE_LIMIT> kScriptAbbrevs;
#define V(id, num, abbrev)       \
  kScriptAbbrevs[USCRIPT_##id] = \
      v8::Symbol::New(isolate, gin::StringToV8(isolate, #id));
  FOR_EACH_SCRIPT(V)
#undef V

  v8::EscapableHandleScope handle_scope(isolate);
  auto unicode = v8::Object::New(isolate);

  // Category name
  auto category_names = v8::Array::New(isolate, arraysize(kCategoryNames));
  auto category_object = v8::Object::New(isolate);
  for (auto i = 0; i < arraysize(kCategoryNames); ++i) {
    auto name = gin::StringToV8(isolate, kCategoryNames[i]);
    category_object->Set(name, name);
    category_names->Set(static_cast<size_t>(i), name);
  }
  unicode->Set(gin::StringToV8(isolate, "Category"), category_object);
  unicode->Set(gin::StringToV8(isolate, "CATEGORY_SHORT_NAMES"),
               category_names);

  // Script name
  auto script_names = v8::Array::New(isolate, USCRIPT_CODE_LIMIT);
  auto script_object = v8::Object::New(isolate);
#define V(id, num, abbrev)                          \
  script_object->Set(gin::StringToV8(isolate, #id), \
                     kScriptAbbrevs[USCRIPT_##id]); \
  script_names->Set(USCRIPT_##id, gin::StringToV8(isolate, #id));
  FOR_EACH_SCRIPT(V)
#undef V
  unicode->Set(gin::StringToV8(isolate, "Script"), script_object);
  unicode->Set(gin::StringToV8(isolate, "SCRIPT_NAMES"), script_names);

  // UCD
  auto ucd = v8::Array::New(isolate, 0x10000);
  unicode->Set(gin::StringToV8(isolate, "UCD"), ucd);
  auto name = gin::StringToV8(isolate, "name");
  auto category = gin::StringToV8(isolate, "category");
  auto script_string = gin::StringToV8(isolate, "script");
  for (auto code = 0; code <= 0xFFFF; ++code) {
    auto data = v8::Object::New(isolate);

    auto error_code = U_ZERO_ERROR;
#if 0
    // Note: Because of Chromium icudata doesn't contain unames.icu. So, we
    // can't have character names.
    // name
    char name_buffer[100];
    u_charName(code, U_UNICODE_CHAR_NAME, name_buffer, sizeof(name_buffer),
               &error_code);
    CHECK_EQ(U_ZERO_ERROR, error_code);
    data->ForceSet(name, gin::StringToV8(isolate, name_buffer));
#endif

    // general category
    auto const category_index = u_charType(code);
    CHECK(category_index < arraysize(kCategoryNames));
    data->ForceSet(category,
                   category_names->Get(static_cast<size_t>(category_index)));

    // script
    auto const script_code = uscript_getScript(code, &error_code);
    data->ForceSet(script_string, kScriptAbbrevs[script_code]);

    ucd->Set(static_cast<size_t>(code), data);
  }
  return handle_scope.Escape(unicode);
}

// Note: Creating UCD object takes a few second in debug build. To make
// test faster, we cache UCD object.
class Unicode final {
 public:
  explicit Unicode(v8::Isolate* isolate)
      : unicode_(new v8::UniquePersistent<v8::Object>()) {
    unicode_->Reset(isolate, CreateUnicode(isolate));
  }
  ~Unicode() = default;

  v8::Local<v8::Object> Get(v8::Isolate* isolate) {
    return v8::Local<v8::Object>::New(isolate, *unicode_);
  }

 private:
  std::unique_ptr<v8::UniquePersistent<v8::Object>> unicode_;

  DISALLOW_COPY_AND_ASSIGN(Unicode);
};

}  // namespace

v8::Local<v8::Object> GetUnicodeObject(v8::Isolate* isolate) {
  CR_DEFINE_STATIC_LOCAL(Unicode, static_unicode, (isolate));
  return static_unicode.Get(isolate);
}

}  // namespace internal
}  // namespace dom
