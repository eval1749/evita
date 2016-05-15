// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/view/public/user_action_source.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// UserActionSource::Observer
//
UserActionSource::Observer::Observer() = default;
UserActionSource::Observer::~Observer() = default;

//////////////////////////////////////////////////////////////////////
//
// UserActionSource
//
UserActionSource::UserActionSource() = default;
UserActionSource::~UserActionSource() = default;

void UserActionSource::AddObserver(Observer* observer) const {}
void UserActionSource::RemoveObserver(Observer* observer) const {}

}  // namespace visuals
