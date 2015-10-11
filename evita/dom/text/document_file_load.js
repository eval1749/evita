// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /** @const @type {!RegExp} */
  const RE_CR = NewRegExp('\r', 'g');

  /**
   * @constructor
   * @class A wrapper class of |TextDecoder| to keep decoded strings.
   */
  function Decoder(encoding) {
    this.decoder = new TextDecoder(encoding, {fatal: true});
    this.strings = [];
  };

  /** @type {?TextDecoder} */
  Decoder.prototype.decoder;

  /** @type {string} */
  Decoder.prototype.encoding;

  Object.defineProperty(Decoder.prototype, 'encoding', {
    /**
     * @this {!Decoder}
     * @return {string}
     */
    get: function() { return this.decoder.encoding; },
  });

  /** @type {!Array.<string>} */
  Decoder.prototype.strings;

  /**
   * @param {!Uint8Array} data
   * @return {boolean}
   */
  Decoder.prototype.decode = function(data) {
    if (!this.decoder)
      return false;
    try {
      this.strings.push(this.decoder.decode(data, {stream: data.length > 0}));
      return true;
    } catch (e) {
      this.decoder = null;
      return false;
    }
  };

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
   * @param {string} name
   * @return {function(*)}
   */
  function logErrorInPromise(name) {
    return function(error) {
      Editor.messageBox(null, name + '\n' + error + '\n' + error['stack'],
                        MessageBox.ICONERROR);
      console.log(name, error, error.stack);
      return Promise.reject(error);
    };
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

    didRead(read_data, num_read) {
      const detector = this.detector_;
      if (!detector.detect(read_data.subarray(0, num_read)))
        throw new Error('Bad encoding');

      // Display loading result
      // Note: We may display text in wrong encoding.
      this.document_.readonly = false;
      const decoder = detector.decoders[0];
      const range = this.range_;
      const string =
          decoder.strings[decoder.strings.length - 1].replace(RE_CR, '');
      range.text = string;
      range.collapseTo(range.end);
      this.document_.readonly = true;

      // Color portion of text.
      this.document_.doColor_(string.length);
    }

    load(file_name) {
      return (async(function * (loader, file_name) {
        const detector = new EncodingDetector();
        // Remember |readonly| property for restoring on error.
        const readonly = loader.document_.readonly;
        loader.range_.start = 0;
        loader.range_.end = loader.document_.length;
        loader.file_ = yield Os.File.open(file_name);
        const file = loader.file_;
        const read_data = new Uint8Array(4096);
        for (;;) {
          const num_read = yield file.read(read_data);
          if (num_read === 0)
            break;
          loader.didRead(read_data, num_read);
        }
        const file_info = yield Os.File.stat(file_name);
        loader.finish(file_info);
      }))(this, file_name);
    }

    // Reading file contents is finished. Record file last write time.
    finish(file_info) {
      const detector = this.detector_;
      const decoder = detector.decoders[0];
      if (!decoder)
        throw new Error('Bad encoding');

      this.readonly_ = file_info.readonly;

      const document = this.document_;
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

      // Update document properties based on file.
      document.encoding = decoder.encoding;
      document.lastWriteTime = file_info.lastModificationDate;
      document.modified = false;
      document.newline = newline;
      document.clearUndo();
    }
  }

  /**
   * @this {!Document}
   * @param {string=} opt_file_name
   * @return {!Promise.<number>}
   */
  function load(opt_file_name) {
    const document = this;
    if (opt_file_name === undefined) {
      if (document.fileName === '')
        throw 'Document isn\'t bound to file.';
    } else {
      const file_name = /** @type{string} */ (opt_file_name);
      // TODO(yosi) FilePath.fullPath() will return Promise.
      const absolute_file_name = FilePath.fullPath(file_name);
      const present = Document.findFile(absolute_file_name);
      if (present && present !== this)
        throw file_name + ' is already bound to ' + present;
      document.fileName = absolute_file_name;
      const new_mode = Mode.chooseModeByFileName(absolute_file_name);
      if (document.mode !== new_mode)
        document.mode = new_mode;
    }

    document.obsolete = Document.Obsolete.CHECKING;
    Editor.messageBox(null, 'Loading ' + document.fileName,
                      MessageBox.ICONINFORMATION);
    document.dispatchEvent(new DocumentEvent(Event.Names.BEFORELOAD));

    // Start loading
    const loader = new FileLoader(document);
    return loader.load(document.fileName)
        .then(function(length) {
          loader.close();
          Editor.messageBox(null, 'Loaded ' + document.fileName,
                            MessageBox.ICONINFORMATION);
          document.obsolete = Document.Obsolete.NO;
          document.lastStatTime_ = new Date();
          document.parseFileProperties();
          const new_mode = Mode.chooseMode(document);
          if (new_mode.name !== document.mode.name) {
            Editor.messageBox(null, 'Change mode to ' + new_mode.name,
                              MessageBox.ICONINFORMATION);
            document.mode = new_mode;
          }
          document.listWindows().forEach(function(window) {
            if (window instanceof TextWindow) {
              window.selection.range.collapseTo(0);
              window.makeSelectionVisible();
            }
          });
          document.dispatchEvent(new DocumentEvent(Event.Names.LOAD));
          return Promise.resolve(length);
        })
        .catch(function(exception) {
          console.log('load.catch', exception, 'during loading',
                      document.fileName, 'into', document, exception.stack);
          loader.close();
          document.lastStatTime_ = new Date();
          document.obsolete = Document.Obsolete.UNKNOWN;
          document.dispatchEvent(new DocumentEvent(Event.Names.LOAD));
          return Promise.reject(exception);
        });
  }

  Object.defineProperty(Document.prototype, 'load', {value: load});
})();
