import QtQuick
import QtQuick.Controls
import QtQuick.Window

Window {
    id: root
    width: 960
    height: 72
    visible: false
    color: "transparent"
    title: "semantic-launcher"
    flags: Qt.FramelessWindowHint

    x: Math.round((Screen.width - width) / 2)
    y: 24

    Shortcut {
        sequence: "Escape"
        onActivated: root.close()
    }

    Rectangle {
        anchors.fill: parent
        radius: 20
        color: "#1a1d24"
        border.color: "#2e3440"
        border.width: 1

        Column {
            anchors.fill: parent
            anchors.margins: 14

            TextField {
                id: searchField
                width: parent.width
                placeholderText: "Search apps..."
                font.pixelSize: 20
                color: "#f4f4f5"
                background: null
                focus: true

                onTextChanged: searchEngine.search(text)

                Keys.onReturnPressed: searchEngine.launch(resultsList.currentIndex)
                Keys.onDownPressed: resultsList.incrementCurrentIndex()
                Keys.onUpPressed: resultsList.decrementCurrentIndex()
            }

            ListView {
                id: resultsList
                width: parent.width
                height: 300
                model: searchEngine.results
                delegate: Text { text: model.NameRole; color: "#f4f4f5" }
            }
        }
    }
}