/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/extensions/browser/brave_extension_user_script_loader.h"

#include "brave/common/brave_wallet_constants.h"
#include "brave/common/extensions/extension_constants.h"
#include "brave/common/pref_names.h"
#include "components/prefs/pref_change_registrar.h"
#include "components/prefs/pref_service.h"
#include "extensions/browser/extensions_browser_client.h"

namespace extensions {

BraveExtensionUserScriptLoader::BraveExtensionUserScriptLoader(
        content::BrowserContext* browser_context,
        const HostID& host_id, bool listen_for_extension_system_loaded) :
    ExtensionUserScriptLoader(browser_context, host_id,
        listen_for_extension_system_loaded) {
  PrefService* prefs=
      ExtensionsBrowserClient::Get()->GetPrefServiceForContext(
          browser_context);
  LOG(ERROR) << "======Registering for Brave Wallet web3 provider listneere";
  pref_change_registrar_ = std::make_unique<PrefChangeRegistrar>();
  pref_change_registrar_->Init(prefs);
  pref_change_registrar_->Add(kBraveWalletWeb3Provider,
    base::Bind(&BraveExtensionUserScriptLoader::OnPreferenceChanged,
        base::Unretained(this)));
}

BraveExtensionUserScriptLoader::~BraveExtensionUserScriptLoader() {
}

void OnExtensionLoaded(content::BrowserContext* browser_context,
                       const Extension* extension) {
  LOG(ERROR) << "======Brave extension user script loader -- extension loaded";

}

void BraveExtensionUserScriptLoader::OnPreferenceChanged() {
  LOG(ERROR) << "======PREF changed web3 provider listneere";
  PrefService* prefs=
      ExtensionsBrowserClient::Get()->GetPrefServiceForContext(
          browser_context());
  auto provider = static_cast<BraveWalletWeb3ProviderTypes>(
      prefs->GetInteger(kBraveWalletWeb3Provider));
  if (provider == BraveWalletWeb3ProviderTypes::CRYPTO_WALLETS) {
    LOG(ERROR) << "======BraveExtensionUserScriptLoader:: Brave Crypto Wallets extension id selected";
  } else if (provider == BraveWalletWeb3ProviderTypes::METAMASK) {
    LOG(ERROR) << "======BraveExtensionUserScriptLoader:: METAMASK extension id selected";
  }
}

}  // namespace extensions
