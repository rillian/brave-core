/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TOKENS_ISSUERS_ISSUERS_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TOKENS_ISSUERS_ISSUERS_H_

#include <map>
#include <set>
#include <string>

#include "bat/ads/public/interfaces/ads.mojom.h"

namespace ads {

class IssuersDelegate;

class Issuers {
 public:
  Issuers();

  ~Issuers();

  void set_delegate(IssuersDelegate* delegate);

  bool HasIssuers();

  void GetIssuers();

  bool PublicKeyExists(const std::string& issuer_name,
                       const std::string& public_key);

 private:
  IssuersDelegate* delegate_ = nullptr;

  void OnGetIssuers(const mojom::UrlResponse& url_response);

  // TODO(tmancey): Remove |public_keys_| as values should be persisted in a
  // database or pref
  std::map<std::string, std::set<std::string>> public_keys_;
  bool ParseResponseBody(const mojom::UrlResponse& url_response);

  void OnDidGetIssuers();
  void OnFailedToGetIssuers();
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_TOKENS_ISSUERS_ISSUERS_H_
