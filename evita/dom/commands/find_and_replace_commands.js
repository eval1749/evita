// Copyright (C) 2014 by Project Vogue.
  // Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

(function() {
  /** @enum {number} */
  var FindDialogBoxId = {
    IDC_FIND_WHAT: 1001,
    IDC_FIND_WITH: 1002,
    IDC_FIND_WORD: 1003,
    IDC_FIND_CASE: 1004,
    IDC_FIND_REGEX: 1005,
    IDC_FIND_ALLDOCS: 1006,
    IDC_FIND_PRESERVE: 1007,
    IDC_FIND_DIRECTION: 1008,
    IDC_FIND_UP: 1009,
    IDC_FIND_DOWN: 1010,
    IDC_FIND_REPLACE_IN: 1011,
    IDC_FIND_SELECTION: 1012,
    IDC_FIND_WHOLE_FILE: 1013,
    IDC_FIND_NEXT: 1014,
    IDC_FIND_PREVIOUS: 1015,
    IDC_FIND_REPLACE: 1016,
    IDC_FIND_REPLACE_ALL: 1017
  };

  /**
   * @constructor
   */
  var FindAndReplaceForm = function() {
    this.form = new Form();
    this.find_what = new TextFieldControl(FindDialogBoxId.IDC_FIND_WHAT);
    this.find_with = new TextFieldControl(FindDialogBoxId.IDC_FIND_WITH);
    this.form.add(this.find_what);
    this.form.add(this.find_with);
    this.form.realize();
  };

  /** @type {FindAndReplaceForm} */
  FindAndReplaceForm.instance_ = null;

  /** @type {!Form} */
  FindAndReplaceForm.prototype.form;

  /** @type {!TextFieldControl} */
  FindAndReplaceForm.prototype.find_with;

  /** @type {TextFieldControl} */
  FindAndReplaceForm.prototype.find_what;

  /**
   * @this {!FindAndReplaceForm}
   * @return {!FindAndReplaceForm}
   */
  FindAndReplaceForm.instance = function() {
    if (!this.instance_)
      this.instance_ = new FindAndReplaceForm();
    return this.instance_;
  };

  function showFindDialogCommand() {
    FindAndReplaceForm.instance().form.show();
  }

  Editor.bindKey(TextWindow, 'Ctrl+F', showFindDialogCommand);
  Editor.bindKey(TextWindow, 'Ctrl+H', showFindDialogCommand);

  Editor.bindKey(TextWindow, 'F3', function() {
    FindAndReplaceForm.instance().form.doFind_(0);
  });

  Editor.bindKey(TextWindow, 'Shift+F3', function() {
    FindAndReplaceForm.instance().form.doFind_(1);
  });
})();
