/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_BUNDLE_CREATIVE_AD_NOTIFICATION_UNITTEST_UTIL_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_BUNDLE_CREATIVE_AD_NOTIFICATION_UNITTEST_UTIL_H_

#include <string>

namespace ads {

struct CreativeAdNotificationInfo;

CreativeAdNotificationInfo GetCreativeAdNotification();

CreativeAdNotificationInfo GetCreativeAdNotification(const std::string& segment,
                                                     const double ptr,
                                                     const int priority);

CreativeAdNotificationInfo GetCreativeAdNotification(
    const std::string& creative_instance_id,
    const std::string& segment);

CreativeAdNotificationInfo GetCreativeAdNotificationForAdvertiser(
    const std::string& advertiser_id);

CreativeAdNotificationInfo GetCreativeAdNotificationForSegment(
    const std::string& segment);

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_BUNDLE_CREATIVE_AD_NOTIFICATION_UNITTEST_UTIL_H_
