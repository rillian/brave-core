/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/brave_wallet_provider_impl.h"

#include <memory>
#include <utility>
#include <vector>

#include "base/json/json_reader.h"
#include "base/test/bind.h"
#include "base/test/task_environment.h"
#include "base/values.h"
#include "brave/browser/brave_wallet/brave_wallet_provider_delegate_impl.h"
#include "brave/components/brave_wallet/browser/brave_wallet_utils.h"
#include "brave/components/brave_wallet/browser/eth_json_rpc_controller.h"
#include "brave/components/brave_wallet/browser/eth_tx_controller.h"
#include "brave/components/brave_wallet/browser/keyring_controller.h"
#include "brave/components/brave_wallet/browser/pref_names.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "brave/components/brave_wallet/common/web3_provider_constants.h"
#include "chrome/browser/prefs/browser_prefs.h"
#include "chrome/test/base/testing_profile.h"
#include "components/prefs/pref_service.h"
#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_task_environment.h"
#include "content/public/test/test_web_contents_factory.h"
#include "services/network/public/cpp/weak_wrapper_shared_url_loader_factory.h"
#include "services/network/test/test_url_loader_factory.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace brave_wallet {

namespace {

void ValidateErrorCode(BraveWalletProviderImpl* provider,
                       const std::string& payload,
                       ProviderErrors expected) {
  bool callback_is_called = false;
  provider->AddEthereumChain(
      payload, base::BindLambdaForTesting([&callback_is_called, &expected](
                                              bool success,
                                              const std::string& response) {
        auto json_value = base::JSONReader::Read(response);
        ASSERT_FALSE(json_value->FindPath("result"));
        EXPECT_EQ(json_value->FindPath("error.code")->GetInt(),
                  static_cast<int>(expected));
        ASSERT_FALSE(
            json_value->FindPath("error.message")->GetString().empty());
        callback_is_called = true;
      }));
  ASSERT_TRUE(callback_is_called);
}

}  // namespace

class BraveWalletProviderImplUnitTest : public testing::Test {
 public:
  BraveWalletProviderImplUnitTest() {
    TestingProfile::Builder builder;
    auto prefs_service =
        std::make_unique<sync_preferences::TestingPrefServiceSyncable>();
    RegisterUserProfilePrefs(prefs_service->registry());
    builder.SetPrefService(std::move(prefs_service));
    profile_ = builder.Build();

    shared_url_loader_factory_ =
        base::MakeRefCounted<network::WeakWrapperSharedURLLoaderFactory>(
            &url_loader_factory_);
    web_contents_ = factory_.CreateWebContents(profile_.get());

    eth_json_rpc_controller_ = std::make_unique<EthJsonRpcController>(
        shared_url_loader_factory(), prefs());
    auto tx_state_manager = std::make_unique<EthTxStateManager>(
        prefs(), eth_json_rpc_controller_->MakeRemote());
    auto eth_nonce_tracker = std::make_unique<EthNonceTracker>(
        tx_state_manager.get(), eth_json_rpc_controller_.get());
    auto eth_pending_tx_tracker = std::make_unique<EthPendingTxTracker>(
        tx_state_manager.get(), eth_json_rpc_controller_.get(),
        eth_nonce_tracker.get());
    keyring_controller_ = std::make_unique<KeyringController>(prefs());
    eth_tx_controller_ = std::make_unique<EthTxController>(
        eth_json_rpc_controller_.get(), keyring_controller_.get(),
        std::move(tx_state_manager), std::move(eth_nonce_tracker),
        std::move(eth_pending_tx_tracker), prefs());
  }

  ~BraveWalletProviderImplUnitTest() override = default;

  scoped_refptr<network::SharedURLLoaderFactory> shared_url_loader_factory() {
    return shared_url_loader_factory_;
  }

  content::WebContents* web_contents() { return web_contents_; }
  KeyringController* keyring_controller() { return keyring_controller_.get(); }
  EthTxController* eth_tx_controller() { return eth_tx_controller_.get(); }
  EthJsonRpcController* eth_json_rpc_controller() {
    return eth_json_rpc_controller_.get();
  }

  content::BrowserContext* browser_context() { return profile_.get(); }

  PrefService* prefs() { return profile_->GetPrefs(); }

 private:
  std::unique_ptr<EthJsonRpcController> eth_json_rpc_controller_;
  std::unique_ptr<KeyringController> keyring_controller_;
  std::unique_ptr<EthTxController> eth_tx_controller_;

  content::BrowserTaskEnvironment browser_task_environment_;
  std::unique_ptr<TestingProfile> profile_;
  content::TestWebContentsFactory factory_;
  content::WebContents* web_contents_ = nullptr;
  network::TestURLLoaderFactory url_loader_factory_;
  scoped_refptr<network::SharedURLLoaderFactory> shared_url_loader_factory_;
};

TEST_F(BraveWalletProviderImplUnitTest, ValidateBrokenPayloads) {
  BraveWalletProviderImpl provider_impl(
      eth_json_rpc_controller()->MakeRemote(),
      eth_tx_controller()->MakeRemote(),
      std::make_unique<brave_wallet::BraveWalletProviderDelegateImpl>(
          web_contents(), web_contents()->GetMainFrame()),
      prefs());
  ValidateErrorCode(&provider_impl, "", ProviderErrors::kInvalidParams);
  ValidateErrorCode(&provider_impl, R"({})", ProviderErrors::kInvalidParams);
  ValidateErrorCode(&provider_impl, R"({"params": []})",
                    ProviderErrors::kInvalidParams);
  ValidateErrorCode(&provider_impl, R"({"params": [{}]})",
                    ProviderErrors::kInvalidParams);
  ValidateErrorCode(&provider_impl, R"({"params": {}})",
                    ProviderErrors::kInvalidParams);
  ValidateErrorCode(&provider_impl, R"({"params": [{
        "chainName": 'Binance1 Smart Chain',
      }]})",
                    ProviderErrors::kInvalidParams);
  ValidateErrorCode(&provider_impl, R"({"params": [{
      "chainId": '0x386'
    }]})",
                    ProviderErrors::kInvalidParams);
  ValidateErrorCode(&provider_impl, R"({"params": [{
      "rpcUrls": ['https://bsc-dataseed.binance.org/'],
    }]})",
                    ProviderErrors::kInvalidParams);
  ValidateErrorCode(&provider_impl, R"({"params": [{
      "chainName": 'Binance1 Smart Chain',
      "rpcUrls": ['https://bsc-dataseed.binance.org/'],
    }]})",
                    ProviderErrors::kInvalidParams);
}

TEST_F(BraveWalletProviderImplUnitTest, EmptyDelegate) {
  BraveWalletProviderImpl provider_impl(eth_json_rpc_controller()->MakeRemote(),
                                        eth_tx_controller()->MakeRemote(),
                                        nullptr, prefs());
  ValidateErrorCode(&provider_impl,
                    R"({"params": [{
        "chainId": "0x111",
        "chainName": "Binance1 Smart Chain",
        "rpcUrls": ["https://bsc-dataseed.binance.org/"]
      }]})",
                    ProviderErrors::kInternalError);
}

TEST_F(BraveWalletProviderImplUnitTest, OnAddEthereumChain) {
  BraveWalletProviderImpl provider_impl(
      eth_json_rpc_controller()->MakeRemote(),
      eth_tx_controller()->MakeRemote(),
      std::make_unique<brave_wallet::BraveWalletProviderDelegateImpl>(
          web_contents(), web_contents()->GetMainFrame()),
      prefs());
  bool callback_is_called = false;
  provider_impl.AddEthereumChain(
      R"({"params": [{
        "chainId": "0x111",
        "chainName": "Binance1 Smart Chain",
        "rpcUrls": ["https://bsc-dataseed.binance.org/"]
      }]})",
      base::BindLambdaForTesting(
          [&callback_is_called](bool success, const std::string& response) {
            auto json_value = base::JSONReader::Read(response);
            ASSERT_FALSE(json_value->FindPath("result"));
            EXPECT_EQ(json_value->FindPath("error.code")->GetInt(),
                      static_cast<int>(ProviderErrors::kUserRejectedRequest));
            ASSERT_FALSE(
                json_value->FindPath("error.message")->GetString().empty());
            callback_is_called = true;
          }));
  ASSERT_FALSE(callback_is_called);
  provider_impl.OnAddEthereumChain("0x111", false);
  ASSERT_TRUE(callback_is_called);
}

TEST_F(BraveWalletProviderImplUnitTest,
       OnAddEthereumChainRequestCompletedError) {
  BraveWalletProviderImpl provider_impl(
      eth_json_rpc_controller()->MakeRemote(),
      eth_tx_controller()->MakeRemote(),
      std::make_unique<brave_wallet::BraveWalletProviderDelegateImpl>(
          web_contents(), web_contents()->GetMainFrame()),
      prefs());
  int callback_is_called = 0;
  provider_impl.AddEthereumChain(
      R"({"params": [{
        "chainId": "0x111",
        "chainName": "Binance1 Smart Chain",
        "rpcUrls": ["https://bsc-dataseed.binance.org/"]
      }]})",
      base::BindLambdaForTesting(
          [&callback_is_called](bool success, const std::string& response) {
            auto json_value = base::JSONReader::Read(response);
            ASSERT_FALSE(json_value->FindPath("result"));
            EXPECT_EQ(json_value->FindPath("error.code")->GetInt(),
                      static_cast<int>(ProviderErrors::kUserRejectedRequest));
            EXPECT_EQ(json_value->FindPath("error.message")->GetString(),
                      "test message");
            callback_is_called++;
          }));
  EXPECT_EQ(callback_is_called, 0);
  provider_impl.OnAddEthereumChainRequestCompleted("0x111", "test message");
  EXPECT_EQ(callback_is_called, 1);
  provider_impl.OnAddEthereumChainRequestCompleted("0x111", "test message");
  EXPECT_EQ(callback_is_called, 1);
}

TEST_F(BraveWalletProviderImplUnitTest,
       OnAddEthereumChainRequestCompletedSuccess) {
  BraveWalletProviderImpl provider_impl(
      eth_json_rpc_controller()->MakeRemote(),
      eth_tx_controller()->MakeRemote(),
      std::make_unique<brave_wallet::BraveWalletProviderDelegateImpl>(
          web_contents(), web_contents()->GetMainFrame()),
      prefs());
  int callback_is_called = 0;
  provider_impl.AddEthereumChain(
      R"({"params": [{
        "chainId": "0x111",
        "chainName": "Binance1 Smart Chain",
        "rpcUrls": ["https://bsc-dataseed.binance.org/"]
      }]})",
      base::BindLambdaForTesting(
          [&callback_is_called](bool success, const std::string& response) {
            auto json_value = base::JSONReader::Read(response);
            base::Value empty;
            ASSERT_TRUE(json_value->FindPath("result")->Equals(&empty));
            ASSERT_FALSE(json_value->FindPath("error"));
            callback_is_called++;
          }));
  EXPECT_EQ(callback_is_called, 0);
  provider_impl.OnAddEthereumChainRequestCompleted("0x111", "");
  EXPECT_EQ(callback_is_called, 1);
  provider_impl.OnAddEthereumChainRequestCompleted("0x111", "");
  EXPECT_EQ(callback_is_called, 1);
}

}  // namespace brave_wallet
