// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

// Provide funciton names are as same as FireFox OS.Path.
(function() {

  /** @const */ var CURRENT_DIRECTORY = '.';
  /** @const */ var EMPTY_STRING = '';
  /** @const */ var PARENT_DIRECTORY = '..';
  /** @const */ var PATH_SEPARATOR = '\\';
  /** @const */ var RE_BASENAME = NewRegExp('^(.*)[/\\\\]([^/\\\\]+)$');
  /** @const */ var RE_FILENAME_RESERVED_CHARACTERS = NewRegExp('[<>:"|?*]');
  /** @const */ var RE_PATHNAME = NewRegExp('^([^:]+:)?([/\\\\])?(.*)$');
  /** @const */ var RE_COMPONENT_SEPARATOR = NewRegExp('[/\\\\]');

  /**
   * @param {string} fileName
   * @return {string} A fileName part of pathname.
   */
  FilePath.basename = function(fileName) {
    var matches = fileName.match(RE_BASENAME);
    return matches ? matches[2] : fileName;
  };

  /**
   * @param {string} fileName
   * @return {string} the directory part of fileName.
   */
  FilePath.dirname = function(fileName) {
    var matches = fileName.match(RE_BASENAME);
    return matches ? matches[1] : fileName;
  };

  /**
   * @param {string} fileName
   * @return {boolean}
   */
  FilePath.isValidFileName = function(fileName) {
    var data = FilePath.split(fileName);
    return !!data.winDrive && data.absolute &&
           data.components.every(function(component) {
             return !RE_FILENAME_RESERVED_CHARACTERS.test(component);
           });
  };

  /**
   * @param {...string} var_components
   * @return {string}
   */
  FilePath.join = function(var_components) {
    return Array.prototype.slice.call(arguments, 0).join(PATH_SEPARATOR);
  };

  /**
   * @param {string} fileName
   * @return {string}
   */
  FilePath.normalize = function(fileName) {
    var data = FilePath.split(fileName);
    var work = data.components;
    var components = [];
    var parents = [];
    while (work.length) {
      var component = work.shift();
      if (component == CURRENT_DIRECTORY)
        continue;
      if (component == PARENT_DIRECTORY) {
        if (components.length)
          components.pop();
        else
          parents.push(component);
      } else if (component != '' || !work.length) {
        components.push(component);
      }
    }
    return (data.winDrive ? data.winDrive.toUpperCase() + ':' :
                            EMPTY_STRING) +
           (data.absolute ? PATH_SEPARATOR : EMPTY_STRING) +
           components.join(PATH_SEPARATOR);
  };

  /**
   * @param {string} fileName
   * @return {{absolute: boolean, components: Array.<string>,
   *           winDrive: ?string}}
   */
  FilePath.split = function(fileName) {
    var matches = fileName.match(RE_PATHNAME);
    var drive = matches[1];
    return {
      absolute: !!matches[2],
      components: matches[3].split(RE_COMPONENT_SEPARATOR),
      winDrive: drive ? drive.substr(0, drive.length - 1) : null
    };
  };
})();
