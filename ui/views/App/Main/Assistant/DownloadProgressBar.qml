import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Common 1.0
import Linphone 1.0
import LinphoneUtils 1.0
import Linphone.Styles 1.0
import Utils 1.0

// =============================================================================
Item {
  id: downloadProgressBar

  property var fileDownloader

  // ---------------------------------------------------------------------------
  anchors.alignWhenCentered : true
  // ---------------------------------------------------------------------------
  // Info.
  // ---------------------------------------------------------------------------

  Column {
     Layout.fillWidth: true
     Layout.fillHeight: true

     spacing: ChatStyle.entry.message.file.status.spacing

     Text {
        id: fileName

        color: ChatStyle.entry.message.outgoing.text.color
        elide: Text.ElideRight

        font {
           bold: true
           pointSize: ChatStyle.entry.message.outgoing.text.pointSize
        }

        text: qsTr('DOWNLOADING CODEC')
        width: parent.width
     }

     ProgressBar {
         id: progressBar

         height: ChatStyle.entry.message.file.status.bar.height
         width: 500

         to: fileDownloader.getTotalBytes()
         value: fileDownloader.getReadBytes() 
         indeterminate : true

         background: Rectangle {
            color: ChatStyle.entry.message.file.status.bar.background.color
            radius: ChatStyle.entry.message.file.status.bar.radius
         }

         contentItem: Item {
            Rectangle {
               color: ChatStyle.entry.message.file.status.bar.contentItem.color
               height: parent.height
               width: progressBar.visualPosition * parent.width
            }
         }
     }

     Text {
        color: fileName.color
        elide: Text.ElideRight
        font.pointSize: fileName.font.pointSize
        text: {
           var fileSize = Utils.formatSize(progressBar.to)
           return Utils.formatSize(progressBar.value) + '/' + fileSize
        }
     }
  }

  // ---------------------------------------------------------------------------
  // fileDownloader.
  // ---------------------------------------------------------------------------

  Connections {
     target: fileDownloader
     onDownloadFinished: {
     window.attachVirtualWindow(Utils.buildDialogUri('InfoDialog'), {
     descriptionText:  qsTr('Codec DownloadFinished.')
     }, function (status) {
       if (status) {
       unlockView()
       window.setView('Home')
       } 
     })
    }
    onTotalBytesChanged: {
       progressBar.to = fileDownloader.getTotalBytes()
    }
	onReadBytesChanged: {
       progressBar.value = fileDownloader.getReadBytes()
    }
  }
}
