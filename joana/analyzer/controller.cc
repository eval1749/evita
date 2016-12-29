// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/controller.h"

#include "joana/analyzer/context.h"

namespace joana {
namespace analyzer {

Controller::Controller(const AnalyzerSettings& settings)
    : context_(new Context(settings)) {}

Controller::~Controller() = default;

void Controller::Analyze() {}

void Controller::Load(const ast::Node& node) {}

}  // namespace analyzer
}  // namespace joana
