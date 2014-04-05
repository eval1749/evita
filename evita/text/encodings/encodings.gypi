# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'encodings',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
      ],
      'msvs_precompiled_header': '',
      'msvs_precompiled_source': '',
      'sources': [
        'decoder.cc',
        'decoder.h',
        'encoder.cc',
        'encoder.h',
        'encodings.cc',
        'encodings.h',
        'euc_jp_decoder.cc',
        'euc_jp_decoder.h',
        'shift_jis_decoder.cc',
        'shift_jis_decoder.h',
        'utf8_decoder.cc',
        'utf8_decoder.h',
        'utf8_encoder.cc',
        'utf8_encoder.h',
      ], # sources
    },
  ] # targets
}
