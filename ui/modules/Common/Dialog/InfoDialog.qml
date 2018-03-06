import Common 1.0
import Common.Styles 1.0

// =============================================================================
// A dialog with OK/Cancel buttons.
// =============================================================================

DialogPlus {
  buttons: [

    TextButtonB {
      text: qsTr('OK')

      onClicked: exit(1)
    }
  ]

  centeredButtons: true

  height: DialogStyle.confirmDialog.height
  width: DialogStyle.confirmDialog.width
}