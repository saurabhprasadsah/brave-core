j/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_observer.h"

class PrefChangeRegistrar;

namespace extensions {

class BraveExtensionUserScriptLoader : public ExtensionRegistryObserver
 public:
  BraveExtensionUserScriptLoader(content::BrowserContext* browser_context);
  ~BraveExtensionUserScriptLoader() override;
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
  DISALLOW_COPY_AND_ASSIGN(BraveExtensionUserScriptLoader);
};

}  // namespace extensions
