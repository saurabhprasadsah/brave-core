// Copyright (c) 2019 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import {
  generateTopSiteProperties,
  isExistingSite,
  getTopSitesWhitelist,
  isTopSitePinned,
  isTopSiteBookmarked
} from '../helpers/newTabUtils'

export function topSitesReducerSetFirstRenderData (
  state: NewTab.State,
  topSites: chrome.topSites.MostVisitedURL[]
): NewTab.State {
  const topSitesWhitelist = getTopSitesWhitelist(topSites)
  const newGridSites: NewTab.Site[] = []
  for (const [index, topSite] of topSitesWhitelist.entries()) {
    if (isExistingSite(state.gridSites, topSite)) {
      // if topSite from Chromium exists in our gridSites list,
      // skip and iterate over the next item
      continue
    }
    newGridSites.push(generateTopSiteProperties(topSite, index))
  }
  state = topSitesReducerAddSites(state, newGridSites)
  return state
}

export function topSitesReducerDataUpdated (
  state: NewTab.State,
  sitesData: NewTab.Site[]
): NewTab.State {
  let updatedGridSites: NewTab.Site[] = []
  // iterate over all items and populate a new array with them.
  // if an item is pinned, add it to its original index inside
  // the new array without removing anything. this will push other
  // items to the right, which is what we want
  for (const site of sitesData) {
    if (site.pinnedIndex === undefined) {
      updatedGridSites.push(site)
    } else {
      updatedGridSites.splice(site.pinnedIndex, 0, site)
    }
  }
  state = { ...state, gridSites: updatedGridSites }
  return state
}

export function topSitesReducerToggleTopSitePinned (
  state: NewTab.State,
  pinnedSite: NewTab.Site
): NewTab.State {
  const updatedGridSites: NewTab.Site[] = []
  for (const [index, gridSite] of state.gridSites.entries()) {
    if (gridSite.url === pinnedSite.url) {
      updatedGridSites.push({
        ...gridSite,
        pinnedIndex: isTopSitePinned(gridSite) ? undefined : index
      })
    } else {
      updatedGridSites.push(gridSite)
    }
  }
  state = topSitesReducerDataUpdated(state, updatedGridSites)
  return state
}

export function topSitesReducerRemoveSite (
  state: NewTab.State,
  ignoredSite: NewTab.Site
): NewTab.State {
  state = {
    ...state,
    ignoredTopSites: [ ...state.ignoredTopSites, ignoredSite ]
  }

  const filterExcludedFromGridSites = state.gridSites
    .filter((site: NewTab.Site) => {
      // in the updatedGridSites we only want sites not excluded by the user
      return state.ignoredTopSites
        .every((excludedSite: NewTab.Site) => excludedSite.url !== site.url)
    })
  state = topSitesReducerDataUpdated(state, filterExcludedFromGridSites)
  return state
}

export function topSitesReducerUndoRemoveSite (
  state: NewTab.State
): NewTab.State {
  // get the last item from the array
  const removedItem: NewTab.Site | undefined
    = state.ignoredTopSites[state.ignoredTopSites.length - 1]
  // modify and remove it from the ignored list
  state.ignoredTopSites.pop()

  // push back the item into the grid array by adding the site
  state = topSitesReducerAddSites(state, [removedItem])
  return state
}

export function topSitesReducerUndoRemoveAllSites (
  state: NewTab.State
): NewTab.State {
  // erase all ignored sites data
  const allRemovedSites: NewTab.Site[] = state.ignoredTopSites
  // remove all sites from the exclusion list
  state = { ...state, ignoredTopSites: [] }
  // add them all back into grid
  state = topSitesReducerAddSites(state, allRemovedSites)
  return state
}

export const topSitesReducerUpdateTopSiteBookmarkInfo = (
  state: NewTab.State,
  bookmarkInfo: chrome.bookmarks.BookmarkTreeNode
): NewTab.State => {
  const updatedGridSites: NewTab.Site[] = []
  for (const [index, gridSite] of state.gridSites.entries()) {
    const updatedBookmarkTreeNode = bookmarkInfo[index]
    if (
      updatedBookmarkTreeNode !== undefined &&
      gridSite.url === updatedBookmarkTreeNode.url
    ) {
      updatedGridSites.push({
        ...gridSite,
        bookmarkInfo: updatedBookmarkTreeNode
      })
    } else {
      updatedGridSites.push(gridSite)
    }
  }
  state = topSitesReducerDataUpdated(state, updatedGridSites)
  return state
}

export const topSitesReducerToggleTopSiteBookmarkInfo = (
  state: NewTab.State,
  url: string,
  bookmarkInfo: chrome.bookmarks.BookmarkTreeNode
): NewTab.State => {
  const updatedGridSites: NewTab.Site[] = []
  for (const gridSite of state.gridSites) {
    if (url === gridSite.url) {
      updatedGridSites.push({
        ...gridSite,
        bookmarkInfo: isTopSiteBookmarked(bookmarkInfo)
          ? undefined
          // Add a transitory state for bookmarks.
          // This will be overriden by a new mount and is used
          // as a secondary render until data is ready,
          : { title: gridSite.title, id: 'TEMPORARY' }
      })
    } else {
      updatedGridSites.push(gridSite)
    }
  }
  state = topSitesReducerDataUpdated(state, updatedGridSites)
  return state
}

export function topSitesReducerAddSites (
  state: NewTab.State,
  addedSites: NewTab.Site[]
): NewTab.State {
  if (addedSites.length === 0) {
    return state
  }
  const currentGridSitesWithNewItems: NewTab.Site[] = [
    ...addedSites,
    ...state.gridSites
  ]
  state = topSitesReducerDataUpdated(state, currentGridSitesWithNewItems)
  return state
}

export function topSitesReducerShowSiteRemovalNotification (
  state: NewTab.State,
  shouldShow: boolean
): NewTab.State {
  state = { ...state, shouldShowSiteRemovalNotification: shouldShow }
  return state
}
