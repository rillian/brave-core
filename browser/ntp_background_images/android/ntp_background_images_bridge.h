/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_NTP_BACKGROUND_IMAGES_ANDROID_NTP_BACKGROUND_IMAGES_BRIDGE_H_
#define BRAVE_BROWSER_NTP_BACKGROUND_IMAGES_ANDROID_NTP_BACKGROUND_IMAGES_BRIDGE_H_

#include <memory>
#include <string>

#include "base/android/jni_android.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/singleton.h"
#include "brave/components/ntp_background_images/browser/ntp_background_images_service.h"
#include "brave/components/ntp_background_images/buildflags/buildflags.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "components/keyed_service/core/keyed_service.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

class Profile;

namespace ntp_background_images {
#if BUILDFLAG(ENABLE_NTP_BACKGROUND_IMAGES)
struct NTPBackgroundImagesData;
#endif
struct NTPSponsoredImagesData;
class ViewCounterService;
}

#if BUILDFLAG(ENABLE_NTP_BACKGROUND_IMAGES)
using ntp_background_images::NTPBackgroundImagesData;
#endif
using ntp_background_images::NTPBackgroundImagesService;
using ntp_background_images::NTPSponsoredImagesData;
using ntp_background_images::ViewCounterService;

class NTPBackgroundImagesBridge : public NTPBackgroundImagesService::Observer,
                                 public KeyedService {
 public:
  explicit NTPBackgroundImagesBridge(Profile* profile);
  ~NTPBackgroundImagesBridge() override;

  void RegisterPageView(JNIEnv* env,
                        const base::android::JavaParamRef<jobject>& obj);
  void WallpaperLogoClicked(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& obj,
      const base::android::JavaParamRef<jstring>& jcreativeInstanceId,
      const base::android::JavaParamRef<jstring>& jdestinationUrl,
      const base::android::JavaParamRef<jstring>& jwallpaperId);
  base::android::ScopedJavaLocalRef<jobject> GetCurrentWallpaper(
      JNIEnv* env, const base::android::JavaParamRef<jobject>& obj);
  void GetTopSites(JNIEnv* env,
                        const base::android::JavaParamRef<jobject>& obj);
  bool IsSuperReferral(JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj);
  base::android::ScopedJavaLocalRef<jstring>
  GetSuperReferralThemeName(JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj);
  base::android::ScopedJavaLocalRef<jstring> GetSuperReferralCode(JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj);
  base::android::ScopedJavaLocalRef<jstring> GetReferralApiKey(JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj);

  base::android::ScopedJavaLocalRef<jobject> GetJavaObject();

 private:
#if BUILDFLAG(ENABLE_NTP_BACKGROUND_IMAGES)
  void OnUpdated(NTPBackgroundImagesData* data) override;
#endif
  void OnUpdated(NTPSponsoredImagesData* data) override;
  void OnSuperReferralEnded() override;

#if BUILDFLAG(ENABLE_NTP_BACKGROUND_IMAGES)
  base::android::ScopedJavaLocalRef<jobject> CreateWallpaper(base::Value* data);
#endif
  base::android::ScopedJavaLocalRef<jobject> CreateBrandedWallpaper(
      base::Value* data);

  Profile* profile_;
  ViewCounterService* view_counter_service_;
  NTPBackgroundImagesService* background_images_service_;
  base::android::ScopedJavaGlobalRef<jobject> java_object_;

  DISALLOW_COPY_AND_ASSIGN(NTPBackgroundImagesBridge);
};

namespace ntp_background_images {

class NTPBackgroundImagesBridgeFactory
    : public BrowserContextKeyedServiceFactory {
 public:
  static NTPBackgroundImagesBridgeFactory* GetInstance();
  static NTPBackgroundImagesBridge* GetForProfile(Profile* profile);

 private:
  friend struct base::DefaultSingletonTraits<NTPBackgroundImagesBridgeFactory>;

  NTPBackgroundImagesBridgeFactory();
  ~NTPBackgroundImagesBridgeFactory() override;

  // BrowserContextKeyedServiceFactory:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  bool ServiceIsCreatedWithBrowserContext() const override;

  DISALLOW_COPY_AND_ASSIGN(NTPBackgroundImagesBridgeFactory);
};

}  // namespace ntp_background_images

#endif  // BRAVE_BROWSER_NTP_BACKGROUND_IMAGES_ANDROID_NTP_BACKGROUND_IMAGES_BRIDGE_H_
