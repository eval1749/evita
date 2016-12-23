// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

#include "joana/ir/operator_factory.h"

#include "joana/ir/common_operators.h"

namespace joana {
namespace ir {

constexpr size_t kMaxStringLengthForCache = 10;

//
// OperatorFactory::Cache
//
class OperatorFactory::Cache final {
 public:
  explicit Cache(Zone* zone);
  ~Cache() = default;

  const Operator& GetBoolOperator(bool value) const;
  const Operator& Set(const Operator& op);
  const Operator* TryGet(OperationCode opcode) const;

  // TODO(eval1749): We should share |Cache| implementation with
  // |TypeFactory::Cache| via template.
  template <typename Key>
  const Operator* Find(const Key& key) {
    const auto& map = MapFor(key);
    const auto& it = map.find(key);
    return it == map.end() ? nullptr : it->second;
  }

  template <typename Key>
  const Operator& Register(const Key& key, const Operator& op) {
    auto& map = MapFor(key);
    const auto& result = map.emplace(key, &op);
    DCHECK(result.second);
    return *result.first->second;
  }

 private:
  Zone& zone_;

  std::unordered_map<float64_t, const Operator*>& MapFor(float64_t) {
    return float64_map_;
  }

  std::unordered_map<int64_t, const Operator*>& MapFor(int64_t) {
    return int64_map_;
  }

  std::map<std::pair<OperationCode, size_t>, const Operator*>& MapFor(
      const std::pair<OperationCode, size_t>&) {
    return size_map_;
  }

  std::unordered_map<base::StringPiece16,
                     const Operator*,
                     base::StringPiece16Hash>&
  MapFor(base::StringPiece16) {
    return string_map_;
  }

  const Operator& false_operator_;
  const Operator& true_operator_;
  std::vector<const Operator*> map_;
  std::unordered_map<float64_t, const Operator*> float64_map_;
  std::unordered_map<int64_t, const Operator*> int64_map_;
  std::map<std::pair<OperationCode, size_t>, const Operator*> size_map_;
  std::unordered_map<base::StringPiece16,
                     const Operator*,
                     base::StringPiece16Hash>
      string_map_;

  DISALLOW_COPY_AND_ASSIGN(Cache);
};

OperatorFactory::Cache::Cache(Zone* zone)
    : zone_(*zone),
      false_operator_(*new (&zone_) LiteralBoolOperator(false)),
      true_operator_(*new (&zone_) LiteralBoolOperator(true)),
      map_(kNumberOfOperations) {}

const Operator& OperatorFactory::Cache::GetBoolOperator(bool data) const {
  return data ? true_operator_ : false_operator_;
}

const Operator& OperatorFactory::Cache::Set(const Operator& op) {
  DCHECK_EQ(map_.at(static_cast<size_t>(op.opcode())), nullptr);
  DCHECK_EQ(op.format().number_of_members(), 0u) << op;
  map_[static_cast<size_t>(op.opcode())] = &op;
  return op;
}

const Operator* OperatorFactory::Cache::TryGet(OperationCode opcode) const {
  return map_[static_cast<size_t>(opcode)];
}

//
// OperatorFactory
//
OperatorFactory::OperatorFactory(Zone* zone)
    : zone_(*zone), cache_(new Cache(&zone_)) {}

OperatorFactory::~OperatorFactory() = default;

#define IMPLEMENT_FACTORY_FUNCTION_0(name)                \
  const Operator& OperatorFactory::New##name() {          \
    if (auto* op = cache_->TryGet(OperationCode::##name)) \
      return *op;                                         \
    return cache_->Set(*new (&zone_) name##Operator());   \
  }

#define IMPLEMENT_FACTORY_FUNCTION_1(name, type1)                       \
  const Operator& OperatorFactory::New##name(type1 value1) {            \
    const auto& key = std::make_pair(OperationCode::##name, value1);    \
    if (auto* op = cache_->Find(key))                                   \
      return *op;                                                       \
    return cache_->Register(key, *new (&zone_) name##Operator(value1)); \
  }

IMPLEMENT_FACTORY_FUNCTION_0(Exit)
IMPLEMENT_FACTORY_FUNCTION_0(If)
IMPLEMENT_FACTORY_FUNCTION_0(IfException)
IMPLEMENT_FACTORY_FUNCTION_0(IfFalse)
IMPLEMENT_FACTORY_FUNCTION_0(IfSuccess)
IMPLEMENT_FACTORY_FUNCTION_0(IfTrue)

const Operator& OperatorFactory::NewLiteralBool(bool data) {
  return cache_->GetBoolOperator(data);
}

const Operator& OperatorFactory::NewLiteralFloat64(float64_t data) {
  if (auto* op = cache_->Find(data))
    return *op;
  return cache_->Register(data, *new (&zone_) LiteralFloat64Operator(data));
}

const Operator& OperatorFactory::NewLiteralInt64(int64_t data) {
  if (auto* op = cache_->Find(data))
    return *op;
  return cache_->Register(data, *new (&zone_) LiteralInt64Operator(data));
}

const Operator& OperatorFactory::NewLiteralString(base::StringPiece16 data) {
  if (auto* op = cache_->Find(data))
    return *op;
  return cache_->Register(data, *new (&zone_) LiteralStringOperator(data));
}

IMPLEMENT_FACTORY_FUNCTION_0(LiteralVoid)
IMPLEMENT_FACTORY_FUNCTION_1(Projection, size_t)
IMPLEMENT_FACTORY_FUNCTION_0(Ret)
IMPLEMENT_FACTORY_FUNCTION_0(Start)
IMPLEMENT_FACTORY_FUNCTION_1(Tuple, size_t)

}  // namespace ir
}  // namespace joana
