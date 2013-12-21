# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'evita_code': 1,
  }, # variables

  'includes': [
    'evita.gypi',
    'evita_test.gypi',
  ], # includes

  'target_defaults': {
    # Precompiled header
    # See gyp/pylib/gyp/msvs_settings.py for details
    'msvs_precompiled_header': 'precomp.h',
    'msvs_precompiled_source': 'precomp.cpp',
  }, # target_defaults
}
