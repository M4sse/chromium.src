// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_IMPORTER_PROFILE_WRITER_H_
#define CHROME_BROWSER_IMPORTER_PROFILE_WRITER_H_

#include <vector>

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_vector.h"
#include "base/string16.h"
#include "base/time.h"
#include "build/build_config.h"
#include "chrome/browser/history/history_types.h"
#include "googleurl/src/gurl.h"

class Profile;
class TemplateURL;

namespace webkit {
namespace forms {
struct PasswordForm;
}
}

#if defined(OS_WIN)
struct IE7PasswordInfo;
#endif

// ProfileWriter encapsulates profile for writing entries into it.
// This object must be invoked on UI thread.
class ProfileWriter : public base::RefCountedThreadSafe<ProfileWriter> {
 public:
  struct BookmarkEntry {
    BookmarkEntry();
    ~BookmarkEntry();
    bool operator==(const BookmarkEntry& other) const;

    bool in_toolbar;
    bool is_folder;
    GURL url;
    std::vector<string16> path;
    string16 title;
    base::Time creation_time;
  };

  explicit ProfileWriter(Profile* profile);

  // These functions return true if the corresponding model has been loaded.
  // If the models haven't been loaded, the importer waits to run until they've
  // completed.
  virtual bool BookmarkModelIsLoaded() const;
  virtual bool TemplateURLServiceIsLoaded() const;

  // Helper methods for adding data to local stores.
  virtual void AddPasswordForm(const webkit::forms::PasswordForm& form);

#if defined(OS_WIN)
  virtual void AddIE7PasswordInfo(const IE7PasswordInfo& info);
#endif

  virtual void AddHistoryPage(const history::URLRows& page,
                              history::VisitSource visit_source);

  virtual void AddHomepage(const GURL& homepage);

  // Adds the |bookmarks| to the bookmark model.
  //
  // (a) If the bookmarks bar is empty:
  //     (i) If |bookmarks| includes at least one bookmark that was originally
  //         located in a toolbar, all such bookmarks are imported directly to
  //         the toolbar; any other bookmarks are imported to a subfolder in
  //         the toolbar.
  //     (i) If |bookmarks| includes no bookmarks that were originally located
  //         in a toolbar, all bookmarks are imported directly to the toolbar.
  // (b) If the bookmarks bar is not empty, all bookmarks are imported to a
  //     subfolder in the toolbar.
  //
  // In either case, if a subfolder is created, the name will be the value of
  // |top_level_folder_name|, unless a folder with this name already exists.
  // If a folder with this name already exists, then the name is uniquified.
  // For example, if |first_folder_name| is 'Imported from IE' and a folder with
  // the name 'Imported from IE' already exists in the bookmarks toolbar, then
  // we will instead create a subfolder named 'Imported from IE (1)'.
  virtual void AddBookmarks(const std::vector<BookmarkEntry>& bookmarks,
                            const string16& top_level_folder_name);

  virtual void AddFavicons(
      const std::vector<history::ImportedFaviconUsage>& favicons);

  // Adds the TemplateURLs in |template_urls| to the local store.  The local
  // store becomes the owner of the TemplateURLs.  Some TemplateURLs in
  // |template_urls| may conflict (same keyword or same host name in the URL)
  // with existing TemplateURLs in the local store, in which case the existing
  // ones take precedence and the duplicates in |template_urls| are deleted.
  // If |unique_on_host_and_path| is true, a TemplateURL is only added if there
  // is not an existing TemplateURL that has a replaceable search url with the
  // same host+path combination.
  virtual void AddKeywords(ScopedVector<TemplateURL> template_urls,
                           bool unique_on_host_and_path);

 protected:
  friend class base::RefCountedThreadSafe<ProfileWriter>;

  virtual ~ProfileWriter();

 private:
  Profile* const profile_;

  DISALLOW_COPY_AND_ASSIGN(ProfileWriter);
};

#endif  // CHROME_BROWSER_IMPORTER_PROFILE_WRITER_H_
