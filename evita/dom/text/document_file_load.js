// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
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

  /**
   * @param {!Document} document
   * @param {string} file_name
   * @return {!Promise.<number|!Os.File.Error>}
   *
   * Load contents of |file_name| into |document|. The |document| is readonly
   * during reading file contents.
   *
   * |load| function updates following properties when loading succeeded:
   *    * |encoding|
   *    * |lastWriteTime|
   *    * |newline|
   *    * |readonly|
   */
  function load(document, file_name) {
    /** @type {string} */ var encoding = '';
    /** @type {?Os.File} */ var opened_file = null;
    /** @type {number} */ var newline = 0;
    // Remember |readonly| property for restoring on error.
    /** @type {boolean} */ var readonly = document.readonly;
    document.readonly = true;
    var range = new Range(document);
    range.end = document.length;
    return Os.File.open(file_name).then(function(x) {
      var file = /** @type {!Os.File} */(x);
      opened_file = file;
      var detector = new EncodingDetector();
      function readLoop() {
        var data = new Uint8Array(1024 * 64);
        function handleRead(num_bytes) {
          if (num_bytes) {
            if (!detector.detect(data.subarray(0, num_bytes)))
              return Promise.reject(new Error('Bad encoding'));

            // Display loading result
            // Note: We may display text in wrong encoding.
            document.readonly = false;
            var decoder = detector.decoders[0];
            var string = decoder.strings[decoder.strings.length - 1];
            string = string.replace(/\r\n/g, '\n');
            string = string.replace(/\r/g, '\n');
            range.text = string;
            range.collapseTo(range.end);
            document.readonly = true;

            // Read rest of contents.
            return readLoop();
          }

          // Reading file contents is finished. Record file last write
          // time.
          return Os.File.stat(file_name).then(function(x) {
            var file_info = /** @type {!Os.File.Info} */(x);
            file.close();
            var decoder = detector.decoders[0];
            if (!decoder)
              return Promise.reject(new Error('Bad encoding'));

            // Inserts file contents into document with replacing CRLF to LF.
            var range = new Range(document);
            range.end = document.length;
            var newline = 0;
            var has_cr = false;
            decoder.strings.forEach(function(string) {
              if (!newline) {
                if (string.indexOf('\r\n') >= 0)
                  newline = 3;
                else if (string.indexOf('\n') >= 0)
                  newline = 1;
                if (string.indexOf('\r') >= 0)
                  has_cr = true;
              }
              document.readonly = false;
              if (newline == 3)
                string = string.replace(/\r\n/g, '\n');
              if (has_cr)
                string = string.replace(/\r/g, '\n');
              range.text = string;
              document.readonly = true;
              range.collapseTo(range.end);
            });

            // Update document properties based on file.
            document.encoding = decoder.encoding;
            document.lastWriteTime = file_info.lastModificationDate;
            document.modified = false;
            document.newline = newline;
            document.readonly = file_info.readonly || readonly;
            document.clearUndo();
            return Promise.accept(document.length);
          }).catch(logErrorInPromise('load/open/stat'));
        }
        return file.read(data).then(handleRead);
      }
      return readLoop();
    }).catch(function(error) {
      console.log('load', 'error', error, 'opened_file', opened_file);
      document.readonly = readonly;
      if (opened_file)
        opened_file.close();
      return Promise.reject(error);
    });
  }


  /**
   * @param {string=} opt_filename
   * @return {!Promise.<number>}
   */
  global.Document.prototype.load = function(opt_filename) {
    var document = this;
    if (!arguments.length) {
      if (document.fileName == '')
        throw 'Document isn\'t bound to file.';
    } else {
      var filename = /** @type{string} */(opt_filename);
      // TODO(yosi) FilePath.fullPath() will return Promise.
      var absolute_filename = FilePath.fullPath(filename);
      var present = Document.findFile(absolute_filename);
      if (present && present !== this)
        throw filename + ' is already bound to ' + present;
      document.fileName = absolute_filename;
    }

    document.obsolete = Document.Obsolete.CHECKING;
    Editor.messageBox(null, 'Loading ' + document.fileName,
                      MessageBox.ICONINFORMATION);
    return load(document, document.fileName).then(function(length) {
      Editor.messageBox(null, 'Loaded ' + document.fileName,
                        MessageBox.ICONINFORMATION);
      document.obsolete = Document.Obsolete.NO;
      document.lastStatTime_ = new Date();
      document.parseFileProperties();
      var new_mode = Mode.chooseMode(document);
      if (new_mode.name != document.mode.name) {
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
      document.doColor_(Math.min(document.length, 1024 * 8));
      document.dispatchEvent(new DocumentEvent('load'));
      return Promise.resolve(length);
    }).catch(function(exception) {
      console.log('load.catch', exception, 'during loading', filename,
                  'into', document, exception.stack);
      document.lastStatTime_ = new Date();
      document.obsolete = Document.Obsolete.UNKNOWN;
      return Promise.reject(exception);
    });
  };
})();
