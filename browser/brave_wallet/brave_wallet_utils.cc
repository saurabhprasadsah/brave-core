/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/widevine/widevine_utils.h"

#include "brave/common/brave_wallet_constants.h"
#include "brave/common/extensions/extension_constants.h"
#include "brave/common/pref_names.h"
#include "chrome/browser/profiles/profile.h"
#include "components/prefs/pref_registry_simple.h"
#include "extensions/browser/extension_prefs.h"

namespace brave_wallet {

void MigrateBraveWalletPrefs(Profile* profile) {
  LOG(ERROR) << "===checking if should do migration of prefs";
  PrefService* prefs = profile->GetPrefs();
  if (!prefs->HasPrefPath(kBraveWalletEnabledDeprecated)) {
    LOG(ERROR) << "===Skipping brave wallet enabled check because it doesn't exist";
    return;
  }
  LOG(ERROR) << "====proceeding with brave wallet enabled migration";
  bool wallet_enabled = prefs->GetBoolean(kBraveWalletEnabledDeprecated);
  bool has_crypto_wallets = extensions::ExtensionPrefs::Get(profile)->
      HasPrefForExtension(ethereum_remote_client_extension_id);
  bool has_metamask = extensions::ExtensionPrefs::Get(profile)->
      HasPrefForExtension(metamask_extension_id);

  BraveWalletWeb3ProviderTypes provider =
      BraveWalletWeb3ProviderTypes::ASK;
  if (!wallet_enabled && has_metamask) {
    // If Crypto Wallets was disabled and MetaMask is installed, set to MetaMask
    provider = BraveWalletWeb3ProviderTypes::METAMASK;
  } else if (!wallet_enabled && !has_metamask) {
    // If Crypto Wallets is diabled, and MetaMask not installed, set None
    provider = BraveWalletWeb3ProviderTypes::NONE;
  } else if (wallet_enabled && has_metamask) {
    // If Crypto Wallets is enabled, and MetaMask is installed, set
    // to Crypto Wallets
    provider = BraveWalletWeb3ProviderTypes::CRYPTO_WALLETS;
  } else if (has_crypto_wallets && wallet_enabled) {
    // If CryptoWallets is enabled and installed, but MetaMask is not
    // installed, set Crypto Wallets.
    provider = BraveWalletWeb3ProviderTypes::CRYPTO_WALLETS;
  } else if (!has_crypto_wallets && wallet_enabled) {
    // If CryptoWallets is enabled and not installed yet, and MetaMask is not
    // installed, set Ask
    provider = BraveWalletWeb3ProviderTypes::ASK;
  }
  prefs->SetInteger(kBraveWalletWeb3Provider, static_cast<int>(provider));
  prefs->ClearPref(kBraveWalletEnabledDeprecated);
}

}  // brave_wallet
