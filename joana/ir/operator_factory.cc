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

  const Operator* Find(OperationCode opcode) const;
  const Operator* Find(OperationCode opcode, size_t value) const;

  const Operator* FindByFloat64(float64_t data) const;
  const Operator* FindByInt64(int64_t data) const;
  const Operator* FindByString(base::StringPiece16 data) const;

  const Operator& GetBoolOperator(bool data) const;

  const Operator* Set(const Operator* op);
  const Operator* Set(const Operator* op, size_t value);

  const Operator* SetWithFloat64(const Operator* op, float64_t data);
  const Operator* SetWithInt64(const Operator* op, int64_t data);
  const Operator* SetWithString(const Operator* op, base::StringPiece16 data);

 private:
  Zone& zone_;

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

const Operator* OperatorFactory::Cache::Find(OperationCode opcode) const {
  return map_[static_cast<size_t>(opcode)];
}

const Operator* OperatorFactory::Cache::Find(OperationCode opcode,
                                             size_t value) const {
  const auto& it = size_map_.find(std::make_pair(opcode, value));
  return it == size_map_.end() ? nullptr : it->second;
}

const Operator* OperatorFactory::Cache::FindByFloat64(float64_t data) const {
  const auto& it = float64_map_.find(data);
  return it == float64_map_.end() ? nullptr : it->second;
}

const Operator* OperatorFactory::Cache::FindByInt64(int64_t data) const {
  const auto& it = int64_map_.find(data);
  return it == int64_map_.end() ? nullptr : it->second;
}

const Operator* OperatorFactory::Cache::FindByString(
    base::StringPiece16 data) const {
  if (data.size() > kMaxStringLengthForCache)
    return nullptr;
  const auto& it = string_map_.find(data);
  return it == string_map_.end() ? nullptr : it->second;
}

const Operator& OperatorFactory::Cache::GetBoolOperator(bool data) const {
  return data ? true_operator_ : false_operator_;
}

const Operator* OperatorFactory::Cache::Set(const Operator* op) {
  DCHECK_EQ(map_.at(static_cast<size_t>(op->opcode())), nullptr);
  DCHECK_EQ(op->format().number_of_members(), 0u) << op;
  map_[static_cast<size_t>(op->opcode())] = op;
  return op;
}

const Operator* OperatorFactory::Cache::Set(const Operator* op, size_t value) {
  const auto& result =
      size_map_.emplace(std::make_pair(op->opcode(), value), op);
  DCHECK(result.second);
  return result.first->second;
}

const Operator* OperatorFactory::Cache::SetWithFloat64(const Operator* op,
                                                       float64_t data) {
  const auto& result = float64_map_.emplace(data, op);
  DCHECK(result.second);
  return result.first->second;
}

const Operator* OperatorFactory::Cache::SetWithInt64(const Operator* op,
                                                     int64_t data) {
  const auto& result = int64_map_.emplace(data, op);
  DCHECK(result.second);
  return result.first->second;
}

const Operator* OperatorFactory::Cache::SetWithString(
    const Operator* op,
    base::StringPiece16 data) {
  const auto& result = string_map_.emplace(data, op);
  DCHECK(result.second);
  return result.first->second;
}

//
// OperatorFactory
//
OperatorFactory::OperatorFactory(Zone* zone)
    : zone_(*zone), cache_(new Cache(&zone_)) {}

OperatorFactory::~OperatorFactory() = default;

#define IMPLEMENT_FACTORY_FUNCTION_0(name)              \
  const Operator& OperatorFactory::New##name() {        \
    if (auto* op = cache_->Find(OperationCode::##name)) \
      return *op;                                       \
    return *cache_->Set(new (&zone_) name##Operator()); \
  }

#define IMPLEMENT_FACTORY_FUNCTION_1(name, type1)                     \
  const Operator& OperatorFactory::New##name(type1 value1) {          \
    if (auto* op = cache_->Find(OperationCode::##name, value1))       \
      return *op;                                                     \
    return *cache_->Set(new (&zone_) name##Operator(value1), value1); \
  }

IMPLEMENT_FACTORY_FUNCTION_0(Exit)

const Operator& OperatorFactory::NewLiteralBool(bool data) {
  return cache_->GetBoolOperator(data);
}

const Operator& OperatorFactory::NewLiteralFloat64(float64_t data) {
  if (auto* op = cache_->FindByFloat64(data))
    return *op;
  return *cache_->SetWithFloat64(new (&zone_) LiteralFloat64Operator(data),
                                 data);
}

const Operator& OperatorFactory::NewLiteralInt64(int64_t data) {
  if (auto* op = cache_->FindByInt64(data))
    return *op;
  return *cache_->SetWithInt64(new (&zone_) LiteralInt64Operator(data), data);
}

const Operator& OperatorFactory::NewLiteralString(base::StringPiece16 data) {
  if (auto* op = cache_->FindByString(data))
    return *op;
  return *cache_->SetWithString(new (&zone_) LiteralStringOperator(data), data);
}

IMPLEMENT_FACTORY_FUNCTION_0(LiteralVoid)
IMPLEMENT_FACTORY_FUNCTION_1(Projection, size_t)
IMPLEMENT_FACTORY_FUNCTION_0(Ret)
IMPLEMENT_FACTORY_FUNCTION_0(Start)
IMPLEMENT_FACTORY_FUNCTION_1(Tuple, size_t)

}  // namespace ir
}  // namespace joana
