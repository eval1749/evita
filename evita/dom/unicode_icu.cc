// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <memory>

#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "base/macros.h"
#include "evita/v8_glue/converter.h"
#include "third_party/icu/source/common/unicode/uchar.h"
#include "third_party/icu/source/common/unicode/uscript.h"
#include "third_party/icu/source/common/unicode/utypes.h"

namespace dom {
namespace internal {

namespace {
// Note: The order of |kBidiClassNames| must be matched to |UCharDirection|.
const char* kBidiClassNames[] = {
    "L",   "R",   "EN", "ES",  "ET",  "AN",  "CS",  "B",
    "S",   "WS",  "ON", "LRE", "LRO", "AL",  "RLE", "RLO",
    "PDF", "NSM", "BN", "FSI", "LRI", "RLI", "PDI",
};

static_assert(sizeof(kBidiClassNames) / sizeof(*kBidiClassNames) ==
                  U_CHAR_DIRECTION_COUNT,
              "kBidiClassNames size mismatch");

// Note: The order of |kCategoryNames| must be matched to |UCharCategory|.
const char* kCategoryNames[] = {
    "Cn", "Lu", "Ll", "Lt", "Lm", "Lo", "Mn", "Me", "Mc", "Nd",
    "Nl", "No", "Zs", "Zl", "Zp", "Cc", "Cf", "Co", "Cs", "Pd",
    "Ps", "Pe", "Pc", "Po", "Sm", "Sc", "Sk", "So", "Pi", "Pf",
};

static_assert(sizeof(kCategoryNames) / sizeof(*kCategoryNames) ==
                  U_CHAR_CATEGORY_COUNT,
              "kCategoryNames size mismatch");

#define FOR_EACH_SCRIPT(V)                                    \
  V(CAUCASIAN_ALBANIAN, "Aghb", "Caucasian Albanian")         \
  V(AHOM, "Ahom", "Ahom")                                     \
  V(ARABIC, "Arab", "Arabic")                                 \
  V(IMPERIAL_ARAMAIC, "Armi", "Imperial Aramaic")             \
  V(ARMENIAN, "Armn", "Armenian")                             \
  V(AVESTAN, "Avst", "Avestan")                               \
  V(BALINESE, "Bali", "Balinese")                             \
  V(BAMUM, "Bamu", "Bamum")                                   \
  V(BASSA_VAH, "Bass", "Bassa Vah")                           \
  V(BATAK, "Batk", "Batak")                                   \
  V(BENGALI, "Beng", "Bengali")                               \
  V(BOPOMOFO, "Bopo", "Bopomofo")                             \
  V(BRAHMI, "Brah", "Brahmi")                                 \
  V(BRAILLE, "Brai", "Braille")                               \
  V(BUGINESE, "Bugi", "Buginese")                             \
  V(BUHID, "Buhd", "Buhid")                                   \
  V(CHAKMA, "Cakm", "Chakma")                                 \
  V(CANADIAN_ABORIGINAL, "Cans", "Canadian Aboriginal")       \
  V(CARIAN, "Cari", "Carian")                                 \
  V(CHAM, "Cham", "Cham")                                     \
  V(CHEROKEE, "Cher", "Cherokee")                             \
  V(COPTIC, "Copt", "Coptic")                                 \
  V(CYPRIOT, "Cprt", "Cypriot")                               \
  V(CYRILLIC, "Cyrl", "Cyrillic")                             \
  V(DEVANAGARI, "Deva", "Devanagari")                         \
  V(DESERET, "Dsrt", "Deseret")                               \
  V(DUPLOYAN, "Dupl", "Duployan")                             \
  V(EGYPTIAN_HIEROGLYPHS, "Egyp", "Egyptian Hieroglyphs")     \
  V(ELBASAN, "Elba", "Elbasan")                               \
  V(ETHIOPIC, "Ethi", "Ethiopic")                             \
  V(GEORGIAN, "Geor", "Georgian")                             \
  V(GLAGOLITIC, "Glag", "Glagolitic")                         \
  V(GOTHIC, "Goth", "Gothic")                                 \
  V(GRANTHA, "Gran", "Grantha")                               \
  V(GREEK, "Grek", "Greek")                                   \
  V(GUJARATI, "Gujr", "Gujarati")                             \
  V(GURMUKHI, "Guru", "Gurmukhi")                             \
  V(HANGUL, "Hang", "Hangul")                                 \
  V(HAN, "Hani", "Han")                                       \
  V(HANUNOO, "Hano", "Hanunoo")                               \
  V(HATRAN, "Hatr", "Hatran")                                 \
  V(HEBREW, "Hebr", "Hebrew")                                 \
  V(HIRAGANA, "Hira", "Hiragana")                             \
  V(ANATOLIAN_HIEROGLYPHS, "Hluw", "Anatolian Hieroglyphs")   \
  V(PAHAWH_HMONG, "Hmng", "Pahawh Hmong")                     \
  V(KATAKANA_OR_HIRAGANA, "Hrkt", "Katakana Or Hiragana")     \
  V(OLD_HUNGARIAN, "Hung", "Old Hungarian")                   \
  V(OLD_ITALIC, "Ital", "Old Italic")                         \
  V(JAVANESE, "Java", "Javanese")                             \
  V(KAYAH_LI, "Kali", "Kayah Li")                             \
  V(KATAKANA, "Kana", "Katakana")                             \
  V(KHAROSHTHI, "Khar", "Kharoshthi")                         \
  V(KHMER, "Khmr", "Khmer")                                   \
  V(KHOJKI, "Khoj", "Khojki")                                 \
  V(KANNADA, "Knda", "Kannada")                               \
  V(KAITHI, "Kthi", "Kaithi")                                 \
  V(TAI_THAM, "Lana", "Tai Tham")                             \
  V(LAO, "Laoo", "Lao")                                       \
  V(LATIN, "Latn", "Latin")                                   \
  V(LEPCHA, "Lepc", "Lepcha")                                 \
  V(LIMBU, "Limb", "Limbu")                                   \
  V(LINEAR_A, "Lina", "Linear A")                             \
  V(LINEAR_B, "Linb", "Linear B")                             \
  V(LISU, "Lisu", "Lisu")                                     \
  V(LYCIAN, "Lyci", "Lycian")                                 \
  V(LYDIAN, "Lydi", "Lydian")                                 \
  V(MAHAJANI, "Mahj", "Mahajani")                             \
  V(MANDAIC, "Mand", "Mandaic")                               \
  V(MANICHAEAN, "Mani", "Manichaean")                         \
  V(MENDE_KIKAKUI, "Mend", "Mende Kikakui")                   \
  V(MEROITIC_CURSIVE, "Merc", "Meroitic Cursive")             \
  V(MEROITIC_HIEROGLYPHS, "Mero", "Meroitic Hieroglyphs")     \
  V(MALAYALAM, "Mlym", "Malayalam")                           \
  V(MODI, "Modi", "Modi")                                     \
  V(MONGOLIAN, "Mong", "Mongolian")                           \
  V(MRO, "Mroo", "Mro")                                       \
  V(MEETEI_MAYEK, "Mtei", "Meetei Mayek")                     \
  V(MULTANI, "Mult", "Multani")                               \
  V(MYANMAR, "Mymr", "Myanmar")                               \
  V(OLD_NORTH_ARABIAN, "Narb", "Old North Arabian")           \
  V(NABATAEAN, "Nbat", "Nabataean")                           \
  V(NKO, "Nkoo", "Nko")                                       \
  V(OGHAM, "Ogam", "Ogham")                                   \
  V(OL_CHIKI, "Olck", "Ol Chiki")                             \
  V(OLD_TURKIC, "Orkh", "Old Turkic")                         \
  V(ORIYA, "Orya", "Oriya")                                   \
  V(OSMANYA, "Osma", "Osmanya")                               \
  V(PALMYRENE, "Palm", "Palmyrene")                           \
  V(PAU_CIN_HAU, "Pauc", "Pau Cin Hau")                       \
  V(OLD_PERMIC, "Perm", "Old Permic")                         \
  V(PHAGS_PA, "Phag", "Phags Pa")                             \
  V(INSCRIPTIONAL_PAHLAVI, "Phli", "Inscriptional Pahlavi")   \
  V(PSALTER_PAHLAVI, "Phlp", "Psalter Pahlavi")               \
  V(PHOENICIAN, "Phnx", "Phoenician")                         \
  V(MIAO, "Plrd", "Miao")                                     \
  V(INSCRIPTIONAL_PARTHIAN, "Prti", "Inscriptional Parthian") \
  V(REJANG, "Rjng", "Rejang")                                 \
  V(RUNIC, "Runr", "Runic")                                   \
  V(SAMARITAN, "Samr", "Samaritan")                           \
  V(OLD_SOUTH_ARABIAN, "Sarb", "Old South Arabian")           \
  V(SAURASHTRA, "Saur", "Saurashtra")                         \
  V(SIGN_WRITING, "Sgnw", "Sign Writing")                     \
  V(SHAVIAN, "Shaw", "Shavian")                               \
  V(SHARADA, "Shrd", "Sharada")                               \
  V(SIDDHAM, "Sidd", "Siddham")                               \
  V(KHUDAWADI, "Sind", "Khudawadi")                           \
  V(SINHALA, "Sinh", "Sinhala")                               \
  V(SORA_SOMPENG, "Sora", "Sora Sompeng")                     \
  V(SUNDANESE, "Sund", "Sundanese")                           \
  V(SYLOTI_NAGRI, "Sylo", "Syloti Nagri")                     \
  V(SYRIAC, "Syrc", "Syriac")                                 \
  V(TAGBANWA, "Tagb", "Tagbanwa")                             \
  V(TAKRI, "Takr", "Takri")                                   \
  V(TAI_LE, "Tale", "Tai Le")                                 \
  V(NEW_TAI_LUE, "Talu", "New Tai Lue")                       \
  V(TAMIL, "Taml", "Tamil")                                   \
  V(TAI_VIET, "Tavt", "Tai Viet")                             \
  V(TELUGU, "Telu", "Telugu")                                 \
  V(TIFINAGH, "Tfng", "Tifinagh")                             \
  V(TAGALOG, "Tglg", "Tagalog")                               \
  V(THAANA, "Thaa", "Thaana")                                 \
  V(THAI, "Thai", "Thai")                                     \
  V(TIBETAN, "Tibt", "Tibetan")                               \
  V(TIRHUTA, "Tirh", "Tirhuta")                               \
  V(UGARITIC, "Ugar", "Ugaritic")                             \
  V(VAI, "Vaii", "Vai")                                       \
  V(WARANG_CITI, "Wara", "Warang Citi")                       \
  V(OLD_PERSIAN, "Xpeo", "Old Persian")                       \
  V(CUNEIFORM, "Xsux", "Cuneiform")                           \
  V(YI, "Yiii", "Yi")                                         \
  V(INHERITED, "Zinh", "Inherited")                           \
  V(COMMON, "Zyyy", "Common")                                 \
  V(UNKNOWN, "Zzzz", "Unknown")

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

// Unicode.BIDI_CLASS_SHORT_NAMES : Array.<string>
// Unicode.GENERAL_CATEGORY_SHOT_NAMES : Array.<string>
// Unicode.Bidi : enum
// Unicode.Category : enum
// Unicode.Script: enum
// Uicode.UCD : Array.<{
//  bidi: Unicode.Bidi,
//  category: Unicode.Category,
//  script: Unicode.Script,
// }>
v8::Handle<v8::Object> CreateUnicode(v8::Isolate* isolate) {
  DCHECK_EQ_CHAR_1(kBidiClassNames, U_LEFT_TO_RIGHT, "L");
  DCHECK_EQ_CHAR_1(kBidiClassNames, U_RIGHT_TO_LEFT, "R");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_EUROPEAN_NUMBER, "EN");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_EUROPEAN_NUMBER_SEPARATOR, "ES");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_EUROPEAN_NUMBER_TERMINATOR, "ET");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_ARABIC_NUMBER, "AN");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_COMMON_NUMBER_SEPARATOR, "CS");
  DCHECK_EQ_CHAR_1(kBidiClassNames, U_BLOCK_SEPARATOR, "B");
  DCHECK_EQ_CHAR_1(kBidiClassNames, U_SEGMENT_SEPARATOR, "S");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_WHITE_SPACE_NEUTRAL, "WS");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_OTHER_NEUTRAL, "ON");
  DCHECK_EQ_CHAR_3(kBidiClassNames, U_LEFT_TO_RIGHT_EMBEDDING, "LRE");
  DCHECK_EQ_CHAR_3(kBidiClassNames, U_LEFT_TO_RIGHT_OVERRIDE, "LRO");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_RIGHT_TO_LEFT_ARABIC, "AL");
  DCHECK_EQ_CHAR_3(kBidiClassNames, U_RIGHT_TO_LEFT_EMBEDDING, "RLE");
  DCHECK_EQ_CHAR_3(kBidiClassNames, U_RIGHT_TO_LEFT_OVERRIDE, "RLO");
  DCHECK_EQ_CHAR_3(kBidiClassNames, U_POP_DIRECTIONAL_FORMAT, "PDF");
  DCHECK_EQ_CHAR_3(kBidiClassNames, U_DIR_NON_SPACING_MARK, "NSM");
  DCHECK_EQ_CHAR_2(kBidiClassNames, U_BOUNDARY_NEUTRAL, "BN");

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
#define V(id, abbrev, name)      \
  kScriptAbbrevs[USCRIPT_##id] = \
      v8::Symbol::New(isolate, gin::StringToV8(isolate, abbrev));
  FOR_EACH_SCRIPT(V)
#undef V

  v8::EscapableHandleScope handle_scope(isolate);
  auto unicode = v8::Object::New(isolate);

  // Bidi class list
  auto bidi_names = v8::Array::New(isolate, arraysize(kBidiClassNames));
  auto bidi_object = v8::Object::New(isolate);
  for (auto i = 0; i < arraysize(kBidiClassNames); ++i) {
    auto name = gin::StringToV8(isolate, kBidiClassNames[i]);
    bidi_object->Set(name, name);
    bidi_names->Set(static_cast<size_t>(i), name);
  }
  unicode->Set(gin::StringToV8(isolate, "Bidi"), bidi_object);
  unicode->Set(gin::StringToV8(isolate, "BIDI_CLASS_SHORT_NAMES"), bidi_names);

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
  auto script_object = v8::Object::New(isolate);
#define V(id, abbrev, name)                         \
  script_object->Set(gin::StringToV8(isolate, #id), \
                     kScriptAbbrevs[USCRIPT_##id]);
  FOR_EACH_SCRIPT(V)
#undef V
  unicode->Set(gin::StringToV8(isolate, "Script"), script_object);

  // UCD
  auto ucd = v8::Array::New(isolate, 0x10000);
  unicode->Set(gin::StringToV8(isolate, "UCD"), ucd);
  auto name = gin::StringToV8(isolate, "name");
  auto category = gin::StringToV8(isolate, "category");
  auto bidi_class = gin::StringToV8(isolate, "bidi");
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

    // bidi class
    auto const bidi_class_index = u_charDirection(code);
    CHECK(bidi_class_index < arraysize(kBidiClassNames));
    data->ForceSet(bidi_class, bidi_names->Get(bidi_class_index));

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

  v8::Handle<v8::Object> Get(v8::Isolate* isolate) {
    return v8::Local<v8::Object>::New(isolate, *unicode_);
  }

 private:
  std::unique_ptr<v8::UniquePersistent<v8::Object>> unicode_;

  DISALLOW_COPY_AND_ASSIGN(Unicode);
};

}  // namespace

v8::Handle<v8::Object> GetUnicodeObject(v8::Isolate* isolate) {
  CR_DEFINE_STATIC_LOCAL(Unicode, static_unicode, (isolate));
  return static_unicode.Get(isolate);
}

}  // namespace internal
}  // namespace dom
