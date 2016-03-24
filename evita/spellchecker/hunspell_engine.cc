// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include <string>

#include "evita/spellchecker/hunspell_engine.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/synchronization/lock.h"
#include "common/win/scoped_handle.h"
#include "third_party/hunspell/src/hunspell/hunspell.hxx"

namespace spellchecker {

namespace {
// Maximum length of words we actually check.
// 64 is the observed limits for OSX system checker.
const size_t kMaxCheckedLen = 64;

// Maximum length of words we provide suggestions for.
// 24 is the observed limits for OSX system checker.
const size_t kMaxSuggestLen = 24;

static_assert(kMaxCheckedLen <= size_t(MAXWORDLEN),
              "MaxCheckedLen too long");
static_assert(kMaxSuggestLen <= kMaxCheckedLen,
              "MaxSuggestLen too long");

// Max number of dictionary suggestions.
const int kMaxSuggestions = 5;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// HunspellEngine::Dictionary
//
class HunspellEngine::Dictionary final {
 public:
  Dictionary();
  ~Dictionary();

  bool is_valid() const { return is_valid_; }
  const uint8_t* data() const { return data_.data(); }
  size_t size() const { return data_.size(); }

 private:
  static base::string16 GetFileName();

  bool is_valid_;
  std::vector<uint8_t> data_;

  DISALLOW_COPY_AND_ASSIGN(Dictionary);
};

HunspellEngine::Dictionary::Dictionary() : is_valid_(false) {
  auto const file_name = GetFileName();
  common::win::scoped_handle file(
      ::CreateFileW(file_name.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                    OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr));
  if (!file) {
    PLOG(ERROR) << "CreateFile " << file_name << " failed";
    return;
  }
  BY_HANDLE_FILE_INFORMATION info;
  if (!::GetFileInformationByHandle(file.get(), &info)) {
    PLOG(ERROR) << "GetFileInformationByHandle failed";
    return;
  }
  DCHECK(!info.nFileSizeHigh);
  data_.resize(info.nFileSizeLow);
  DWORD read;
  if (!::ReadFile(file.get(), &data_[0], static_cast<DWORD>(data_.size()),
                  &read, nullptr)) {
    PLOG(ERROR) << "ReadFile failed";
    return;
  }
  is_valid_ = true;
}

HunspellEngine::Dictionary::~Dictionary() {}

base::string16 HunspellEngine::Dictionary::GetFileName() {
  base::string16 file_name(MAX_PATH, '?');
  auto length_with_zero = ::ExpandEnvironmentStrings(
      L"%LOCALAPPDATA%/Google/Chrome/User Data/en-US-6-1.bdic", &file_name[0],
      static_cast<DWORD>(file_name.size()));
  file_name.resize(length_with_zero - 1);
  return file_name;
}

//////////////////////////////////////////////////////////////////////
//
// HunspellEngine
//
HunspellEngine::HunspellEngine() : lock_(new base::Lock()) {}

HunspellEngine::~HunspellEngine() {}

// spellchecker::SpellingEngine
bool HunspellEngine::CheckSpelling(const base::string16& word_to_check) {
  DCHECK(!word_to_check.empty());
  if (!EnsureInitialized())
    return true;
  std::string utf8_word_to_check(base::UTF16ToUTF8(word_to_check));
  if (utf8_word_to_check.size() > kMaxCheckedLen)
    return true;
  return hunspell_->spell(utf8_word_to_check.c_str()) != 0;
}

bool HunspellEngine::EnsureInitialized() {
  if (hunspell_)
    return true;
  base::AutoLock lock_scope(*lock_);
  if (dictionary_)
    return dictionary_->is_valid();
  dictionary_.reset(new Dictionary());
  if (!dictionary_->is_valid())
    return false;
  hunspell_.reset(new Hunspell(dictionary_->data(), dictionary_->size()));
  return true;
}

std::vector<base::string16> HunspellEngine::GetSpellingSuggestions(
    const base::string16& wrong_word) {
  DCHECK(!wrong_word.empty());
  if (!EnsureInitialized())
    return std::vector<base::string16>();
  std::string utf8_wrong_word(base::UTF16ToUTF8(wrong_word));
  char** utf8_suggestions = nullptr;
  auto const num_suggestions =
      hunspell_->suggest(&utf8_suggestions, utf8_wrong_word.c_str());
  std::vector<base::string16> suggestions;
  for (auto i = 0; i < num_suggestions; ++i) {
    auto const utf8_suggestion = utf8_suggestions[i];
    if (i < kMaxSuggestions)
      suggestions.push_back(base::UTF8ToUTF16(utf8_suggestion));
    ::free(utf8_suggestion);
  }
  if (utf8_suggestions)
    ::free(utf8_suggestions);
  return std::move(suggestions);
}

}  // namespace spellchecker
