/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_WEB3_PROVIDER_BROWSER_WEB3_PROVIDER_CONTROLLER_H_
#define BRAVE_COMPONENTS_WEB3_PROVIDER_BROWSER_WEB3_PROVIDER_CONTROLLER_H_

#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_observer.h"

class PrefChangeRegistrar;

namespace extensions {

class Web3ProviderController : public ExtensionRegistryObserver
  
 public:
  Web3ProviderController(content::BrowserContext* browser_context);
  ~Web3ProviderController() override;
  void OnPreferenceChanged();

 protected:
  // ExtensionRegistryObserver implementation.
  void OnExtensionLoaded(content::BrowserContext* browser_context,
                         const Extension* extension) override;

  // The browser_context for which the scripts managed here are installed.
  content::BrowserContext* browser_context_;
  std::unique_ptr<PrefChangeRegistrar> pref_change_registrar_;
  ScopedObserver<ExtensionRegistry, ExtensionRegistryObserver>
      extension_registry_observer_{this};

  DISALLOW_COPY_AND_ASSIGN(Web3ProviderController);
};

}  // namespace extensions

#endif  // BRAVE_COMPONENTS_WEB3_PROVIDER_BROWSER_WEB3_PROVIDER_CONTROLLER_H_
