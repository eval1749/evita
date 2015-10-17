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
    this.buffer = new Uint8Array(FileWriter.kBufferSize);
    this.file = file;
    this.freeBuffers = [];
    this.offset = 0;
  }

  /** @const @type {number} */
  // TODO(eval1749): Once, we confirm save works fine, we should increase buffer
  // size.
  FileWriter.kBufferSize = 1024 * 1;

  /** @type {!Uint8Array} */
  FileWriter.prototype.buffer;

  /** @type {!Os.File} */
  FileWriter.prototype.file;

  /** @type {!Array.<!Uint8Array>} */
  FileWriter.prototype.freeBuffers;

  /** @type {number} */
  FileWriter.prototype.offset;

  /**
   * @return {!Promise}
   */
  FileWriter.prototype.flush = function() {
    var busy_buffer = this.buffer;
    var array_to_write = this.buffer.subarray(0, this.offset);
    var fileWriter = this;
    var promise = this.file.write(array_to_write).then(function(num_bytes) {
      fileWriter.freeBuffers.push(busy_buffer);
      return num_bytes;
    }).catch(function(error) {
      fileWriter.freeBuffers.push(busy_buffer);
      throw error;
    });

    // Prepare buffer for next write.
    if (this.freeBuffers.length)
      this.buffer = this.freeBuffers.pop();
     else
      this.buffer = new Uint8Array(FileWriter.kBufferSize);
    this.offset = 0;
    return promise;
  };

  /**
   * @param {!Uint8Array} source
   */
  FileWriter.prototype.write = function(source) {
    var source_offset = 0;
    var promises = [];
    while (source_offset < source.length) {
      var source_rest = source.length - source_offset;
      var next_offset = Math.min(this.offset + source_rest, this.buffer.length);
      while (this.offset < next_offset) {
        this.buffer[this.offset] = source[source_offset];
         ++this.offset;
         ++source_offset;
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
      return document.newline == 1 ? '\n' : '\r\n';
    }
    document.obsolete = Document.Obsolete.CHECKING;
    var encoder = new TextEncoder(document.encoding || 'utf-8');
    var readonly = document.readonly;
    document.readonly = true;

    // We close a file, by |Os.File.open| in |should_close| in |catch| clause.
    var should_close = null;

    // When error is occurred, we should remove temporary file |should_remove|
    // named by |Os.File.MakeTempFileName|. In normal situation, we rename
    // temporary file to actual file name as result of save operation.
    var should_remove = null;

    var dir_path = FilePath.dirname(document.fileName);
    return Os.File.makeTempFileName(dir_path, 'ed').then(function(temp_name) {
      return Os.File.open(temp_name, 'w').then(function(file) {
        should_close = file;
        should_remove = temp_name;

        var lines = document.lines;
        var writer = new FileWriter(file);
        var line_separator = lineSeparator(document);
        var total_bytes = 0;
        function writeLoop() {
          var line = lines.next();
          if (line.done) {
            return writer.flush().then(function(num_bytes) {
              // DCHECK_EQ(total_bytes, num_bytes);
              return total_bytes;
            });
          }
          var line_text = line.value;
          var length = line_text.length;
          // Strip newline character
          if (length && line_text.charCodeAt(length - 1) == 0x0A)
            line_text = line_text.substr(0, length - 1);
          // Note: We've always add line separator, because some tools don't
          // work well without line separator.
          var bytes = encoder.encode(line_text + line_separator);
          return writer.write(bytes).then(function(x) {
            total_bytes += bytes.length;
            return writeLoop();
          });
        }
        return writeLoop();
      }).then(function(num_bytes) {
        // Finalize file saving
        return should_close.close().then(function() {
          should_close = null;
          return Os.File.move(temp_name, document.fileName).then(function() {
            should_remove = null;
            return Os.File.stat(document.fileName).then(function(result) {
              var info = /** Os.File.Info */(result);
              document.lastStatTime_ = new Date();
              document.lastWriteTime = info.lastModificationDate;
              document.modified = false;
              document.obsolete = Document.Obsolete.NO;
              document.readonly = readonly;
              document.dispatchEvent(new DocumentEvent('save'));
            });
          });
        });
      });
    }).catch(function(error) {
      if (should_close)
        should_close.close();
      if (should_remove)
        Os.File.remove(should_remove);
      document.lastStatTime_ = new Date();
      document.obsolete = Document.Obsolete.UNKNOWN;
      document.readonly = readonly;
      console.log('save', 'error', error, error.stack);
      throw error;
    });
  }

  /**
   * @param {string=} opt_fileName
   * @return {!Promise}
   */
  Document.prototype.save = function(opt_fileName) {
    var document = this;
    if (!arguments.length) {
      if (document.fileName == '')
        throw 'Document isn\'t bound to file.';
    } else {
      var fileName = /** @type{string} */(opt_fileName);
      var absoluteFileName = FilePath.fullPath(fileName);
      // TODO(eval1749): FilePath.fullPath() will return Promise.
      var present = Document.findFile(absoluteFileName);
      if (present && present !== this)
        throw fileName + ' is already bound to ' + present;
      document.fileName = absoluteFileName;
    }

    var fileName = document.fileName;
    Editor.messageBox(null, 'Saving to ' + fileName,
                      MessageBox.ICONINFORMATION);
    return save(document).then(function() {
      Editor.messageBox(null, 'Saved to ' + fileName,
                        MessageBox.ICONINFORMATION);
    }).catch(function(error) {
      console.log('Failed to save', document, 'to', fileName, error,
                  error.stack);
      Editor.messageBox(null, 'Failed to save\n' + document + '\n' +
        'to ' + fileName + '\n' +
        'reason: ' + error + '\n' + error['stack'],
        MessageBox.ICONERROR);
      throw error;
    });
  };
})();
