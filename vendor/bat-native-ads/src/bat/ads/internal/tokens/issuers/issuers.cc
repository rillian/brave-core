/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/tokens/issuers/issuers.h"

#include <utility>

#include "base/bind.h"
#include "base/check_op.h"
#include "base/json/json_reader.h"
#include "base/values.h"
#include "bat/ads/internal/ads_client_helper.h"
#include "bat/ads/internal/logging.h"
#include "bat/ads/internal/logging_util.h"
#include "bat/ads/internal/tokens/issuers/get_issuers_url_request_builder.h"
#include "bat/ads/internal/tokens/issuers/issuers_delegate.h"
#include "net/http/http_status_code.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace ads {

Issuers::Issuers() = default;

Issuers::~Issuers() {
  delegate_ = nullptr;
}

void Issuers::set_delegate(IssuersDelegate* delegate) {
  DCHECK_EQ(delegate_, nullptr);
  delegate_ = delegate;
}

bool Issuers::HasIssuers() {
  if (public_keys_.empty()) {
    return false;
  }

  return true;
}

// TODO(tmancey): How often should we fetch the latest issuers?
void Issuers::GetIssuers() {
  BLOG(1, "GetIssuers");
  BLOG(2, "GET /v1/issuers/");

  GetIssuersUrlRequestBuilder url_request_builder;
  mojom::UrlRequestPtr url_request = url_request_builder.Build();
  BLOG(6, UrlRequestToString(url_request));
  BLOG(7, UrlRequestHeadersToString(url_request));

  const auto callback =
      std::bind(&Issuers::OnGetIssuers, this, std::placeholders::_1);
  AdsClientHelper::Get()->UrlRequest(std::move(url_request), callback);
}

bool Issuers::PublicKeyExists(const std::string& issuer_name,
                              const std::string& public_key) {
  const auto iter = public_keys_.find(issuer_name);
  if (iter == public_keys_.end()) {
    return false;
  }

  // TODO(tmancey): Refactor
  return static_cast<bool>(iter->second.count(public_key));
}

//////////////////////////////////////////////////////////////////////////////

void Issuers::OnGetIssuers(const mojom::UrlResponse& url_response) {
  BLOG(1, "OnGetIssuers");

  BLOG(6, UrlResponseToString(url_response));
  BLOG(7, UrlResponseHeadersToString(url_response));

  if (url_response.status_code != net::HTTP_OK) {
    BLOG(0, "Failed to get issuers");
    OnFailedToGetIssuers();
    return;
  }

  const bool success = ParseResponseBody(url_response);
  if (!success) {
    BLOG(3, "Failed to parse response: " << url_response.body);
    OnFailedToGetIssuers();
    return;
  }

  OnDidGetIssuers();
}

bool Issuers::ParseResponseBody(const mojom::UrlResponse& url_response) {
  // TODO(tmancey): Decouple JSONReader to issuers_json_reader.h/.cc
  std::map<std::string, std::set<std::string>> public_keys;

  absl::optional<base::Value> issuer_list =
      base::JSONReader::Read(url_response.body);

  if (!issuer_list || !issuer_list->is_list()) {
    return false;
  }

  // TODO(tmancey): Refactor as seems over complicated to parse the following
  // JSON:
  //
  // [
  //   {
  //     "name": "confirmations",
  //     "publicKeys": [
  //       "cKo0rk1iS8Obgyni0X3RRoydDIGHsivTkfX/TM1Xl24="
  //     ]
  //   },
  //   {
  //     "name": "payments",
  //     "publicKeys": [
  //       "UHrWoG29ze91jRE4F0sJJRV6aFAgHndO8KYqHRKPbG4=",
  //       "GNJKVR2NuU0qPp6fZzH6of60JPoGkzuYMXQ5RLbo7TU=",
  //       "OAhrcTws9LZ15PezS7diPXwElMMgyNuVfFwmOyJyDhM=",
  //       "lKTtvRtX6wzVJ6yinrSwL4ZOUFFzq0fgWl+w8uV68yk=",
  //       "gJAuDO7BXkLmlFoxQnmblaIxxovZBsHngBkR+0dJ8hQ=",
  //       "AjgaKCHwdzVmrQVl8+afkEabY4Lu7H3UEPo9jraBWiU=",
  //       "VMKSj4AeyPwAF4AuS7AN+1stD/ZN0R9tW9WEtXXN6HM=",
  //       "3t9vPWOt5lN60Zjsmr76fNDLfOzxYIq3vKjWdM9X/Bk=",
  //       "ooqYLwDv/vLWeuGX+I1/D8mR3LeN2ioKCEHSBnlHkmw="
  //     ]
  //   }
  // ]

  for (const auto& value : issuer_list->GetList()) {
    if (!value.is_dict()) {
      return false;
    }

    const base::Value* public_key_dict = value.FindPath("");
    const std::string* public_key_name = public_key_dict->FindStringKey("name");
    if (!public_key_name) {
      continue;
    }

    std::set<std::string> current_public_keys;
    const base::Value* public_key_list =
        public_key_dict->FindListKey("publicKeys");
    for (const auto& public_key : public_key_list->GetList()) {
      if (!public_key.is_string()) {
        continue;
      }
      current_public_keys.insert(public_key.GetString());
    }

    public_keys[*public_key_name] = current_public_keys;
  }

  public_keys_ = public_keys;

  return true;
}

void Issuers::OnDidGetIssuers() {
  if (!delegate_) {
    return;
  }

  delegate_->OnDidGetIssuers();
}

void Issuers::OnFailedToGetIssuers() {
  if (!delegate_) {
    return;
  }

  delegate_->OnFailedToGetIssuers();
}

}  // namespace ads
