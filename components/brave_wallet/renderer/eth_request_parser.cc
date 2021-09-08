/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/renderer/eth_request_parser.h"

#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "brave/components/brave_wallet/common/web3_provider_constants.h"

namespace brave_wallet {

absl::optional<base::Value> GetObjectFromParamsList(const std::string& json) {
  auto json_value = base::JSONReader::Read(json);
  if (!json_value || !json_value->is_dict()) {
    // Invalid parameters.
    return absl::nullopt;
  }

  const base::Value* params = json_value->FindListPath(brave_wallet::kParams);
  if (!params || !params->is_list()) {
    // Invalid parameters.
    return absl::nullopt;
  }
  const auto list = params->GetList();
  if (list.empty() || !list.front().is_dict()) {
    // Expected single, object parameter.
    return absl::nullopt;
  }

  return list.front().Clone();
}

mojom::TxDataPtr ValueToTxData(const base::Value& tx_value,
                               std::string* from_out) {
  CHECK(from_out);
  auto tx_data = mojom::TxData::New();
  const base::DictionaryValue* params_dict = nullptr;
  if (!tx_value.GetAsDictionary(&params_dict) || !params_dict)
    return nullptr;

  const std::string* from = params_dict->FindStringKey("from");
  if (!from) {
    return nullptr;
  }
  *from_out = *from;

  const std::string* to = params_dict->FindStringKey("to");
  if (to) {
    tx_data->to = *to;
  } else {
    tx_data->to = "0x0000000000000000000000000000000000000000";
  }

  const std::string* gas = params_dict->FindStringKey("gas");
  if (gas) {
    tx_data->gas_limit = *gas;
  }

  const std::string* gas_price = params_dict->FindStringKey("gasPrice");
  if (gas_price) {
    tx_data->gas_price = *gas_price;
  }

  const std::string* value = params_dict->FindStringKey("value");
  if (value) {
    tx_data->value = *value;
  }

  const std::string* data = params_dict->FindStringKey("data");
  // TODO(bbondy): Better validation on data here, will require refactor
  // of brave_wallet_utils to common
  if (data && data->length() > 2 && data->substr(0, 2) == "0x") {
    std::string hex_substr = data->substr(2);
    std::vector<uint8_t> bytes;
    if (!base::HexStringToBytes(hex_substr, &bytes)) {
      return nullptr;
    }
    tx_data->data = bytes;
  }

  // `to` is required unless there's contract creation happening
  if (tx_data->data.size() == 0 && !to) {
    return nullptr;
  }

  return tx_data;
}

mojom::TxDataPtr ParseEthSendTransactionParams(const std::string& json,
                                               std::string* from) {
  CHECK(from);
  from->clear();

  auto param_obj = GetObjectFromParamsList(json);
  if (!param_obj) {
    return nullptr;
  }
  return brave_wallet::ValueToTxData(*param_obj, from);
}

mojom::TxData1559Ptr ParseEthSendTransaction1559Params(const std::string& json,
                                                       std::string* from) {
  CHECK(from);
  from->clear();
  auto param_obj = GetObjectFromParamsList(json);
  if (!param_obj) {
    return nullptr;
  }

  auto tx_data = mojom::TxData1559::New();
  *tx_data->base_data = *brave_wallet::ValueToTxData(*param_obj, from);

  const base::DictionaryValue* params_dict = nullptr;
  if (!param_obj->GetAsDictionary(&params_dict) || !params_dict)
    return nullptr;

  const std::string* max_priority_fee_per_gas =
      params_dict->FindStringKey("maxPriorityFeePerGas");
  if (max_priority_fee_per_gas) {
    tx_data->max_priority_fee_per_gas = *max_priority_fee_per_gas;
  }

  const std::string* max_fee_per_gas =
      params_dict->FindStringKey("maxFeePerGas");
  if (max_fee_per_gas) {
    tx_data->max_fee_per_gas = *max_fee_per_gas;
  }

  return nullptr;
}

}  // namespace brave_wallet
