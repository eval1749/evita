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

  /** @type {?Form} */
  var form = null;

  /** @return {!Form} */
  function ensureForm() {
    if (form)
      return form;
    form = new Form();
    /** @type {!TextFieldControl} */
    var find_what = new TextFieldControl(FindDialogBoxId.IDC_FIND_WHAT);

    /** @type {!TextFieldControl} */
    var find_with = new TextFieldControl(FindDialogBoxId.IDC_FIND_WITH);

    form.add(find_what);
    form.add(find_with);
    form.realize();

    // TODO(yosi) |global.findForm| is only for debugging purpose.
    global.findForm = form;
    return form;
  }

  function showFindDialogCommand() {
    ensureForm().show();
  }

  Editor.bindKey(TextWindow, 'Ctrl+F', showFindDialogCommand);
  Editor.bindKey(TextWindow, 'Ctrl+H', showFindDialogCommand);

  Editor.bindKey(TextWindow, 'F3', function() {
    ensureForm().doFind_(0);
  });

  Editor.bindKey(TextWindow, 'Shift+F3', function() {
    ensureForm().doFind_(1);
  });
})();
