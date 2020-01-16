// Copyright (c) 2019 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

// Types
import { types } from '../constants/top_sites_types'
import { action } from 'typesafe-actions'
import { InitialData } from '../api/initialData'
import { Dispatch } from 'redux'

// API
import {
  fetchAllBookmarkTreeNodes,
  updateBookmarkTreeNode
} from '../api/bookmarks'

export const setFirstRenderTopSitesData = (initialData: InitialData) => {
  return action(types.TOP_SITES_SET_FIRST_RENDER_DATA, initialData)
}

export const topSitesDataUpdated = (gridSites: NewTab.Site[]) => {
  return action(types.TOP_SITES_DATA_UPDATED, { gridSites })
}

export const toggleTopSitePinned = (pinnedSite: NewTab.Site) => {
  return action(types.TOP_SITES_TOGGLE_SITE_PINNED, { pinnedSite })
}

export const removeTopSite = (ignoredSite: NewTab.Site) => {
  return action(types.TOP_SITES_REMOVE_SITE, { ignoredSite })
}

export const undoRemoveTopSite = () => {
  return action(types.TOP_SITES_UNDO_REMOVE_SITE)
}

export const undoRemoveAllTopSites = () => {
  return action(types.TOP_SITES_UNDO_REMOVE_ALL_SITES)
}

export const updateTopSitesBookmarkInfo = (
  sites: chrome.topSites.MostVisitedURL[]
) => {
  return async (dispatch: Dispatch) => {
    const bookmarkInfo = await fetchAllBookmarkTreeNodes(sites)
    dispatch(action(types.TOP_SITES_UPDATE_SITE_BOOKMARK_INFO, {
      bookmarkInfo
    }))
  }
}

export const toggleTopSiteBookmarkInfo = (site: NewTab.Site) => {
  return async (dispatch: Dispatch) => {
    const bookmarkInfo = await updateBookmarkTreeNode(site)
    dispatch(action(types.TOP_SITES_TOGGLE_SITE_BOOKMARK_INFO, {
      url: site.url,
      bookmarkInfo
    }))
  }
}

export const addTopSites = (site: NewTab.Site) => {
  return action(types.TOP_SITES_ADD_SITES, { site })
}

export const showSiteRemovalNotification = (shouldShow: boolean) => {
  return action(types.TOP_SITES_SHOW_SITE_REMOVAL_NOTIFICATION, { shouldShow })
}
