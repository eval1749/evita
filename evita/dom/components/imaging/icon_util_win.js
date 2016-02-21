// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.provide('imaging');

(function() {
  //////////////////////////////////////////////////////////////////////
  //
  // IconLoader
  //
  class IconLoader {
    /** @param {!WinResource} resource */
    constructor(resource) { this.resource_ = resource; }

    close() { this.resource_.close(); }

    /** @return {!Promise<!Array<string>>} */
    getIconNames() {
      return this.resource_.getResourceNames(WinResource.RT_GROUP_ICON);
    }

    /**
     * @param {string} iconGroupName
     * @param {number} iconSize
     * @return {!Promise<!ImageData>}
     */
    load(iconGroupName, iconSize) {
      const result = (async(function * (loader, iconGroupName, iconSize) {
        const buffer = new Uint8Array(1024 * 32);
        const iconGroupSize = yield loader.resource_.load(
            WinResource.RT_GROUP_ICON, iconGroupName, buffer);
        const iconId =
            WinResource.lookupIcon(buffer.subarray(0, iconGroupSize), iconSize);
        const iconDataSize = yield loader.resource_.load(
            WinResource.RT_ICON, `#${iconId}`, buffer);
        return ImageData.decode(
            `image/ico;size=${iconSize}`, buffer.subarray(0, iconDataSize));
      }))(this, iconGroupName, iconSize);
      return /** @type {!Promise<!ImageData>} */ (result);
    }

    /**
     * @param {string} fileName
     * @return {!Promise<!IconLoader>}
     */
    static open(fileName) {
      return WinResource.open(fileName).then(
          resource => new IconLoader(resource));
    }
  }

  //////////////////////////////////////////////////////////////////////
  //
  // IconUtil
  //
  class IconUtil {
    /**
     * @param {string} fileName
     * @param {string} iconGroupName
     * @param {number=} opt_iconSize
     * @return {!Promise<ImageData>}
     */
    static loadIcon(fileName, iconGroupName, opt_iconSize = 16) {
      const iconSize = /** @type {number} */ (opt_iconSize);
      let maybeLoader = null;
      const result = IconLoader.open(fileName)
                         .then(loader => {
                           maybeLoader = loader;
                           return loader.load(iconGroupName, iconSize);
                         })
                         .then(image => {
                           maybeLoader.close();
                           return image;
                         })
                         .catch(exception => {
                           if (maybeLoader)
                             maybeLoader.close();
                           throw exception;
                         });
      return /** @type {!Promise<ImageData>} */ (result);
    }

    /**
     * @param {string} fileName
     * @param {number} iconGroupIndex
     * @param {number=} opt_iconSize
     * @return {!Promise<ImageData>}
     */
    static loadIconByIndex(fileName, iconGroupIndex, opt_iconSize = 16) {
      const iconSize = /** @type {number} */ (opt_iconSize);
      let maybeLoader = null;
      const result =
          IconLoader.open(fileName)
              .then(loader => {
                maybeLoader = loader;
                return loader.getIconNames();
              })
              .then(names => {
                return maybeLoader.load(names[iconGroupIndex], iconSize);
              })
              .then(image => {
                maybeLoader.close();
                return image;
              })
              .catch(exception => {
                if (maybeLoader)
                  maybeLoader.close();
                throw exception;
              });
      return /** @type {!Promise<ImageData>} */ (result);
    }

    /**
     * @param {string} extension
     * @param {number=} opt_iconSize
     * @return {!Promise<ImageData>}
     */
    static loadIconForExtension(extension, opt_iconSize = 16) {
      const iconSize = /** @type {number} */ (opt_iconSize);
      const typeName =
          WinRegistry.readString(`HKEY_CLASSES_ROOT\\${extension}\\`);
      if (typeName === '')
        return Promise.resolve(/** @type {ImageData} */ (null));
      const iconSpec = WinRegistry.readString(
          `HKEY_CLASSES_ROOT\\${typeName}\\DefaultIcon\\`);
      if (iconSpec === '')
        return Promise.resolve(/** @type {ImageData} */ (null));
      const comma = iconSpec.indexOf(',');
      if (comma < 0)
        return Promise.resolve(/** @type {ImageData} */ (null));
      const iconFile = iconSpec.substr(0, comma);
      const iconIndex = parseInt(iconSpec.substr(comma + 1), 10);
      if (iconIndex < 0)
        return IconUtil.loadIcon(iconFile, `#${-iconIndex}`, iconSize);
      return IconUtil.loadIconByIndex(iconFile, iconIndex, iconSize);
    }
  }

  /** @const */
  imaging.IconUtil = IconUtil;
})();
