/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/extensions/brave_webtorrent_navigation_throttle.h"

#include "brave/common/brave_wallet_constants.h"
#include "brave/common/extensions/extension_constants.h"
#include "brave/common/pref_names.h"
#include "components/prefs/pref_change_registrar.h"
#include "components/prefs/pref_service.h"
#include "extensions/browser/extensions_browser_client.h"

namespace extensions {

Web3ProviderController::Web3ProviderController(
        content::BrowserContext* browser_context) :
    browser_context_(browser_context) {
  PrefService* prefs=
      ExtensionsBrowserClient::Get()->GetPrefServiceForContext(
          browser_context);
  LOG(ERROR) << "======Registering for Brave Wallet web3 provider listneere";
  pref_change_registrar_ = std::make_unique<PrefChangeRegistrar>();
  pref_change_registrar_->Init(prefs);
  pref_change_registrar_->Add(kBraveWalletWeb3Provider,
    base::Bind(&Web3ProviderController::OnPreferenceChanged,
        base::Unretained(this)));
}

Web3ProviderController::~Web3ProviderController() {
}

void Web3ProviderController::OnPreferenceChanged() {
  LOG(ERROR) << "======PREF changed web3 provider listneere";
  PrefService* prefs =
      ExtensionsBrowserClient::Get()->GetPrefServiceForContext(
          browser_context());
  auto provider = static_cast<BraveWalletWeb3ProviderTypes>(
      prefs->GetInteger(kBraveWalletWeb3Provider));
  if (provider == BraveWalletWeb3ProviderTypes::CRYPTO_WALLETS) {
    LOG(ERROR) << "======Web3ProviderController:: Brave Crypto Wallets extension id selected";
  } else if (provider == BraveWalletWeb3ProviderTypes::METAMASK) {
    LOG(ERROR) << "======Web3ProviderController:: METAMASK extension id selected";
  }
}

void Web3ProviderController::OnExtensionLoaded(
    content::BrowserContext* browser_context, const Extension* extension) {
  LOG(ERROR) << "======Web3ProviderController:: on extension loaded: " << extension->id();
}

}  // namespace extensions

