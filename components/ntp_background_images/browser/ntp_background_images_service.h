/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_NTP_BACKGROUND_IMAGES_BROWSER_NTP_BACKGROUND_IMAGES_SERVICE_H_
#define BRAVE_COMPONENTS_NTP_BACKGROUND_IMAGES_BROWSER_NTP_BACKGROUND_IMAGES_SERVICE_H_

#include <memory>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/gtest_prod_util.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/timer/timer.h"
#include "base/values.h"
#include "brave/components/ntp_background_images/buildflags/buildflags.h"
#include "components/prefs/pref_change_registrar.h"

namespace component_updater {
class ComponentUpdateService;
}  // namespace component_updater

class PrefRegistrySimple;
class PrefService;

namespace ntp_background_images {

#if BUILDFLAG(ENABLE_NTP_BACKGROUND_IMAGES)
struct NTPBackgroundImagesData;
#endif
struct NTPSponsoredImagesData;

class NTPBackgroundImagesService {
 public:
  class Observer {
   public:
    // Called whenever ntp background images component is updated.
#if BUILDFLAG(ENABLE_NTP_BACKGROUND_IMAGES)
    virtual void OnUpdated(NTPBackgroundImagesData* data) = 0;
#endif
    virtual void OnUpdated(NTPSponsoredImagesData* data) = 0;
    // Called when SR campaign ended.
    virtual void OnSuperReferralEnded() = 0;
   protected:
    virtual ~Observer() {}
  };

  static void RegisterLocalStatePrefs(PrefRegistrySimple* registry);

  NTPBackgroundImagesService(
      component_updater::ComponentUpdateService* cus,
      PrefService* local_pref);
  virtual ~NTPBackgroundImagesService();

  NTPBackgroundImagesService(const NTPBackgroundImagesService&) = delete;
  NTPBackgroundImagesService& operator=(
      const NTPBackgroundImagesService&) = delete;

  void Init();

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);
  bool HasObserver(Observer* observer);

#if BUILDFLAG(ENABLE_NTP_BACKGROUND_IMAGES)
  NTPBackgroundImagesData* GetBackgroundImagesData() const;
#endif
  NTPSponsoredImagesData* GetBrandedImagesData(bool super_referral) const;

  bool test_data_used() const { return test_data_used_; }

  bool IsSuperReferral() const;
  std::string GetSuperReferralThemeName() const;
  std::string GetSuperReferralCode() const;

  std::vector<std::string> GetTopSitesFaviconList() const;

 private:
  friend class TestNTPBackgroundImagesService;
  friend class NTPBackgroundImagesServiceTest;
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesServiceTest, InternalDataTest);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesServiceTest,
                           WithDefaultReferralCodeTest1);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesServiceTest,
                           WithDefaultReferralCodeTest2);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesServiceTest,
                           WithNonSuperReferralCodeTest);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesServiceTest,
                           WithSuperReferralCodeTest);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesServiceTest,
                           BasicSuperReferralTest);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesServiceTest,
                           CheckReferralServiceInitStatusTest);
  FRIEND_TEST_ALL_PREFIXES(
      NTPBackgroundImagesServiceTest,
      CheckRecoverShutdownWhileMappingTableFetchingWithDefaultCode);
  FRIEND_TEST_ALL_PREFIXES(
      NTPBackgroundImagesServiceTest,
      CheckRecoverShutdownWhileMappingTableFetchingWithNonDefaultCode);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesViewCounterTest,
                           SINotActiveInitially);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesViewCounterTest,
                           SINotActiveWithBadData);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesViewCounterTest,
                           NotActiveOptedOut);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesViewCounterTest,
                           IsActiveOptedIn);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesViewCounterTest,
                           ActiveInitiallyOptedIn);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesViewCounterTest,
                           ActiveOptedInWithNTPBackgoundOption);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesViewCounterTest, ModelTest);
#if BUILDFLAG(ENABLE_NTP_BACKGROUND_IMAGES)
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesViewCounterTest,
                           BINotActiveInitially);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesViewCounterTest,
                           BINotActiveWithBadData);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesViewCounterTest,
                           BINotActiveWithNTPBackgoundOptionOptedOut);
#endif
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesSourceTest, BasicTest);
  FRIEND_TEST_ALL_PREFIXES(NTPBackgroundImagesSourceTest,
                           BasicSuperReferralDataTest);

  void OnSponsoredComponentReady(bool is_super_referral,
                                 const base::FilePath& installed_dir);
  void OnGetSponsoredComponentJsonData(bool is_super_referral,
                                       const std::string& json_string);
#if BUILDFLAG(ENABLE_NTP_BACKGROUND_IMAGES)
  void OnComponentReady(const base::FilePath& installed_dir);
  void OnGetComponentJsonData(const std::string& json_string);
#endif
  void OnMappingTableComponentReady(const base::FilePath& installed_dir);
  void OnPreferenceChanged(const std::string& pref_name);
  void OnGetMappingTableData(const std::string& json_string);

  std::string GetReferralPromoCode() const;
  bool IsValidSuperReferralComponentInfo(
      const base::Value& component_info) const;

  void CacheTopSitesFaviconList();
  void CheckImagesComponentUpdate(const std::string& component_id);

  // virtual for test.
  virtual void CheckSuperReferralComponent();
#if BUILDFLAG(ENABLE_NTP_BACKGROUND_IMAGES)
  virtual void RegisterBackgroundImagesComponent();
#endif
  virtual void RegisterSponsoredImagesComponent();
  virtual void RegisterSuperReferralComponent();
  virtual void DownloadSuperReferralMappingTable();
  virtual void MonitorReferralPromoCodeChange();
  virtual void UnRegisterSuperReferralComponent();
  virtual void MarkThisInstallIsNotSuperReferralForever();

  base::RepeatingTimer si_update_check_timer_;
  std::vector<std::string> top_site_favicon_list_;
  bool test_data_used_ = false;
  component_updater::ComponentUpdateService* component_update_service_;
  PrefService* local_pref_;
#if BUILDFLAG(ENABLE_NTP_BACKGROUND_IMAGES)
  base::FilePath bi_installed_dir_;
  std::unique_ptr<NTPBackgroundImagesData> bi_images_data_;
#endif
  base::FilePath si_installed_dir_;
  base::FilePath sr_installed_dir_;
  base::ObserverList<Observer>::Unchecked observer_list_;
  std::unique_ptr<NTPSponsoredImagesData> si_images_data_;
  std::unique_ptr<NTPSponsoredImagesData> sr_images_data_;
  PrefChangeRegistrar pref_change_registrar_;
  // This is only used for registration during initial(first) SR component
  // download. After initial download is done, it's cached to
  // |kNewTabPageCachedSuperReferralComponentInfo|. At next launch, this cached
  // info is used for registering SR component.
  // Why component info is temporarily stored to |initial_sr_component_info_|
  // when mapping table is fetched instead of directly store it into that prefs?
  // The reason is |kNewTabPageCachedSuperReferralComponentInfo| is used to
  // check whether initial download is finished or not. Knowing initial download
  // is done is important for super referral. If this is SR install, we should
  // not show SI images until user chooses Brave default images. So, we should
  // know the exact timing whether SR assets is ready to use or not.
  base::Value initial_sr_component_info_;
  base::WeakPtrFactory<NTPBackgroundImagesService> weak_factory_;
};

}  // namespace ntp_background_images

#endif  // BRAVE_COMPONENTS_NTP_BACKGROUND_IMAGES_BROWSER_NTP_BACKGROUND_IMAGES_SERVICE_H_
