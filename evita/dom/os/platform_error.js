// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//////////////////////////////////////////////////////////////////////
//
// PlatformError
//
class PlatformError extends Error {
  /**
   * @param {string} apiName
   * @param {number} errorCode
   */
  constructor(apiName, errorCode) {
    super(`Platform API ${apiName} failed with ${errorCode}`);
    this.apiName_ = apiName;
    this.errorCode_ = errorCode;
  }

  /** @return {string} */
  get apiName() { return this.apiName_; }

  /** @return {number} */
  get errorCode() { return this.errorCode_; }
}
