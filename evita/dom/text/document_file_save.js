// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /**
   * @constructor
   * @class A buffered file writer
   * @file {!Os.File} file
   */
  function FileWriter(file) {
    this.buffer = new Uint8Array(1024 * 1);
    this.file = file;
    this.offset = 0;

  /**
   * @return {!Promise}
   */
  FileWriter.prototype.flush = function() {
    var promise = file.write(this.buffer.subarray(0, this.offset));
    this.offset = 0;
    return promise;
  };
  }

  /**
   * @param {!Uint8Array} source
   */
  FileWriter.prototype.write = function(source) {
    var source_offset = 0;
    var source_rest = source.length;
    var promises = [];
    while (source_rest) {
      var next_offset = Math.min(this.offset + source_rest, this.buffer.length);
      var num_copy = next_offset - this.offset;
      if (num_copy) {
        var view = new Uint8Array(this.buffer.buffer, this.offset, num_copy);
        view.set(new Uint8Array(source.buffer, source_offset, num_copy));
        this.offset = next_offset;
        source_offset += num_copy;
        source_rest -= num_copy;
      }
      if (this.offset == this.buffer.length)
        promises.push(this.flush());
    }
    return Promise.all(promises);
  };

  /**
   * @param {!Document} document
   * @return {!Promise}
   *
   * This function does following steps:
   *  - Make temporary file name
   *  - Open temporary file for writing
   *  - Encode line and write to file by using |FileWriter|.
   *  - Close temporary file
   *  - Rename temporary file to real file name
   *  - Query file information
   *  - Populate |Document| properties
   *  - Dispatch "save" event
   */
  function save(document) {
    function lineSeparator(document) {
      return document.newline == 2 ? '\n' : '\r\n';
    }
    document.obsolete = Document.Obsolete.CHECKING;
    var encoder = new TextEncoder(document.encoding || 'utf-8');
    var opened_file = null;
    var readonly = document.readonly;
    document.readonly = true;
    var dir_path = FilePath.dirname(document.filename);
    return Os.File.makeTempFileName(dir_path, 'ed').then(function(temp_name) {
      return Os.File.open(temp_name, 'w').then(function(file) {
        opened_file = file;
        var lines = document.lines;
        var writer = new FileWriter(file);
        var line_separator = lineSeparator(document);
        var total_bytes = 0;
        function writeLoop() {
          var line = lines.next();
          if (line.done) {
            return writer.flush().then(function(num_bytes) {
              // DCHECK_EQ(total_bytes, num_bytes);
              console.log('save', 'flush', num_bytes, total_bytes);
              return total_bytes;
            });
          }
          var line_text = line.value.trimRight();
          // Note: We've always add line separator, because some tools don't
          // work well without line separator.
          var bytes = encoder.encode(line_text + line_separator);
          return writer.write(bytes).then(function(x) {
            console.log('save', 'write', bytes.length, x);
            total_bytes += bytes.length;
            return writeLoop();
          });
        }
        return writeLoop();
      }).then(function(num_bytes) {
        // Finalize file saving
        return opened_file.close().then(function() {
          opened_file = null;
          console.log('save', 'done', num_bytes, temp_name, document.filename);
          return Os.File.move(temp_name, document.filename).then(function() {
            return Os.File.stat(document.filename).then(function(result) {
              var info = /** Os.File.Info */(result);
              document.lastStatTime_ = new Date();
              document.lastWriteTime = info.lastModificationDate;
              document.obsolete = Document.Obsolete.NO;
              document.readonly = readonly;
              document.dispatchEvent(new DocumentEvent('save'));
            });
          });
        });
      });
    }).catch(function(error) {
      if (opened_file)
        opened_file.close();
      document.lastStatTime_ = new Date();
      document.obsolete = Document.Obsolete.UNKNOWN;
      document.readonly = readonly;
      console.log('save', 'error', error, error.stack);
    });
  }

  /**
   * @param {string=} opt_filename
   * @return {!Promise}
   */
  Document.prototype.save = function(opt_filename) {
    var document = this;
    if (!arguments.length) {
      if (document.filename == '')
        throw 'Document isn\'t bound to file.';
    } else {
      var filename = /** @type{string} */(opt_filename);
      var absolute_filename = FilePath.fullPath(filename);
      // TODO(yosi) FilePath.fullPath() will return Promise.
      var present = Document.findFile(absolute_filename);
      if (present && present !== this)
        throw filename + ' is already bound to ' + present;
      document.filename = absolute_filename;
    }

    var file_name = document.filename;
    Editor.messageBox(null, 'Saving to ' + file_name,
                      MessageBox.ICONINFORMATION);
    return save(document).then(function() {
      Editor.messageBox(null, 'Save to ' + file_name,
                        MessageBox.ICONINFORMATION);
    }).catch(function(error) {
      console.log('Failed to save', document, 'to', file_name, error,
                  error.stack);
      Editor.messageBox(null, 'Failed to save\n' + document + '\n' +
        'to ' + file_name + '\n' +
        'reason: ' + error + '\n' + error['stack'],
        MessageBox.ICONERROR);
    });
  };
})();
