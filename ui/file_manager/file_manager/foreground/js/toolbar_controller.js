// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * This class controls wires toolbar UI and selection model. When selection
 * status is changed, this class changes the view of toolbar. If cancel
 * selection button is pressed, this class clears the selection.
 * @param {!HTMLElement} cancelSelectionButton Button to cancel selection.
 * @param {!HTMLElement} filesSelectedLabel Label to show how many files are
 *     selected.
 * @param {!FileSelectionHandler} selectionHandler
 * @param {!cr.ui.ListSelectionModel|!cr.ui.ListSingleSelectionModel}
 *     selectionModel
 * @constructor
 * @struct
 */
function ToolbarController(cancelSelectionButton,
                           filesSelectedLabel,
                           selectionHandler,
                           selectionModel) {
  /** @private {!HTMLElement} */
  this.filesSelectedLabel_ = filesSelectedLabel;

  /** @private {!FileSelectionHandler} */
  this.selectionHandler_ = selectionHandler;

  /** @private {!cr.ui.ListSelectionModel|!cr.ui.ListSingleSelectionModel} */
  this.selectionModel_ = selectionModel;

  this.selectionHandler_.addEventListener(
      FileSelectionHandler.EventType.CHANGE,
      this.onSelectionChanged_.bind(this));

  cancelSelectionButton.addEventListener(
      'click', this.onCancelSelectionButtonClicked_.bind(this));
}

/**
 * Handles selection's change event to update the UI.
 * @private
 */
ToolbarController.prototype.onSelectionChanged_ = function() {
  var selection = this.selectionHandler_.selection;

  // Update the label "x files selected." on the header.
  if (selection.totalCount === 0) {
    this.filesSelectedLabel_.textContent = '';
  } else {
    var text;
    if (selection.directoryCount == 0)
      text = strf('MANY_FILES_SELECTED', selection.fileCount);
    else if (selection.fileCount == 0)
      text = strf('MANY_DIRECTORIES_SELECTED', selection.directoryCount);
    else
      text = strf('MANY_ENTRIES_SELECTED', selection.totalCount);
    this.filesSelectedLabel_.textContent = text;
  }

  // Set .selecting class to containing element to change the view accordingly.
  // TODO(fukino): This code changes the state of body, not the toolbar, to
  // update the checkmark visibility on grid view. This should be moved to a
  // controller which controls whole app window. Or, both toolbar and FileGrid
  // should listen to the FileSelectionHandler.
  this.filesSelectedLabel_.ownerDocument.body.classList.toggle(
      'selecting', selection.totalCount > 0);
}

/**
 * Handles click event for cancel button to change the selection state.
 * @private
 */
ToolbarController.prototype.onCancelSelectionButtonClicked_ = function() {
  this.selectionModel_.unselectAll();
}
