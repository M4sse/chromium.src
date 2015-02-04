// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ANDROID_BANNERS_APP_BANNER_MANAGER_H_
#define CHROME_BROWSER_ANDROID_BANNERS_APP_BANNER_MANAGER_H_

#include "base/android/jni_android.h"
#include "base/android/jni_weak_ref.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/browser/android/banners/app_banner_infobar_delegate.h"
#include "chrome/browser/bitmap_fetcher/bitmap_fetcher.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/manifest.h"

namespace content {
struct FrameNavigateParams;
struct LoadCommittedDetails;
struct Manifest;
}  // namespace content

/**
 * Manages when an app banner is created or dismissed.
 *
 * Hooks the wiring together for getting the data for a particular app.
 * Monitors at most one package at a time, and tracks the info for the
 * most recent app that was requested.  Any work in progress for other apps is
 * discarded.
 *
 * The procedure for creating a banner spans multiple asynchronous calls across
 * the JNI boundary, as well as querying a Service to get info about the app.
 *
 * 0) A navigation of the main frame is triggered.  Upon completion of the load,
 *    the page is parsed for the correct meta tag.  If it doesn't exist, abort.
 *
 * 1) The AppBannerManager is alerted about the tag's contents, which should
 *    be the Play Store package name.  This is sent to the Java side
 *    AppBannerManager.
 *
 * 2) The AppBannerManager's ServiceDelegate is asynchronously queried about the
 *    package name.
 *
 * 3) At some point, the Java-side AppBannerManager is alerted of the completed
 *    query and is given back data about the requested package, which includes a
 *    URL for the app's icon.  This URL is sent to native code for retrieval.
 *
 * 4) The process of fetching the icon begins by invoking the BitmapFetcher,
 *    which works asynchonously.
 *
 * 5) Once the icon has been downloaded, the icon is sent to the Java-side
 *    AppBannerManager to (finally) create a AppBannerView, assuming that the
 *    app we retrieved the details for is still for the page that requested it.
 *
 * Because of the asynchronous nature of this pipeline, it's entirely possible
 * that a request to show a banner is interrupted by another request.  The
 * Java side manages what happens in these situations, which will usually result
 * in dropping the old banner request on the floor.
 */

namespace banners {

class AppBannerManager : public chrome::BitmapFetcherDelegate,
                         public content::WebContentsObserver,
                         public AppBannerInfoBarDelegate::AppDelegate {
 public:
  AppBannerManager(JNIEnv* env, jobject obj);
  ~AppBannerManager() override;

  // Destroys the AppBannerManager.
  void Destroy(JNIEnv* env, jobject obj);

  // Blocks a banner for |package_name| from appearing on the domain for |url|.
  void BlockBanner(JNIEnv* env, jobject obj, jstring jurl, jstring jpackage);

  // Observes a new WebContents, if necessary.
  void ReplaceWebContents(JNIEnv* env,
                          jobject obj,
                          jobject jweb_contents);

  // Fetches the icon at the given URL asynchronously.
  // Returns |false| if this couldn't be kicked off.
  bool FetchIcon(JNIEnv* env,
                 jobject obj,
                 jstring jimage_url);

  // Fetches the icon at the given URL asynchronously.
  // Returns |false| if this couldn't be kicked off.
  bool FetchIcon(const GURL& image_url);

  // Installs the app defined by the manifest.
  // TODO(dfalcantara): Fold into Install() when more CLs land.
  static void InstallManifestApp(const content::Manifest& manifest,
                                 const SkBitmap& icon);

  // WebContentsObserver overrides.
  void DidNavigateMainFrame(
      const content::LoadCommittedDetails& details,
      const content::FrameNavigateParams& params) override;
  void DidFinishLoad(content::RenderFrameHost* render_frame_host,
                     const GURL& validated_url) override;
  bool OnMessageReceived(const IPC::Message& message) override;

  // BitmapFetcherDelegate overrides.
  void OnFetchComplete(const GURL url, const SkBitmap* bitmap) override;

  // AppBannerInfoBarDelegate::AppDelegate overrides.
  void Block() const override;
  void Install() const override;
  gfx::Image GetIcon() const override;

 private:
  // Gets the preferred icon size for the banner icons.
  int GetPreferredIconSize();

  // Called when the manifest has been retrieved, or if there is no manifest to
  // retrieve.
  void OnDidGetManifest(const content::Manifest& manifest);

  // Called when the renderer has returned information about the meta tag.
  // If there is some metadata for the play store tag, this kicks off the
  // process of showing a banner for the package designated by |tag_content| on
  // the page at the |expected_url|.
  void OnDidRetrieveMetaTagContent(bool success,
                                   const std::string& tag_name,
                                   const std::string& tag_content,
                                   const GURL& expected_url);

  // Fetches the icon for an app.
  scoped_ptr<chrome::BitmapFetcher> fetcher_;
  GURL validated_url_;
  content::Manifest manifest_;
  scoped_ptr<SkBitmap> app_icon_;

  // AppBannerManager on the Java side.
  JavaObjectWeakGlobalRef weak_java_banner_view_manager_;

  DISALLOW_COPY_AND_ASSIGN(AppBannerManager);
};  // class AppBannerManager

// Register native methods
bool RegisterAppBannerManager(JNIEnv* env);

}  // namespace banners

#endif  // CHROME_BROWSER_ANDROID_BANNERS_APP_BANNER_MANAGER_H_
