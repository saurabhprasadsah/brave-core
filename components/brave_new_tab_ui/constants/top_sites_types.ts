// Copyright (c) 2019 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

export const enum types {
  TOP_SITES_SET_FIRST_RENDER_DATA = '@@topSites/TOP_SITES_SET_FIRST_RENDER_DATA',
  TOP_SITES_DATA_UPDATED = '@@topSites/TOP_SITES_DATA_UPDATED',
  TOP_SITES_TOGGLE_SITE_PINNED = '@@topSites/TOP_SITES_SITE_PINNED',
  TOP_SITES_REMOVE_SITE = '@@topSites/TOP_SITES_REMOVE_SITE',
  TOP_SITES_UNDO_REMOVE_SITE = '@@topSites/TOP_SITES_UNDO_REMOVE_SITE',
  TOP_SITES_UNDO_REMOVE_ALL_SITES =
    '@@topSites/TOP_SITES_UNDO_REMOVE_ALL_SITES',
  TOP_SITES_UPDATE_SITE_BOOKMARK_INFO =
    '@@topSites/TOP_SITES_UPDATE_SITE_BOOKMARK_INFO',
  TOP_SITES_TOGGLE_SITE_BOOKMARK_INFO =
    '@@topSites/TOP_SITES_TOGGLE_SITE_BOOKMARK_INFO',
  TOP_SITES_ADD_SITES = '@@topSites/TOP_SITES_ADD_SITES',
  TOP_SITES_SHOW_SITE_REMOVAL_NOTIFICATION =
    '@@topSites/TOP_SITES_SHOW_SITE_REMOVAL_NOTIFICATION'
}
