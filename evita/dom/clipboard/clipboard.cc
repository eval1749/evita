// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include <unordered_map>

#include "evita/dom/clipboard/clipboard.h"

#include "base/logging.h"
#include "common/memory/singleton.h"
#include "evita/dom/clipboard/data_transfer_data.h"
#include "evita/dom/script_host.h"

namespace dom {

namespace {

//////////////////////////////////////////////////////////////////////
//
// ScopedGlobalAlloc
//
class ScopedGlobalAlloc {
  private: HANDLE handle_;

  public: ScopedGlobalAlloc(size_t size)
      : handle_(::GlobalAlloc(GMEM_MOVEABLE, size)) {
  }
  public: ~ScopedGlobalAlloc() {
    if (!handle_)
      return;
    ::GlobalFree(handle_);
  }

  public: operator HANDLE() const { return handle_; }

  public: HANDLE release() {
    auto const handle = handle_;
    handle_ = nullptr;
    return handle;
  }

  DISALLOW_COPY_AND_ASSIGN(ScopedGlobalAlloc);
};

//////////////////////////////////////////////////////////////////////
//
// ScopedGlobalLock
//
template<typename T>
class ScopedGlobalLock {
  private: HANDLE handle_;

  public: ScopedGlobalLock(HANDLE handle) : handle_(handle) {
  }
  public: ~ScopedGlobalLock() {
    if (!handle_)
      return;
    ::GlobalUnlock(handle_);
  }

  public: T* bytes() const {
    return handle_ ? reinterpret_cast<T*>(::GlobalLock(handle_)) : nullptr;
  }

  public: size_t num_bytes() const {
    return handle_ ? ::GlobalSize(handle_) : 0u;
  }

  DISALLOW_COPY_AND_ASSIGN(ScopedGlobalLock);
};

//////////////////////////////////////////////////////////////////////
//
// FormatMap
//
class FormatMap : public common::Singleton<FormatMap> {
  DECLARE_SINGLETON_CLASS(FormatMap);

  private: std::unordered_map<uint32_t, Clipboard::Format*> code_map_;
  private: std::unordered_map<base::string16, Clipboard::Format*>
      mime_type_map_;

  private: FormatMap();
  public: ~FormatMap();

  public: Clipboard::Format* Get(const base::string16& mime_type) const;
  public: Clipboard::Format* Get(uint32_t format_code) const;
  public: void Register(Clipboard::Format* format);
};

FormatMap::FormatMap() {
}

FormatMap::~FormatMap() {
}

Clipboard::Format* FormatMap::Get(const base::string16& mime_type) const {
  auto it = mime_type_map_.find(mime_type);
  return it == mime_type_map_.end() ? nullptr : it->second;
}

Clipboard::Format* FormatMap::Get(uint32_t format_code) const {
  auto it = code_map_.find(format_code);
  return it == code_map_.end() ? nullptr : it->second;
}

void FormatMap::Register(Clipboard::Format* format) {
  code_map_[format->format()] = format;
  mime_type_map_[format->mime_type()] = format;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Clipboard::Format
//
Clipboard::Format::Format(const base::char16* name,
                                     const base::string16& mime_type)
    : Format(::RegisterClipboardFormatW(name), mime_type) {
}

Clipboard::Format::Format(uint32_t format,
                                     const base::string16& mime_type)
    : format_(format), mime_type_(mime_type) {
  FormatMap::instance()->Register(this);
}

Clipboard::Format::~Format() {
}

const Clipboard::Format* Clipboard::Format::Get(
    const base::string16& mime_type) {
  return FormatMap::instance()->Get(mime_type);
}

const Clipboard::Format* Clipboard::Format::Get(uint32_t format) {
  return FormatMap::instance()->Get(format);
}

//////////////////////////////////////////////////////////////////////
//
// FormatText
//
class TextFormat : public Clipboard::Format {
  public: TextFormat();
  public: virtual ~TextFormat();

  // Clipboard::Format
  private: virtual DataTransferData* FromClipboard(
      HANDLE handle) const override;

  DISALLOW_COPY_AND_ASSIGN(TextFormat);
};

TextFormat::TextFormat() : Format(CF_UNICODETEXT, L"text/plain") {
}

TextFormat::~TextFormat() {
}

DataTransferData* TextFormat::FromClipboard(HANDLE handle) const {
  ScopedGlobalLock<uint8_t> lock_scope(handle);
  auto const bytes = lock_scope.bytes();
  auto num_bytes = lock_scope.num_bytes();
  if (!bytes || num_bytes & 1)
    return nullptr;
  // Since, "cmd.exe" puts text with "\0\a\0" into clipboard, we ignore
  // characters after '\0'.
  auto const chars = reinterpret_cast<const base::char16*>(bytes);
  auto const num_chars = num_bytes / 2;
  auto const end_of_chars = chars + num_chars;
  auto runner = chars;
  while (runner < end_of_chars) {
    if(!*runner)
      break;
    ++runner;
  }
  base::string16 string(chars, static_cast<size_t>(runner - chars));
  return new DataTransferStringData(string);
}

//////////////////////////////////////////////////////////////////////
//
// Clipboard
//
Clipboard::Clipboard() : opened_(::OpenClipboard(nullptr) != FALSE) {
  if (!opened_)
    ScriptHost::instance()->PlatformError("OpenClipboard");

  static bool init_format;
  if (init_format)
    return;

  new TextFormat();
#if 0
  // MS CF_HTML
  new Clipboard::Format(L"HTML Format", L"text/plain");
  // FireFox text/html
  new Clipboard::Format(L"text/html", L"text/html");
#endif
}

Clipboard::~Clipboard() {
  if (!opened_)
    return;
  ::CloseClipboard();
}

void Clipboard::Add(const Format* format, const DataTransferData* data) {
  // For CF_UNICODETEXT, we need to add trailing NUL character.
  auto const num_extra_bytes =
      data->kind() == DataTransferData::Kind::String ? 2 : 0;
  ScopedGlobalAlloc handle(data->num_bytes() + num_extra_bytes);
  {
    ScopedGlobalLock<uint8_t> storage(handle);
    ::memcpy(storage.bytes(), data->bytes(), data->num_bytes());
    if (data->kind() == DataTransferData::Kind::String) {
      storage.bytes()[data->num_bytes()] = 0;
      storage.bytes()[data->num_bytes() + 1] = 0;
    }
  }
  if (!::SetClipboardData(format->format(), handle)) {
    ScriptHost::instance()->PlatformError("SetClipboardData");
    return;
  }
  handle.release();
}

void Clipboard::Clear() {
  DCHECK(opened_);
  ::EmptyClipboard();
}

DataTransferData* Clipboard::Get(const Format* format) const {
  auto const handle = ::GetClipboardData(format->format());
  return handle ? format->FromClipboard(handle) : nullptr;
}

}  // namespace dom
