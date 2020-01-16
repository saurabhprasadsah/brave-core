/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

// State helpers
import * as topSitesState from '../../../brave_new_tab_ui/state/topSitesState'

// Helpers
import { generateTopSiteProperties } from '../../../brave_new_tab_ui/helpers/newTabUtils'
import { defaultState } from '../../../brave_new_tab_ui/storage'

const newTopSite1: chrome.topSites.MostVisitedURL = {
  url: 'https://brave.com',
  title: 'brave!'
}

const newTopSite2: chrome.topSites.MostVisitedURL = {
  url: 'https://clifton.io',
  title: 'BSC]]'
}

describe('topSitesState', () => {
  describe('topSitesReducerSetFirstRenderData', () => {
    it('populates gridSites from topSites', () => {
      const assertion = topSitesState
        .topSitesReducerSetFirstRenderData(defaultState, [newTopSite1])
      expect(assertion.gridSites.length).toBe(1)
    })
    it('does not add sites which url is already in the gridSites list', () => {
      const newState = {
        ...defaultState,
        gridSites: [generateTopSiteProperties(newTopSite1, 0)]
      }
      const assertion = topSitesState
        .topSitesReducerSetFirstRenderData(newState, [
          newTopSite1,
          newTopSite1,
          newTopSite1,
          newTopSite1
        ])
      expect(assertion.gridSites.length).toBe(1)
    })
    it('can populate more than one top site if they have different urls', () => {
      const assertion = topSitesState
        .topSitesReducerSetFirstRenderData(defaultState, [
          newTopSite1,
          newTopSite2
        ])
      expect(assertion.gridSites.length).toBe(2)
    })
  })
  describe('topSitesReducerDataUpdated', () => {
    // TODO
  })
  describe('topSitesReducerToggleTopSitePinned', () => {
    // TODO
  })
  describe('topSitesReducerRemoveSite', () => {
    // TODO
  })
  describe('topSitesReducerUndoRemoveSite', () => {
    // TODO
  })
  describe('topSitesReducerUndoRemoveAllSites', () => {
    // TODO
  })
  describe('topSitesReducerToggleTopSiteBookmarked', () => {
    // TODO
  })
  describe('topSitesReducerAddSites', () => {
    // TODO
  })
  describe('topSitesReducerShowSiteRemovalNotification', () => {
    // TODO
  })
})
