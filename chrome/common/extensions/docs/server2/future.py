# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

_no_value = object()


def _DefaultErrorHandler(error):
  raise error


def All(futures, except_pass=None):
  '''Creates a Future which returns a list of results from each Future in
  |futures|.

  If any Future raises an error other than those in |except_pass| the returned
  Future will raise as well.
  '''
  def resolve():
    resolved = []
    for f in futures:
      try:
        resolved.append(f.Get())
      # "except None" will simply not catch any errors.
      except except_pass:
        pass
    return resolved
  return Future(callback=resolve)


def Race(futures, except_pass=None):
  '''Returns a Future which resolves to the first Future in |futures| that
  either succeeds or throws an error apart from those in |except_pass|.

  If all Futures throw errors in |except_pass| then the returned Future
  will re-throw one of those errors, for a nice stack trace.
  '''
  def resolve():
    first_future = None
    for future in futures:
      if first_future is None:
        first_future = future
      try:
        return future.Get()
      # "except None" will simply not catch any errors.
      except except_pass:
        pass
    # Everything failed, propagate the first error even though it was
    # caught by |except_pass|.
    return first_future.Get()
  return Future(callback=resolve)


class Future(object):
  '''Stores a value, error, or callback to be used later.
  '''
  def __init__(self, value=_no_value, callback=None, exc_info=None):
    self._value = value
    self._callback = callback
    self._exc_info = exc_info
    if (self._value is _no_value and
        self._callback is None and
        self._exc_info is None):
      raise ValueError('Must have either a value, error, or callback.')

  def Then(self, callback, error_handler=_DefaultErrorHandler):
    '''Creates and returns a future that runs |callback| on the value of this
    future, or runs optional |error_handler| if resolving this future results in
    an exception.
    '''
    def then():
      try:
        val = self.Get()
      except Exception as e:
        return error_handler(e)
      return callback(val)
    return Future(callback=then)

  def Get(self):
    '''Gets the stored value, error, or callback contents.
    '''
    if self._value is not _no_value:
      return self._value
    if self._exc_info is not None:
      self._Raise()
    try:
      self._value = self._callback()
      return self._value
    except:
      self._exc_info = sys.exc_info()
      self._Raise()

  def _Raise(self):
    exc_info = self._exc_info
    raise exc_info[0], exc_info[1], exc_info[2]
