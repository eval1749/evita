// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /** @const @type {!RegExp} */
  const RE_CR = NewRegExp('\r', 'g');

  /**
   * @class A wrapper class of |TextDecoder| to keep decoded strings.
   */
  class Decoder {
    /**
     * @param {string} encoding
     */
    constructor(encoding) {
      /** @type {?TextDecoder} */
      this.decoder_ = new TextDecoder(encoding, {fatal: true});
      /** @type {!Array.<string>} */
      this.strings = [];
    }

   /** @return {string} */
   get encoding() { return this.decoder_.encoding; }

    /**
     * @param {!Uint8Array} data
     * @return {boolean}
     */
    decode(data) {
      if (!this.decoder_)
        return false;
      try {
        this.strings.push(this.decoder_.decode(data, {stream: data.length > 0}));
        return true;
      } catch (e) {
        this.decoder_ = null;
        return false;
      }
    }
  }

  /**
   * @constructor
   * @class A text encoding detector.
   */
  function EncodingDetector() {
    this.decoders =
        [new Decoder('utf-8'), new Decoder('shift_jis'), new Decoder('euc-jp')];
  }

  /**
   * @type {!Array.<!Decoder>}
   * A list of text encoding decoder in order of preference. The most preferred
   * encoding is at zero.
   */
  EncodingDetector.prototype.decoders;

  /**
   * @param {!Uint8Array} data
   * @return {boolean}
   */
  EncodingDetector.prototype.detect = function(data) {
    this.decoders = this.decoders.filter(function(decoder) {
      return decoder.decode(data);
    });
    return this.decoders.length > 0;
  };

  /**
   * @param {!TextDocument} document
   */
  function resetSelections(document) {
    document.listWindows()
        .filter((window) => window instanceof TextWindow)
        .forEach((window) => { window.selection.range.collapseTo(0); });
  }

  /*
   * Load contents of |fileName| into |document|. The |document| is readonly
   * during reading file contents.
   *
   * |load| function updates following properties when loading succeeded:
   *    * |encoding|
   *    * |lastWriteTime|
   *    * |newline|
   *    * |readonly|
   */
  class FileLoader {
    constructor(document) {
      this.document_ = document;
      this.detector_ = new EncodingDetector();
      this.encoding_ = '';
      this.file_ = null;
      this.firstRead_ = true;
      this.readonly_ = document.readonly;
      this.range_ = new Range(document);
    }

    close() {
      this.document_.readonly = this.readonly_;
      if (!this.file_)
        return;
      this.file_.close();
      this.file_ = null;
    }

    didRead(readData, numRead) {
      const detector = this.detector_;
      if (!detector.detect(readData.subarray(0, numRead)))
        throw new Error('Bad encoding');

      // Display loading result
      // Note: We may display text in wrong encoding.
      const document = this.document_;
      document.readonly = false;
      const decoder = detector.decoders[0];
      const range = this.range_;
      const string =
          decoder.strings[decoder.strings.length - 1].replace(RE_CR, '');
      range.text = string;
      range.collapseTo(range.end);
      document.readonly = true;

      // TDOO(eval1749): We should have loading progress UI feedback since we
      // shows top of document during loading.
      if (this.firstRead_) {
        this.firstRead_ = false;
        resetSelections(document);
      }
    }

    load(fileName) {
      return (async(function * (loader, fileName) {
        const detector = new EncodingDetector();
        // Remember |readonly| property for restoring on error.
        const readonly = loader.document_.readonly;
        loader.range_.start = 0;
        loader.range_.end = loader.document_.length;
        loader.file_ = yield Os.File.open(fileName);
        const file = loader.file_;
        const readData = new Uint8Array(4096);
        for (;;) {
          const numRead = yield file.read(readData);
          if (numRead === 0)
            break;
          loader.didRead(readData, numRead);
        }
        const fileInfo = yield Os.File.stat(fileName);
        loader.finish(fileInfo);
      }))(this, fileName);
    }

    // Reading file contents is finished. Record file last write time.
    finish(fileInfo) {
      const detector = this.detector_;
      const decoder = detector.decoders[0];
      if (!decoder)
        throw new Error('Bad encoding');

      this.readonly_ = fileInfo.readonly;

      const document = this.document_;

      // Adjust newline character
      const range = this.range_;
      range.start = 0;
      range.end = document.length;
      document.readonly = false;
      let newline = Newline.UNKNOWN;
      decoder.strings.forEach(function(string) {
        if (newline === Newline.UNKNOWN) {
          if (string.indexOf('\r\n') >= 0)
            newline = Newline.CRLF;
          else if (string.indexOf('\n') >= 0)
            newline = Newline.LF;
        }
        if (newline === Newline.CRLF)
          string = string.replace(RE_CR, '');
        range.text = string;
        range.collapseTo(range.end);
      });
      resetSelections(document);

      // Update document properties based on file.
      document.encoding = decoder.encoding;
      document.lastWriteTime = fileInfo.lastModificationDate;
      document.modified = false;
      document.newline = newline;
      document.clearUndo();
    }
  }

  /**
   * @param {!TextDocument} document
   * @param {!TextDocument.Obsolete} obsolete
   */
  function finishLoad(document, obsolete) {
    document.obsolete = obsolete;
    document.lastStatTime_ = new Date();
    document.dispatchEvent(new TextDocumentEvent(Event.Names.LOAD));
  }

  /**
   * @this {!TextDocument}
   * @param {string=} opt_fileName
   * @return {!Promise.<number>}
   */
  function load(opt_fileName) {
    const document = this;
    if (opt_fileName === undefined) {
      if (document.fileName === '')
        throw 'TextDocument isn\'t bound to file.';
    } else {
      const fileName = /** @type{string} */ (opt_fileName);
      // TODO(eval1749): FilePath.fullPath() will return Promise.
      const absoluteFile_name = FilePath.fullPath(fileName);
      const present = TextDocument.findFile(absoluteFile_name);
      if (present && present !== this)
        throw fileName + ' is already bound to ' + present;
      document.fileName = absoluteFile_name;
    }

    document.obsolete = TextDocument.Obsolete.CHECKING;
    Editor.messageBox(null, 'Loading ' + document.fileName,
                      MessageBox.ICONINFORMATION);
    document.dispatchEvent(new TextDocumentEvent(Event.Names.BEFORELOAD));

    // Start loading
    const loader = new FileLoader(document);
    return loader.load(document.fileName)
        .then(function(length) {
          loader.close();
          Editor.messageBox(null, 'Loaded ' + document.fileName,
                            MessageBox.ICONINFORMATION);
          finishLoad(document, TextDocument.Obsolete.NO);
          return length;
        })
        .catch(function(exception) {
          loader.close();
          finishLoad(document, TextDocument.Obsolete.UNKNOWN);
          throw exception;
        });
  }

  Object.defineProperty(TextDocument.prototype, 'load', {value: load});
})();
