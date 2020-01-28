/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'
import { bindActionCreators, Dispatch } from 'redux'
import { connect } from 'react-redux'

// Components
import { AdBlockItemList } from './adBlockItemList'
import { CustomFilters } from './customFilters'
import { NumBlockedStat } from './numBlockedStat'

// Components for playlist
import { CloseCircleOIcon } from 'brave-ui/components/icons'
import Table, { Cell, Row } from 'brave-ui/components/dataTables/table'

// Utils
import * as adblockActions from '../actions/adblock_actions'

interface Props {
  actions: any
  adblockData: AdBlock.State
}

interface State {
  playlists: any
}

export class AdblockPage extends React.Component<Props, State> {
  constructor (props: Props) {
    super(props)
    this.state = { playlists: [] }
    this.getPlaylist()
  }

  get actions () {
    return this.props.actions
  }

  getPlaylist = () => {
    chrome.bravePlaylists.getAllPlaylists(playlists => {
      this.setState({ playlists })
    })
  }

  componentDidUpdate(prevProps: any, prevState: any) {
    chrome.bravePlaylists.onPlaylistsChanged.addListener((changeType, id) => {
      if (
        JSON.stringify(prevState.playlists) !==
        JSON.stringify(this.state.playlists)
      ) {
        this.getPlaylist()
      }
    })
  }

  getPlaylistHeader = (): Cell[] => {
    return [
      { content: 'INDEX' },
      { content: 'NAME' },
      { content: 'STATUS' },
      { content: 'REMOVE' }
    ]
  }

  getPlaylistRows = (playlist?: any): Row[] | undefined => {
    if (playlist == null) {
      return
    }

    return playlist.map((video: any, index: any): any => {
      console.log('some cool video data', video)
      const lazyButtonStyle: any = {
        alignItems: 'center',
        WebkitAppearance: 'none',
        width: '50px',
        height: '50px',
        display: 'flex'
      }

      const cell: Row = {
        content: [
          { content: (<span>{index}</span>) },
          { content: (
            <div>
              <video
                src={video.videoMediaFilePath}
                controls={true}
                muted={true}
                width={400}
                height={400}
              />
              <audio
                controls={true}
                src={video.audioMediaFilePath}
              />
              <h3>{video.titles[0]}</h3>
            </div>
          ) },
          { content: (<span>{video.partialReady ? 'Completed' : 'In progress'}</span>) },
          { content: (<button style={lazyButtonStyle} onClick={this.onClickRemoveVideo}><CloseCircleOIcon /></button>) }
        ]
      }
      return cell
    })
  }

  onClickRemoveALlVideos = () => {
    console.log('nothing')
  }

  onClickRemoveVideo = () => {
    console.log('nothing')
  }

  onClickPlayVideo = () => {
    console.log('nothing')
  }

  render () {
    const { actions, adblockData } = this.props
    const { playlists } = this.state

    return (
      <div id='adblockPage'>
        <div style={{ minHeight: '600px' }}>
          <button>Delete all playlists</button>
          <Table header={this.getPlaylistHeader()} rows={this.getPlaylistRows(playlists)}>
            YOUR PLAYLIST IS NOW EMPTY
          </Table>
        </div>
        <hr />
        <NumBlockedStat adsBlockedStat={adblockData.stats.adsBlockedStat || 0} />
        <AdBlockItemList
          actions={actions}
          resources={adblockData.settings.regionalLists}
        />
        <CustomFilters
          actions={actions}
          rules={adblockData.settings.customFilters || ''}
        />
      </div>
    )
  }
}

export const mapStateToProps = (state: AdBlock.ApplicationState) => ({
  adblockData: state.adblockData
})

export const mapDispatchToProps = (dispatch: Dispatch) => ({
  actions: bindActionCreators(adblockActions, dispatch)
})

export default connect(
  mapStateToProps,
  mapDispatchToProps
)(AdblockPage)
