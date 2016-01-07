// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_BASE_TABLE_MODEL_OBSERVER_H_
#define EVITA_UI_BASE_TABLE_MODEL_OBSERVER_H_

#include "base/macros.h"
#include "base/strings/string16.h"

namespace ui {

class TableModelObserver {
 public:
  virtual ~TableModelObserver();

  virtual void DidAddRow(int row_id) = 0;
  virtual void DidChangeRow(int row_id) = 0;
  virtual void DidRemoveRow(int row_id) = 0;

 protected:
  TableModelObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(TableModelObserver);
};

}  // namespace ui

#endif  // EVITA_UI_BASE_TABLE_MODEL_OBSERVER_H_
