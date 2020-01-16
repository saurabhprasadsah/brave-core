// Copyright (c) 2019 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

export const isHttpOrHttps = (url?: string) => {
  if (!url) {
    return false
  }
  return /^https?:/i.test(url)
}

/**
 * Obtains a letter / char that represents the current site
 * @param site - The site requested from the top site's list
 */
export const getCharForSite = (site: Partial<NewTab.Site>): string => {
  let name
  if (!site.title) {
    try {
      name = new window.URL(site.url || '').hostname
    } catch (e) {
      console.warn('getCharForSite', { url: site.url || '' })
    }
  }
  name = site.title || name || '?'
  return name.charAt(0).toUpperCase()
}

export const generateTopSiteId = (currentIndex: number): string => {
  return `topsite-${currentIndex}-${Date.now()}`
}

export const generateTopSiteFavicon = (url: string): string => {
  return `chrome://favicon/size/64@1x/${url}`
}

export const isTopSitePinned = (
  site: NewTab.Site
): boolean => {
  return site.pinnedIndex !== undefined
}

export const isTopSiteBookmarked = (
  bookmarkInfo: chrome.bookmarks.BookmarkTreeNode | undefined
): boolean => {
  return bookmarkInfo !== undefined
}

export const isExistingSite = (
  sitesData: NewTab.Site[],
  topSite: chrome.topSites.MostVisitedURL | NewTab.Site
): boolean => {
  return sitesData.some(site => site.url === topSite.url)
}

export const generateTopSiteProperties = (
  topSite: chrome.topSites.MostVisitedURL,
  index: number
): NewTab.Site => {
  return {
    ...topSite,
    id: generateTopSiteId(index),
    letter: getCharForSite(topSite),
    favicon: generateTopSiteFavicon(topSite.url),
    pinnedIndex: undefined,
    bookmarkInfo: undefined
  }
}

export const getTopSitesWhitelist = (
  topSites: chrome.topSites.MostVisitedURL[]
): chrome.topSites.MostVisitedURL[] => {
  const defaultChromeWebStoreUrl: string = 'https://chrome.google.com/webstore'
  const filteredGridSites: chrome.topSites.MostVisitedURL[] = topSites
    .filter(site => {
      // see https://github.com/brave/brave-browser/issues/5376
      return !site.url.startsWith(defaultChromeWebStoreUrl)
    })
  return filteredGridSites
}
