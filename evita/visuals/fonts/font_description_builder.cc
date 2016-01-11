// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <memory>
#include <tuple>

#include "evita/visuals/fonts/font_description_builder.h"

#include "base/logging.h"
#include "base/memory/singleton.h"

namespace visuals {

using Builder = FontDescription::Builder;

namespace {

//////////////////////////////////////////////////////////////////////
//
// Cache
//
class Cache final {
 public:
  using Key =
      std::tuple<base::string16, float, FontStretch, FontStyle, FontWeight>;

  Cache() = default;
  ~Cache() = default;

  FontDescription* Find(const FontDescription& sample) const;
  const FontDescription& Register(
      std::unique_ptr<FontDescription> new_description);

  static Cache* GetInstance();

 private:
  std::map<Key, std::unique_ptr<FontDescription>> map_;

  DISALLOW_COPY_AND_ASSIGN(Cache);
};

FontDescription* Cache::Find(const FontDescription& sample) const {
  const auto& it = map_.find(std::make_tuple(sample.family(), sample.size(),
                                             sample.stretch(), sample.style(),
                                             sample.weight()));
  return it == map_.end() ? nullptr : it->second.get();
}

const FontDescription& Cache::Register(
    std::unique_ptr<FontDescription> new_description) {
  const auto& result = map_.emplace(
      std::make_tuple(new_description->family(), new_description->size(),
                      new_description->stretch(), new_description->style(),
                      new_description->weight()),
      std::move(new_description));
  DCHECK(result.second) << "Duplicated entry " << *new_description;
  return *result.first->second;
}

// static
Cache* Cache::GetInstance() {
  return base::Singleton<Cache>::get();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FontDescription::Builder
//
Builder::Builder() {}
Builder::~Builder() {}

const FontDescription& Builder::Build() {
  if (const auto present = Cache::GetInstance()->Find(description_))
    return *present;
  auto description = std::unique_ptr<FontDescription>(
      new FontDescription(*this, description_));
  return Cache::GetInstance()->Register(std::move(description));
}

Builder& Builder::SetFamily(const base::string16& family) {
  description_.family_ = family;
  return *this;
}

Builder& Builder::SetSize(float size) {
  description_.size_ = size;
  return *this;
}

Builder& Builder::SetStretch(FontStretch stretch) {
  description_.stretch_ = stretch;
  return *this;
}

Builder& Builder::SetStyle(FontStyle style) {
  description_.style_ = style;
  return *this;
}

Builder& Builder::SetWeight(FontWeight weight) {
  description_.weight_ = weight;
  return *this;
}

}  // namespace visuals
