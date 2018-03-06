pragma Singleton
import QtQml 2.2

// =============================================================================

QtObject {
  property QtObject buttons: QtObject {
    property int spacing: 200

    property QtObject button: QtObject {
      property int height: 40
      property int width: 100
    }
  }
}
