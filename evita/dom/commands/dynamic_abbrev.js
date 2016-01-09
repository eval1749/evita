// Note: DynamicAbbrevExpander should be in file scope. Although, when we use
// IIFE, immediately-invoked function expression, Closure compiler can't find
// |DynamicAbbrevExpander| type.

/**
 * @constructor
 * @param {!TextDocument} document
 */
function DynamicAbbrevExpander(document) {
  this.cursor = new TextRange(document);
  this.direction = 0;
  this.last_selection_range = null;
  this.last_selection_start = 0;
  this.prefix = '';
}

/** @const @type{string} */
DynamicAbbrevExpander.PROPERTY_NAME = 'evita.DynamicAbbrevExpander';

/** @type {!TextRange} */
DynamicAbbrevExpander.cursor;

/** @type {number} */
DynamicAbbrevExpander.direction;

/** @type {?TextRange} */
DynamicAbbrevExpander.last_selection_range;

/** @type {number} */
DynamicAbbrevExpander.last_selection_start;

/** @type {string} */
DynamicAbbrevExpander.prefix;

/**
 * @param {!TextDocument} document
 * @return {!DynamicAbbrevExpander}
 */
DynamicAbbrevExpander.getOrCreate = function(document) {
  if (!document.properties)
    document.properties = new Map();
  var expander = document.properties.get(DynamicAbbrevExpander.PROPERTY_NAME);
  if (expander)
    return expander;
  var new_expander = new DynamicAbbrevExpander(document);
  document.properties.set(DynamicAbbrevExpander.PROPERTY_NAME, new_expander);
  return new_expander;
}

/**
 * @this {!DynamicAbbrevExpander}
 * @param {!TextSelection} selection
 */
DynamicAbbrevExpander.prototype.expand = function(selection) {
  var range = selection.range;
  if (range.start != range.end)
    return;
  if (!this.prepare(selection)) {
    this.cursor.collapseTo(range.start);
    this.cursor.moveStart(Unit.WORD, -1);
    this.direction = -1;
    this.prefix = this.cursor.text;
    this.last_selection_range = range;
    this.last_selection_start = range.start;
    range.start = this.cursor.start;
  }

  var current_word = range.text;
  while (this.findCandidate(current_word)) {
    var new_word = this.cursor.text;
    if (current_word != new_word) {
      range.text = new_word;
      range.collapseTo(range.end);
      this.last_selection_start = range.start;
      return;
    }
  }

  // Nothing changed.
  range.collapseTo(range.end);
  selection.window.status = current_word ?
    'No more words start with "' + this.prefix + '", other than "' +
        current_word + '"' :
    'No word starts with "' + this.prefix + '".';
};

/**
 * @private
 * @this {!DynamicAbbrevExpander}
 * @param {string} current_word
 * @return {boolean}
 *
 * Move |cursor| to the word starts with |prefix| except for |current_word|.
 */
DynamicAbbrevExpander.prototype.findCandidate = function(current_word) {
  /**
   * @param {string} text
   * @param {!TextDocument} document
   * @param {number} offset
   * @return {boolean}
   */
  function matchAt(text, document, offset) {
    for (var k = 0; k < text.length; ++k) {
      if (document.charCodeAt(offset) != text.charCodeAt(k))
        return false;
      ++offset;
    }
    return true;
  }

  /**
   * @param {string} text
   * @param {!TextRange} range
   * @param {number} direction
   * @return {boolean}
   */
  function findWordStartsWith(text, range, direction) {
    if (text == '' || !direction)
      return false;

    var document = range.document;
    var text_length = text.length;
    if (direction > 0) {
      var max_start = document.length - text_length;
      do {
        range.move(Unit.WORD, 1);
        if (range.start > max_start)
          break;
        if (matchAt(text, document, range.start))
          return true;
      } while (range.start < max_start);
      range.collapseTo(document.length);
    } else if (direction < 0) {
      do {
        range.move(Unit.WORD, -1);
        if (range.start + text_length > document.length)
          break;
        if (matchAt(text, document, range.start))
          return true;
      } while (range.start);
      range.collapseTo(0);
    }
    return false;
  }

  var cursor = this.cursor;
  var origin_end = cursor.end;
  var origin_start = cursor.start;
  // Try current direction and opposite direction to find a candidate.
  for (var count = 0; count < 2; ++count) {
    if (findWordStartsWith(this.prefix, cursor, this.direction)) {
      cursor.endOf(Unit.WORD, Alter.EXTEND);
      if (cursor.end == origin_end || cursor.start == origin_start)
        return false;
      if (cursor.end - cursor.start == this.prefix.length)
        continue;
      if (cursor.text == current_word)
        continue;
      return true;
    }

    // Continue find a candiate in oposite direction.
    this.direction = -this.direction;
    if (this.direction> 0)
      cursor.collapseTo(this.last_selection_range.end);
    else
      cursor.collapseTo(this.last_selection_range.start);
  }
  return false;
};

/**
 * @this {!DynamicAbbrevExpander}
 * @param {!TextSelection} selection
 * TODO(eval1749): Check this command and last command
 */
DynamicAbbrevExpander.prototype.prepare = function(selection) {
  if (this.prefix == '')
   return false;
  if (this.last_selection_range !== selection.range) {
    // Someone invokes expander in different window.
    return false;
  }
  if (this.last_selection_start != selection.range.start) {
    // Selection is moved.
    return false;
  }
  // selection is at end of expanded word.
  selection.range.moveStart(Unit.WORD, - 1);
  if (!selection.range.text.startsWith(this.prefix)) {
    selection.range.collapseTo(selection.range.end);
    return false;
  }
  return true;
};

Editor.bindKey(TextWindow, 'Ctrl+/', function() {
  var expander = DynamicAbbrevExpander.getOrCreate(this.document);
  expander.expand(this.selection);
});
