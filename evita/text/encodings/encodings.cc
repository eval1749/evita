// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/encodings/encodings.h"

#include <memory>
#include <unordered_map>

#include "base/logging.h"
#include "evita/text/encodings/euc_jp_decoder.h"
#include "evita/text/encodings/euc_jp_encoder.h"
#include "evita/text/encodings/shift_jis_decoder.h"
#include "evita/text/encodings/shift_jis_encoder.h"
#include "evita/text/encodings/utf8_decoder.h"
#include "evita/text/encodings/utf8_encoder.h"

namespace encodings {

//////////////////////////////////////////////////////////////////////
//
// Encodings::Priate
//
class Encodings::Private {
  public: typedef Decoder* (*NewDecoder)();
  public: typedef Encoder* (*NewEncoder)();

  public: struct Encoding {
    NewDecoder decoder;
    NewEncoder encoder;
  };

  private: std::unordered_map<base::string16, Encoding*> map_;

  public: Private();
  public: ~Private();

  public: Encoding* GetEncoding(const base::string16& name) const;
  private: void Install(std::vector<const char16*> names,
                        NewDecoder new_decoder, NewEncoder new_encoder);
};

namespace {
#define DEFINE_ENCODING(name) \
  Decoder* New ## name ## Decoder() { return new name ## Decoder; } \
  Encoder* New ## name ## Encoder() { return new name ## Encoder; }

DEFINE_ENCODING(EucJp)
DEFINE_ENCODING(ShiftJis)
DEFINE_ENCODING(Utf8)
}  // namespace

Encodings::Private::Private() {
  Install(std::vector<const char16*> {
      L"cseucpkdfmtjapanese",
      L"euc-jp",
      L"x-euc-jp"
  }, &NewEucJpDecoder, &NewEucJpEncoder);

  Install(std::vector<const char16*> {
      L"csshiftjis",
      L"ms_kanji",
      L"shift-jis",
      L"shift_jis",
      L"sjis",
      L"windows-31j",
      L"x-sjis"
  }, &NewShiftJisDecoder, &NewShiftJisEncoder);

  Install(std::vector<const char16*> {
      L"unicode-1-1-utf-8",
      L"utf-8",
      L"utf8",
  }, &NewUtf8Decoder, &NewUtf8Encoder);
}

Encodings::Private::~Private() {
}

Encodings::Private::Encoding* Encodings::Private::GetEncoding(
    const base::string16& name) const {
  base::string16 key(name.size(), 0);
  for (auto index = 0u; index < name.size(); ++index) {
    key[index] = name[index] >= 'A' && name[index] <= 'Z' ?
        static_cast<base::char16>(name[index] - 'A' + 'a') : name[index];
  }
  auto const it = map_.find(key);
  return it == map_.end() ? nullptr : it->second;
}

void Encodings::Private::Install(std::vector<const char16*> names,
                                 NewDecoder new_decoder,
                                 NewEncoder new_encoder) {

  auto const encoding = new Encoding();
  encoding->decoder = new_decoder;
  encoding->encoder = new_encoder;

  for (auto name : names) {
    map_[name] = encoding;
  }
}

//////////////////////////////////////////////////////////////////////
//
// Encodings
//
Encodings::Encodings() : private_(new Private()) {
}

Encodings::~Encodings() {
}

Decoder* Encodings::GetDecoder(const base::string16& name) const {
  auto const encoding = private_->GetEncoding(name);
  return encoding ? encoding->decoder() : nullptr;
}

Encoder* Encodings::GetEncoder(const base::string16& name) const {
  auto const encoding = private_->GetEncoding(name);
  return encoding ? encoding->encoder() : nullptr;
}

}  // namespace encodings
