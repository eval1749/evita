// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /** @const @type {!RegExp} */
  var RE_CR = NewRegExp('\r', 'g');

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
    get: function() {
      return this.decoder.encoding;
    },
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
    this.decoders = [
        new Decoder('utf-8'), new Decoder('shift_jis'), new Decoder('euc-jp')
    ];
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

    didRead(readData, num_read) {
      let detector = this.detector_;
      if (!detector.detect(readData.subarray(0, num_read)))
        throw new Error('Bad encoding');

      // Display loading result
      // Note: We may display text in wrong encoding.
      this.document_.readonly = false;
      let decoder = detector.decoders[0];
      let range = this.range_;
      let string = decoder.strings[decoder.strings.length - 1];
      string = string.replace(RE_CR, '');
      range.text = string;
      range.collapseTo(range.end);
      this.document_.readonly = true;

      // Color portion of text.
      this.document_.doColor_(string.length);
    }

    load(fileName) {
      return (async(function*(loader, fileName) {
        const detector = new EncodingDetector();
        // Remember |readonly| property for restoring on error.
        const readonly = loader.document_.readonly;
        loader.range_.start = 0;
        loader.range_.end = loader.document_.length;
        loader.file_ = yield Os.File.open(fileName);
        let file = loader.file_;
        const readData = new Uint8Array(4096);
        for (;;) {
          let num_read = yield file.read(readData);
          if (num_read == 0)
            break;
          loader.didRead(readData, num_read);
        }
        let file_info = yield Os.File.stat(fileName);
        loader.finish(file_info);
      }))(this, fileName);
    }

    // Reading file contents is finished. Record file last write time.
    finish(file_info) {
      let detector = this.detector_;
      let decoder = detector.decoders[0];
      if (!decoder)
        throw new Error('Bad encoding');

      this.readonly_ = file_info.readonly;

      let document = this.document_;
      let range = this.range_;
      range.start = 0;
      range.end = document.length;

      document.readonly = false;
      let newline = Newline.UNKNOWN;
      decoder.strings.forEach(function(string) {
        if (newline == Newline.UNKNOWN) {
          if (string.indexOf('\r\n') >= 0)
            newline = Newline.CRLF;
          else if (string.indexOf('\n') >= 0)
            newline = Newline.LF;
        }
        if (newline == Newline.CRLF)
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
   * @param {string=} opt_fileName
   * @return {!Promise.<number>}
   */
  global.Document.prototype.load = function(opt_fileName) {
    var document = this;
    if (!arguments.length) {
      if (document.fileName == '')
        throw 'Document isn\'t bound to file.';
    } else {
      var fileName = /** @type{string} */(opt_fileName);
      // TODO(yosi) FilePath.fullPath() will return Promise.
      var absoluteFileName = FilePath.fullPath(fileName);
      var present = Document.findFile(absoluteFileName);
      if (present && present !== this)
        throw fileName + ' is already bound to ' + present;
      document.fileName = absoluteFileName;
      var newMode = Mode.chooseModeByFileName(absoluteFileName);
      if (document.mode != newMode)
        document.mode = newMode;
    }

    document.obsolete = Document.Obsolete.CHECKING;
    Editor.messageBox(null, 'Loading ' + document.fileName,
                      MessageBox.ICONINFORMATION);
    document.dispatchEvent(new DocumentEvent(Event.Names.BEFORELOAD));

    // Start loading
    let loader = new FileLoader(document);
    return loader.load(document.fileName).then(function(length) {
      loader.close();
      Editor.messageBox(null, 'Loaded ' + document.fileName,
                        MessageBox.ICONINFORMATION);
      document.obsolete = Document.Obsolete.NO;
      document.lastStatTime_ = new Date();
      document.parseFileProperties();
      var newMode = Mode.chooseMode(document);
      if (newMode.name != document.mode.name) {
        Editor.messageBox(null, 'Change mode to ' + newMode.name,
                          MessageBox.ICONINFORMATION);
        document.mode = newMode;
      }
      document.listWindows().forEach(function(window) {
        if (window instanceof TextWindow) {
          window.selection.range.collapseTo(0);
          window.makeSelectionVisible();
        }
      });
      document.dispatchEvent(new DocumentEvent(Event.Names.LOAD));
      return Promise.resolve(length);
    }).catch(function(exception) {
      console.log('load.catch', exception, 'during loading', fileName,
                  'into', document, exception.stack);
      loader.close();
      document.lastStatTime_ = new Date();
      document.obsolete = Document.Obsolete.UNKNOWN;
      document.dispatchEvent(new DocumentEvent(Event.Names.LOAD));
      return Promise.reject(exception);
    });
  };
})();
