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
  /** @const */ var RE_BASENAME = new RegExp('^(.*)[/\\\\]([^/\\\\]+)$');
  /** @const */ var RE_FILENAME_RESERVED_CHARACTERS = new RegExp('[<>:"|?*]');
  /** @const */ var RE_PATHNAME = new RegExp('^([^:]+:)?([/\\\\])?(.*)$');
  /** @const */ var RE_COMPONENT_SEPARATOR = new RegExp('[/\\\\]');

  /**
   * @param {string} filename
   * @return {string} A filename part of pathname.
   */
  FilePath.basename = function(filename) {
    var matches = filename.match(RE_BASENAME);
    return matches ? matches[2] : filename;
  };

  /**
   * @param {string} filename
   * @return {string} the directory part of filename.
   */
  FilePath.dirname = function(filename) {
    var matches = filename.match(RE_BASENAME);
    return matches ? matches[1] : filename;
  };

  /**
   * @param {string} filename
   * @return {boolean}
   */
  FilePath.isValidFilename = function(filename) {
    var data = FilePath.split(filename);
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
   * @param {string} filename
   * @return {string}
   */
  FilePath.normalize = function(filename) {
    var data = FilePath.split(filename);
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
   * @param {string} filename
   * @return {{absolute: boolean, components: Array.<string>,
   *           winDrive: ?string}}
   */
  FilePath.split = function(filename) {
    var matches = filename.match(RE_PATHNAME);
    var drive = matches[1];
    return {
      absolute: !!matches[2],
      components: matches[3].split(RE_COMPONENT_SEPARATOR),
      winDrive: drive ? drive.substr(0, drive.length - 1) : null
    };
  };
})();
