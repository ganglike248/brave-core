// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/browser/ui/webui/brave_shields/shields_panel_data_handler.h"

#include <utility>

#include "base/check.h"
#include "brave/browser/brave_browser_process.h"
#include "brave/browser/ui/webui/webcompat_reporter/webcompat_reporter_dialog.h"
#include "brave/components/brave_shields/content/browser/ad_block_service.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/webui/top_chrome/top_chrome_web_ui_controller.h"

using brave_shields::BraveShieldsTabHelper;
using brave_shields::mojom::SiteSettings;

ShieldsPanelDataHandler::ShieldsPanelDataHandler(
    mojo::PendingReceiver<brave_shields::mojom::DataHandler>
        data_handler_receiver,
    TopChromeWebUIController* webui_controller,
    TabStripModel* tab_strip_model)
    : data_handler_receiver_(this, std::move(data_handler_receiver)),
      webui_controller_(webui_controller) {
  DCHECK(tab_strip_model);
  tab_strip_model->AddObserver(this);

  auto* web_contents = tab_strip_model->GetActiveWebContents();
  if (!web_contents) {
    return;
  }
  active_shields_data_controller_ =
      BraveShieldsTabHelper::FromWebContents(web_contents);
  if (!active_shields_data_controller_) {
    return;
  }

  UpdateSiteBlockInfo();
  active_shields_data_controller_->AddObserver(this);
}

ShieldsPanelDataHandler::~ShieldsPanelDataHandler() {
  /* The lifecycle of this class is similar to ShieldsPanelUI and
   * ShieldsPanelUI's cache gets destryed after ~300ms of being idle.
   */
  if (!active_shields_data_controller_) {
    return;
  }

  active_shields_data_controller_->RemoveObserver(this);
  active_shields_data_controller_ = nullptr;
}

void ShieldsPanelDataHandler::RegisterUIHandler(
    mojo::PendingRemote<brave_shields::mojom::UIHandler> ui_handler_receiver) {
  ui_handler_remote_.Bind(std::move(ui_handler_receiver));
  UpdateSiteBlockInfo();
}

void ShieldsPanelDataHandler::GetSiteBlockInfo(
    GetSiteBlockInfoCallback callback) {
  std::move(callback).Run(site_block_info_.Clone());
}

void ShieldsPanelDataHandler::GetSiteSettings(
    GetSiteSettingsCallback callback) {
  if (!active_shields_data_controller_) {
    return;
  }

  SiteSettings settings;
  settings.ad_block_mode = active_shields_data_controller_->GetAdBlockMode();
  settings.fingerprint_mode =
      active_shields_data_controller_->GetFingerprintMode();
  settings.cookie_block_mode =
      active_shields_data_controller_->GetCookieBlockMode();
  settings.https_upgrade_mode =
      active_shields_data_controller_->GetHttpsUpgradeMode();
  settings.is_noscript_enabled =
      active_shields_data_controller_->GetNoScriptEnabled();
  settings.is_forget_first_party_storage_enabled =
      active_shields_data_controller_->GetForgetFirstPartyStorageEnabled();
  settings.webcompat_settings =
      active_shields_data_controller_->GetWebcompatSettings();

  std::move(callback).Run(settings.Clone());
}

void ShieldsPanelDataHandler::SetAdBlockMode(AdBlockMode mode) {
  if (!active_shields_data_controller_) {
    return;
  }

  active_shields_data_controller_->SetAdBlockMode(mode);
}

void ShieldsPanelDataHandler::SetFingerprintMode(FingerprintMode mode) {
  if (!active_shields_data_controller_) {
    return;
  }

  active_shields_data_controller_->SetFingerprintMode(mode);
}

void ShieldsPanelDataHandler::SetCookieBlockMode(CookieBlockMode mode) {
  if (!active_shields_data_controller_) {
    return;
  }

  active_shields_data_controller_->SetCookieBlockMode(mode);
}

void ShieldsPanelDataHandler::SetHttpsUpgradeMode(HttpsUpgradeMode mode) {
  if (!active_shields_data_controller_) {
    return;
  }

  active_shields_data_controller_->SetHttpsUpgradeMode(mode);
}

void ShieldsPanelDataHandler::SetIsNoScriptsEnabled(bool is_enabled) {
  if (!active_shields_data_controller_) {
    return;
  }

  active_shields_data_controller_->SetIsNoScriptEnabled(is_enabled);
}

void ShieldsPanelDataHandler::AllowScriptsOnce(
    const std::vector<std::string>& origins) {
  if (!active_shields_data_controller_) {
    return;
  }

  active_shields_data_controller_->AllowScriptsOnce(origins);
}

void ShieldsPanelDataHandler::BlockAllowedScripts(
    const std::vector<std::string>& origins) {
  if (!active_shields_data_controller_) {
    return;
  }

  active_shields_data_controller_->BlockAllowedScripts(origins);
}

void ShieldsPanelDataHandler::SetBraveShieldsEnabled(bool is_enabled) {
  if (!active_shields_data_controller_) {
    return;
  }

  active_shields_data_controller_->SetBraveShieldsEnabled(is_enabled);
}

void ShieldsPanelDataHandler::SetForgetFirstPartyStorageEnabled(
    bool is_enabled) {
  if (!active_shields_data_controller_) {
    return;
  }

  active_shields_data_controller_->SetForgetFirstPartyStorageEnabled(
      is_enabled);
}

void ShieldsPanelDataHandler::SetWebcompatEnabled(
    ContentSettingsType webcompat_settings_type,
    bool enable) {
  if (!active_shields_data_controller_) {
    return;
  }

  active_shields_data_controller_->SetWebcompatEnabled(webcompat_settings_type,
                                                       enable);
}

void ShieldsPanelDataHandler::OpenWebCompatWindow() {
  if (!active_shields_data_controller_) {
    return;
  }

  webcompat_reporter::OpenReporterDialog(
      active_shields_data_controller_->web_contents(),
      webcompat_reporter::UISource::kShieldsPanel);
}

void ShieldsPanelDataHandler::AreAnyBlockedElementsPresent(
    AreAnyBlockedElementsPresentCallback callback) {
  if (!active_shields_data_controller_) {
    return;
  }

  std::move(callback).Run(
      g_brave_browser_process->ad_block_service()->AreAnyBlockedElementsPresent(
          active_shields_data_controller_->web_contents()->GetURL().host()));
}

void ShieldsPanelDataHandler::ResetBlockedElements() {
  webui_controller_->embedder()->CloseUI();

  if (!active_shields_data_controller_) {
    return;
  }

  g_brave_browser_process->ad_block_service()->ResetCosmeticFilter(
      active_shields_data_controller_->web_contents()->GetURL().host());

  active_shields_data_controller_->web_contents()->GetController().Reload(
      content::ReloadType::NORMAL, true);
}

void ShieldsPanelDataHandler::UpdateFavicon() {
  if (!active_shields_data_controller_) {
    return;
  }

  // TODO(nullhook): Don't update favicon if previous site is the current site
  site_block_info_.favicon_url =
      active_shields_data_controller_->GetFaviconURL(true);

  // Notify remote that favicon changed
  if (ui_handler_remote_) {
    ui_handler_remote_.get()->OnSiteBlockInfoChanged(site_block_info_.Clone());
  }
}

void ShieldsPanelDataHandler::UpdateSiteBlockInfo() {
  if (!active_shields_data_controller_) {
    return;
  }

  site_block_info_.host =
      active_shields_data_controller_->GetCurrentSiteURL().host();
  site_block_info_.total_blocked_resources =
      active_shields_data_controller_->GetTotalBlockedCount();
  site_block_info_.ads_list =
      active_shields_data_controller_->GetBlockedAdsList();
  site_block_info_.blocked_js_list =
      active_shields_data_controller_->GetBlockedJsList();
  site_block_info_.allowed_js_list =
      active_shields_data_controller_->GetAllowedJsList();
  site_block_info_.fingerprints_list =
      active_shields_data_controller_->GetFingerprintsList();
  site_block_info_.http_redirects_list =
      active_shields_data_controller_->GetHttpRedirectsList();
  site_block_info_.is_brave_shields_enabled =
      active_shields_data_controller_->GetBraveShieldsEnabled();
  site_block_info_.is_brave_shields_managed =
      active_shields_data_controller_->IsBraveShieldsManaged();
  const auto& invoked_webcompat_set =
      active_shields_data_controller_->GetInvokedWebcompatFeatures();
  site_block_info_.invoked_webcompat_list = std::vector<ContentSettingsType>(
      invoked_webcompat_set.begin(), invoked_webcompat_set.end());

  // This method gets called from various callsites. Constantly updating favicon
  // url will replace the hashed version too. So, we update this once only
  if (site_block_info_.favicon_url.is_empty()) {
    site_block_info_.favicon_url =
        active_shields_data_controller_->GetFaviconURL(false);
  }

  // Notify remote that data changed
  if (ui_handler_remote_) {
    ui_handler_remote_.get()->OnSiteBlockInfoChanged(site_block_info_.Clone());
  }
}

void ShieldsPanelDataHandler::OnResourcesChanged() {
  UpdateSiteBlockInfo();
}

void ShieldsPanelDataHandler::OnFaviconUpdated() {
  UpdateFavicon();
}

void ShieldsPanelDataHandler::OnTabStripModelChanged(
    TabStripModel* tab_strip_model,
    const TabStripModelChange& change,
    const TabStripSelectionChange& selection) {
  if (selection.active_tab_changed()) {
    // To make logic simpler, always remove observer when active tab changed.
    // And then, start observing when there is new active web contents.
    if (active_shields_data_controller_) {
      active_shields_data_controller_->RemoveObserver(this);
      active_shields_data_controller_ = nullptr;
    }

    if (selection.new_contents) {
      active_shields_data_controller_ =
          BraveShieldsTabHelper::FromWebContents(selection.new_contents);
      active_shields_data_controller_->AddObserver(this);

      // OnResourcesChanged doesnt get triggered instantly on active tab change
      // so trigger this explicitly. Call this after new
      // |active_shields_data_controller_| is set.
      UpdateSiteBlockInfo();
    }
  }
}
