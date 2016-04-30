// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/base/resource/data_pack.h"

#include <algorithm>
#include <iterator>

#include "base/files/file.h"
#include "base/files/memory_mapped_file.h"
#include "base/logging.h"

namespace base {

namespace {

// See https://chromium.googlesource.com/chromium/src/+/master/ui/base/resource/data_pack.cc
// See http://dev.chromium.org/developers/design-documents/linuxresourcesandlocalizedstrings

// Format of Data Pack file
//
//      +-----------------------+
//  +0  | version(=4)           |
//      +-----------------------+
//  +4  | number of entries(=N) |
//      +-----------------------+
//  +8  | encoding type         | 0=binary, 1=UTF8, 2=UTF16
//      +-----------------------+
//  +9  | DataPackEntry[0]      |
//      +-----------------------+
//  +15 | DataPackEntry[1]      |
//      +-----------------------+
//       ....
//      +-----------------------+
//  ... | DataPackEntry[N-1]    |
//      +-----------------------+
//  ... | DataPackEntry[N]      | sentinel to provide length of last entry
//      +-----------------------+
//      | ... data ...          |
//

const uint32_t kFileFormatVersion = 4;
// Length of file header: version, entry count and text encoding type.
const size_t kHeaderLength = 2 * sizeof(uint32_t) + sizeof(uint8_t);

#pragma pack(push, 2)
struct DataPackEntry {
  uint16_t resource_id;
  uint32_t file_offset;
};
#pragma pack(pop)

static_assert(sizeof(DataPackEntry) == 6, "size of entry must be six");

class DataPackEntryCollection {
 public:
  class Iterator
      : public std::iterator<std::forward_iterator_tag, const DataPackEntry> {
   public:
    Iterator(const DataPackEntryCollection* collection,
             const DataPackEntry* current)
        : collection_(collection), current_(current) {}
    Iterator(const Iterator& other)
        : Iterator(other.collection_, other.current_) {}
    ~Iterator() = default;

    pointer operator->() const {
      // |collection_->end_| is a sentinel to provide length of last entry.
      DCHECK_LE(current_, collection_->end_);
      return current_;
    }

    reference operator*() const {
      // |collection_->end_| is a sentinel to provide length of last entry.
      DCHECK_LE(current_, collection_->end_);
      return *current_;
    }

    Iterator& operator++() {
      DCHECK_LT(current_, collection_->end_);
      ++current_;
      return *this;
    }

    bool operator==(const Iterator& other) const {
      DCHECK_EQ(collection_, other.collection_);
      return current_ == other.current_;
    }

    bool operator!=(const Iterator& other) const { return !operator==(other); }

   private:
    const DataPackEntryCollection* collection_;
    const DataPackEntry* current_;
  };

  DataPackEntryCollection(const uint8_t* start, size_t resource_count);
  ~DataPackEntryCollection() = default;

  Iterator begin() const { return Iterator(this, begin_); }
  Iterator end() const { return Iterator(this, end_); }

  base::StringPiece GetStringPiece(uint16_t resource_id) const;

 private:
  const DataPackEntry* const begin_;
  const DataPackEntry* const end_;
  const uint8_t* const start_;

  DISALLOW_COPY_AND_ASSIGN(DataPackEntryCollection);
};

DataPackEntryCollection::DataPackEntryCollection(const uint8_t* start,
                                                 size_t resource_count)
    : begin_(reinterpret_cast<const DataPackEntry*>(start + kHeaderLength)),
      end_(begin_ + resource_count),
      start_(start) {}

base::StringPiece DataPackEntryCollection::GetStringPiece(
    uint16_t resource_id) const {
  const auto& it =
      std::lower_bound(begin(), end(), resource_id,
                       [](const DataPackEntry& entry, uint16_t resource_id) {
                         return entry.resource_id < resource_id;
                       });
  if (it == end() || it->resource_id != resource_id)
    return base::StringPiece();
  return base::StringPiece(
      reinterpret_cast<const char*>(start_ + it->file_offset),
      std::next(it)->file_offset - it->file_offset);
}

}  // namespace

DataPack::DataPack() {}
DataPack::~DataPack() {}

base::StringPiece DataPack::GetStringPiece(uint16_t resource_id) const {
  if (!mmap_)
    return base::StringPiece();
  DataPackEntryCollection collection(mmap_->data(), resource_count_);
  return collection.GetStringPiece(resource_id);
}

bool DataPack::LoadFromFile(base::File file) {
  auto mmap = std::make_unique<base::MemoryMappedFile>();
  if (!mmap->Initialize(std::move(file),
                        base::MemoryMappedFile::Region::kWholeFile)) {
    DLOG(ERROR) << "Failed to mmap data pack";
    return false;
  }
  return LoadInternal(std::move(mmap));
}

bool DataPack::LoadFromPath(const base::FilePath& path) {
  auto mmap = std::make_unique<base::MemoryMappedFile>();
  if (!mmap->Initialize(path)) {
    DLOG(ERROR) << "Failed to mmap data pack";
    return false;
  }
  return LoadInternal(std::move(mmap));
}

bool DataPack::LoadInternal(std::unique_ptr<base::MemoryMappedFile> mmap) {
  if (mmap->length() < kHeaderLength) {
    DLOG(ERROR) << "Data pack file corruption: incomplete file header";
    return false;
  }

  // Parse the header of the file.
  // First uint32_t: version; second: resource count;
  const uint32_t* ptr = reinterpret_cast<const uint32_t*>(mmap->data());
  uint32_t version = ptr[0];
  if (version != kFileFormatVersion) {
    LOG(ERROR) << "Bad data pack version: got " << version << ", expected "
               << kFileFormatVersion;
    return false;
  }
  resource_count_ = ptr[1];

  // Sanity check the file.
  // 1) Check we have enough entries. There's an extra entry after the last item
  // which gives the length of the last item.
  if (kHeaderLength + (resource_count_ + 1) * sizeof(DataPackEntry) >
      mmap->length()) {
    LOG(ERROR) << "Data pack file corruption: too short for number of "
                  "entries specified.";
    mmap.reset();
    return false;
  }
  // 2) Verify the entries are within the appropriate bounds. There's an extra
  // entry after the last item which gives us the length of the last item.
  for (size_t i = 0; i < resource_count_ + 1; ++i) {
    const DataPackEntry* entry = reinterpret_cast<const DataPackEntry*>(
        mmap->data() + kHeaderLength + (i * sizeof(DataPackEntry)));
    if (entry->file_offset > mmap->length()) {
      LOG(ERROR) << "Entry #" << i << " in data pack points off end of file. "
                 << "Was the file corrupted?";
      return false;
    }
  }

  mmap_ = std::move(mmap);
  return true;
}

}  // namespace base
