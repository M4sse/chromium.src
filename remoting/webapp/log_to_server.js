// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * Module for sending log entries to the server.
 */

'use strict';

/** @suppress {duplicate} */
var remoting = remoting || {};

/**
 * @param {remoting.SignalStrategy} signalStrategy Signal strategy.
 * @constructor
 */
remoting.LogToServer = function(signalStrategy) {
  /** @private */
  this.statsAccumulator_ = new remoting.StatsAccumulator();
  /** @private */
  this.sessionId_ = '';
  /** @private */
  this.sessionIdGenerationTime_ = 0;
  /** @private */
  this.sessionStartTime_ = 0;
  /** @private */
  this.signalStrategy_ = signalStrategy;
};

// Constants used for generating a session ID.
/** @private */
remoting.LogToServer.SESSION_ID_ALPHABET_ =
    'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890';
/** @private */
remoting.LogToServer.SESSION_ID_LEN_ = 20;

// The maximum age of a session ID, in milliseconds.
remoting.LogToServer.MAX_SESSION_ID_AGE = 24 * 60 * 60 * 1000;

// The time over which to accumulate connection statistics before logging them
// to the server, in milliseconds.
remoting.LogToServer.CONNECTION_STATS_ACCUMULATE_TIME = 60 * 1000;

/**
 * Logs a client session state change.
 *
 * @param {remoting.ClientSession.State} state
 * @param {remoting.Error} connectionError
 * @param {remoting.ClientSession.Mode} mode
 */
remoting.LogToServer.prototype.logClientSessionStateChange =
    function(state, connectionError, mode) {
  this.maybeExpireSessionId(mode);
  // Maybe set the session ID and start time.
  if (remoting.LogToServer.isStartOfSession(state)) {
    if (this.sessionId_ == '') {
      this.setSessionId();
    }
    if (this.sessionStartTime_ == 0) {
      this.sessionStartTime_ = new Date().getTime();
    }
  }
  // Log the session state change.
  var entry = remoting.ServerLogEntry.makeClientSessionStateChange(
      state, connectionError, mode);
  entry.addHostFields();
  entry.addChromeVersionField();
  entry.addWebappVersionField();
  entry.addSessionIdField(this.sessionId_);
  // Maybe clear the session start time, and log the session duration.
  if (remoting.LogToServer.shouldAddDuration(state) &&
      (this.sessionStartTime_ != 0)) {
    entry.addSessionDurationField(
        (new Date().getTime() - this.sessionStartTime_) / 1000.0);
    if (remoting.LogToServer.isEndOfSession(state)) {
      this.sessionStartTime_ = 0;
    }
  }
  this.log(entry);
  // Don't accumulate connection statistics across state changes.
  this.logAccumulatedStatistics(mode);
  this.statsAccumulator_.empty();
  // Maybe clear the session ID.
  if (remoting.LogToServer.isEndOfSession(state)) {
    this.clearSessionId();
  }
};

/**
 * Whether a session state is one of the states that occurs at the start of
 * a session.
 *
 * @private
 * @param {remoting.ClientSession.State} state
 * @return {boolean}
 */
remoting.LogToServer.isStartOfSession = function(state) {
  return ((state == remoting.ClientSession.State.CONNECTING) ||
      (state == remoting.ClientSession.State.INITIALIZING) ||
      (state == remoting.ClientSession.State.CONNECTED));
};

/**
 * Whether a session state is one of the states that occurs at the end of
 * a session.
 *
 * @private
 * @param {remoting.ClientSession.State} state
 * @return {boolean}
 */
remoting.LogToServer.isEndOfSession = function(state) {
  return ((state == remoting.ClientSession.State.CLOSED) ||
      (state == remoting.ClientSession.State.FAILED) ||
      (state == remoting.ClientSession.State.CONNECTION_DROPPED) ||
      (state == remoting.ClientSession.State.CONNECTION_CANCELED));
};

/**
 * Whether the duration should be added to the log entry for this state.
 *
 * @private
 * @param {remoting.ClientSession.State} state
 * @return {boolean}
 */
remoting.LogToServer.shouldAddDuration = function(state) {
  // Duration is added to log entries at the end of the session, as well as at
  // some intermediate states where it is relevant (e.g. to determine how long
  // it took for a session to become CONNECTED).
  return (remoting.LogToServer.isEndOfSession(state) ||
      (state == remoting.ClientSession.State.CONNECTED));
};

/**
 * Logs connection statistics.
 * @param {Object.<string, number>} stats the connection statistics
 * @param {remoting.ClientSession.Mode} mode
 */
remoting.LogToServer.prototype.logStatistics = function(stats, mode) {
  this.maybeExpireSessionId(mode);
  // Store the statistics.
  this.statsAccumulator_.add(stats);
  // Send statistics to the server if they've been accumulating for at least
  // 60 seconds.
  if (this.statsAccumulator_.getTimeSinceFirstValue() >=
      remoting.LogToServer.CONNECTION_STATS_ACCUMULATE_TIME) {
    this.logAccumulatedStatistics(mode);
  }
};

/**
 * Moves connection statistics from the accumulator to the log server.
 *
 * If all the statistics are zero, then the accumulator is still emptied,
 * but the statistics are not sent to the log server.
 *
 * @private
 * @param {remoting.ClientSession.Mode} mode
 */
remoting.LogToServer.prototype.logAccumulatedStatistics = function(mode) {
  var entry = remoting.ServerLogEntry.makeStats(this.statsAccumulator_, mode);
  if (entry) {
    entry.addHostFields();
    entry.addChromeVersionField();
    entry.addWebappVersionField();
    entry.addSessionIdField(this.sessionId_);
    this.log(entry);
  }
  this.statsAccumulator_.empty();
};

/**
 * Sends a log entry to the server.
 *
 * @private
 * @param {remoting.ServerLogEntry} entry
 */
remoting.LogToServer.prototype.log = function(entry) {
  // Send the stanza to the debug log.
  console.log('Enqueueing log entry:');
  entry.toDebugLog(1);

  var stanza = '<cli:iq to="' + remoting.settings.DIRECTORY_BOT_JID + '" ' +
                       'type="set" xmlns:cli="jabber:client">' +
                 '<gr:log xmlns:gr="google:remoting">' +
                   entry.toStanza() +
                 '</gr:log>' +
               '</cli:iq>';
  this.signalStrategy_.sendMessage(stanza);
};

/**
 * Sets the session ID to a random string.
 *
 * @private
 */
remoting.LogToServer.prototype.setSessionId = function() {
  this.sessionId_ = remoting.LogToServer.generateSessionId();
  this.sessionIdGenerationTime_ = new Date().getTime();
};

/**
 * Clears the session ID.
 *
 * @private
 */
remoting.LogToServer.prototype.clearSessionId = function() {
  this.sessionId_ = '';
  this.sessionIdGenerationTime_ = 0;
};

/**
 * Sets a new session ID, if the current session ID has reached its maximum age.
 *
 * This method also logs the old and new session IDs to the server, in separate
 * log entries.
 *
 * @private
 * @param {remoting.ClientSession.Mode} mode
 */
remoting.LogToServer.prototype.maybeExpireSessionId = function(mode) {
  if ((this.sessionId_ != '') &&
      (new Date().getTime() - this.sessionIdGenerationTime_ >=
      remoting.LogToServer.MAX_SESSION_ID_AGE)) {
    // Log the old session ID.
    var entry = remoting.ServerLogEntry.makeSessionIdOld(this.sessionId_, mode);
    this.log(entry);
    // Generate a new session ID.
    this.setSessionId();
    // Log the new session ID.
    entry = remoting.ServerLogEntry.makeSessionIdNew(this.sessionId_, mode);
    this.log(entry);
  }
};

/**
 * Generates a string that can be used as a session ID.
 *
 * @private
 * @return {string} a session ID
 */
remoting.LogToServer.generateSessionId = function() {
  var idArray = [];
  for (var i = 0; i < remoting.LogToServer.SESSION_ID_LEN_; i++) {
    var index =
        Math.random() * remoting.LogToServer.SESSION_ID_ALPHABET_.length;
    idArray.push(
        remoting.LogToServer.SESSION_ID_ALPHABET_.slice(index, index + 1));
  }
  return idArray.join('');
};
