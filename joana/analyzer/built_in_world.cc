// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/built_in_world.h"

#include "base/memory/singleton.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/node_factory.h"
#include "joana/base/memory/zone.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_factory.h"
#include "joana/base/source_code_range.h"

namespace joana {
namespace analyzer {

namespace {

const SourceCode& NewSourceCodeForBuildIn(Zone* zone) {
  static const char* const kSourceCode = "(global)";
  SourceCode::Factory source_code_factory(zone);
  const auto& text16 = base::UTF8ToUTF16(kSourceCode);
  const auto size = text16.size() * sizeof(base::char16);
  auto* data = static_cast<base::char16*>(zone->Allocate(size));
  ::memcpy(data, text16.data(), size);
  return source_code_factory.New(base::FilePath(),
                                 base::StringPiece16(data, text16.size()));
}

}  // namespace

//
// BuiltInWorld::Private
//
class BuiltInWorld::Private final {
 public:
  Private();
  ~Private() = default;

  ast::NodeFactory& node_factory() { return node_factory_; }
  const SourceCode& source_code() const { return source_code_; }

 private:
  Zone zone_;

  // |ast::NodeFactory| constructor takes |zone_|.
  ast::NodeFactory node_factory_;
  const SourceCode& source_code_;

  DISALLOW_COPY_AND_ASSIGN(Private);
};

BuiltInWorld::Private::Private()
    : zone_("BuiltInWorld"),
      node_factory_(&zone_),
      source_code_(NewSourceCodeForBuildIn(&zone_)) {}

//
// BuiltInWorld
//
BuiltInWorld::BuiltInWorld()
    : private_(std::make_unique<Private>()),
      global_module_(private_->node_factory().NewModule(
          SourceCodeRange::CollapseToStart(private_->source_code().range()),
          {})) {}

BuiltInWorld::~BuiltInWorld() = default;

// static
BuiltInWorld* BuiltInWorld::GetInstance() {
  return base::Singleton<BuiltInWorld>::get();
}

}  // namespace analyzer
}  // namespace joana
