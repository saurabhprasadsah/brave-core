// Copyright (c) 2019 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'

import { SortableElement, SortableElementProps } from 'react-sortable-hoc'

// Feature-specific components
import {
  Tile,
  TileActionsContainer,
  TileAction,
  TileFavicon
} from '../../components/default'

// Helpers
import {
  isTopSitePinned,
  isTopSiteBookmarked
} from '../../helpers/newTabUtils'

// Icons
import {
  PinIcon,
  PinOIcon,
  BookmarkIcon,
  BookmarkOIcon,
  CloseStrokeIcon
} from 'brave-ui/components/icons'

import * as newTabActions from '../../actions/new_tab_actions'
import * as topSitesActions from '../../actions/top_sites_actions'

interface Props {
  actions: typeof newTabActions & typeof topSitesActions
  siteData: NewTab.Site
}

class TopSite extends React.PureComponent<Props, {}> {
  onTogglePinnedTopSite (site: NewTab.Site) {
    this.props.actions.toggleTopSitePinned(site)
  }

  onIgnoredTopSite (site: NewTab.Site) {
    this.props.actions.removeTopSite(site)
    this.props.actions.showSiteRemovalNotification(true)
  }

  onToggleBookmark (site: NewTab.Site) {
    this.props.actions.toggleTopSiteBookmarkInfo(site)
  }

  render () {
    const { siteData } = this.props

    return (
      <Tile
        title={siteData.title}
        tabIndex={0}
        style={{
          cursor: isTopSitePinned(siteData) ? 'not-allowed' : 'grab'
        }}
      >
        <TileActionsContainer>
          <TileAction onClick={this.onTogglePinnedTopSite.bind(this, siteData)}>
            {isTopSitePinned(siteData) ? <PinIcon /> : <PinOIcon />}
          </TileAction>
          <TileAction onClick={this.onToggleBookmark.bind(this, siteData)}>
            {
              isTopSiteBookmarked(siteData.bookmarkInfo)
                ? <BookmarkIcon />
                : <BookmarkOIcon />
            }
          </TileAction>
          {
            // you can't delete a pinned item
            isTopSitePinned(siteData)
              ? (
                <TileAction
                  style={{ opacity: 0.25, cursor: 'not-allowed' }}
                >
                  <CloseStrokeIcon />
                </TileAction>
              ) : (
                <TileAction
                  onClick={this.onIgnoredTopSite.bind(this, siteData)}
                >
                  <CloseStrokeIcon />
                </TileAction>
              )
          }
        </TileActionsContainer>
        {
          isTopSitePinned(siteData)
            ? (
              <TileAction
                onClick={this.onTogglePinnedTopSite.bind(this, siteData)}
                standalone={true}
              >
                <PinIcon />
              </TileAction>
            ) : null
          }
          <a href={siteData.url}><TileFavicon src={siteData.favicon} /></a>
      </Tile>
    )
  }
}

type TopSiteSortableElementProps = SortableElementProps & Props
export default SortableElement(
  (props: TopSiteSortableElementProps) => <TopSite {...props} />
)
