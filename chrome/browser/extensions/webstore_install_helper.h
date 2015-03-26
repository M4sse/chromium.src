// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_WEBSTORE_INSTALL_HELPER_H_
#define CHROME_BROWSER_EXTENSIONS_WEBSTORE_INSTALL_HELPER_H_

#include <vector>

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/image_decoder.h"
#include "content/public/browser/utility_process_host_client.h"
#include "net/url_request/url_fetcher_delegate.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "url/gurl.h"

class SkBitmap;

namespace base {
class DictionaryValue;
class ListValue;
}

namespace content {
class UtilityProcessHost;
}

namespace net {
class URLFetcher;
class URLRequestContextGetter;
}

namespace extensions {

// This is a class to help dealing with webstore-provided data. It manages
// sending work to the utility process for parsing manifests and
// fetching/decoding icon data. Clients must implement the
// WebstoreInstallHelper::Delegate interface to receive the parsed data.
class WebstoreInstallHelper : public ImageDecoder::ImageRequest,
                              public content::UtilityProcessHostClient,
                              public net::URLFetcherDelegate {
 public:
  class Delegate {
   public:
    enum InstallHelperResultCode {
      UNKNOWN_ERROR,
      ICON_ERROR,
      MANIFEST_ERROR
    };

    // Called when we've successfully parsed the manifest and decoded the icon
    // in the utility process. Ownership of parsed_manifest is transferred.
    virtual void OnWebstoreParseSuccess(
        const std::string& id,
        const SkBitmap& icon,
        base::DictionaryValue* parsed_manifest) = 0;

    // Called to indicate a parse failure. The |result_code| parameter should
    // indicate whether the problem was with the manifest or icon.
    virtual void OnWebstoreParseFailure(
        const std::string& id,
        InstallHelperResultCode result_code,
        const std::string& error_message) = 0;

   protected:
    virtual ~Delegate() {}
  };

  // It is legal for |icon_url| to be empty.
  WebstoreInstallHelper(Delegate* delegate,
                        const std::string& id,
                        const std::string& manifest,
                        const GURL& icon_url,
                        net::URLRequestContextGetter* context_getter);
  void Start();

 private:
  ~WebstoreInstallHelper() override;

  void StartWorkOnIOThread();
  void ReportResultsIfComplete();
  void ReportResultFromUIThread();

  // Implementing the net::URLFetcherDelegate interface.
  void OnURLFetchComplete(const net::URLFetcher* source) override;

  // Implementing pieces of the UtilityProcessHostClient interface.
  bool OnMessageReceived(const IPC::Message& message) override;

  // Message handlers.
  void OnJSONParseSucceeded(const base::ListValue& wrapper);
  void OnJSONParseFailed(const std::string& error_message);

  // ImageDecoder::ImageRequest implementation.
  void OnImageDecoded(const SkBitmap& decoded_image) override;
  void OnDecodeImageFailed() override;

  // The client who we'll report results back to.
  Delegate* delegate_;

  // The extension id of the manifest we're parsing.
  std::string id_;

  // The manifest to parse.
  std::string manifest_;

  // If |icon_url_| is non-empty, it needs to be fetched and decoded into an
  // SkBitmap.
  GURL icon_url_;

  // For fetching the icon, if needed.
  scoped_ptr<net::URLFetcher> url_fetcher_;
  net::URLRequestContextGetter* context_getter_; // Only usable on UI thread.

  base::WeakPtr<content::UtilityProcessHost> utility_host_;

  // Flags for whether we're done doing icon decoding and manifest parsing.
  bool icon_decode_complete_;
  bool manifest_parse_complete_;

  // The results of succesful decoding/parsing.
  SkBitmap icon_;
  scoped_ptr<base::DictionaryValue> parsed_manifest_;

  // A details string for keeping track of any errors.
  std::string error_;

  // A code to distinguish between an error with the icon, and an error with the
  // manifest.
  Delegate::InstallHelperResultCode parse_error_;
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_WEBSTORE_INSTALL_HELPER_H_
