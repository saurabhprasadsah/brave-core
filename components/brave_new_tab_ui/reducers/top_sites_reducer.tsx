// Copyright (c) 2019 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

// Redux API
import { Reducer } from 'redux'

// Types
import { types } from '../constants/top_sites_types'

// API
import * as topSitesState from '../state/topSitesState'
import * as storage from '../storage'

const initialState = storage.load()

export const topSitesReducer: Reducer<NewTab.State | undefined> = (
  state: NewTab.State | undefined,
  action
) => {
  if (state === undefined) {
    state = initialState
  }

  const payload = action.payload
  const startingState = state

  switch (action.type) {
    case types.TOP_SITES_SET_FIRST_RENDER_DATA: {
      state = topSitesState
        .topSitesReducerSetFirstRenderData(state, payload.topSites)
      break
    }

    case types.TOP_SITES_DATA_UPDATED: {
      state = topSitesState
        .topSitesReducerDataUpdated(state, payload.gridSites)
      break
    }

    case types.TOP_SITES_TOGGLE_SITE_PINNED: {
      state = topSitesState
        .topSitesReducerToggleTopSitePinned(state, payload.pinnedSite)
      break
    }

    case types.TOP_SITES_REMOVE_SITE: {
      state = topSitesState
        .topSitesReducerRemoveSite(state, payload.ignoredSite)
      break
    }

    case types.TOP_SITES_UNDO_REMOVE_SITE: {
      state = topSitesState
        .topSitesReducerUndoRemoveSite(state)
      break
    }

    case types.TOP_SITES_UNDO_REMOVE_ALL_SITES: {
      state = topSitesState
        .topSitesReducerUndoRemoveAllSites(state)
      break
    }

    case types.TOP_SITES_UPDATE_SITE_BOOKMARK_INFO: {
      state = topSitesState
        .topSitesReducerUpdateTopSiteBookmarkInfo(state, payload.bookmarkInfo)
      break
    }

    case types.TOP_SITES_TOGGLE_SITE_BOOKMARK_INFO: {
      state = topSitesState
        .topSitesReducerToggleTopSiteBookmarkInfo(
          state,
          payload.url,
          payload.bookmarkInfo
        )
      break
    }

    case types.TOP_SITES_ADD_SITES: {
      state = topSitesState.topSitesReducerAddSites(state, [payload.site])
      break
    }

    case types.TOP_SITES_SHOW_SITE_REMOVAL_NOTIFICATION: {
      state = topSitesState
        .topSitesReducerShowSiteRemovalNotification(state, payload.shouldShow)
      break
    }
  }

  if (JSON.stringify(state) !== JSON.stringify(startingState)) {
    storage.debouncedSave(state)
  }

  return state
}

export default topSitesReducer
