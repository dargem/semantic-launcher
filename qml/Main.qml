import QtQuick
import QtQuick.Controls
import QtQuick.Window

Window {
    id: root
    width: 960
    visible: false
    color: "transparent"
    title: "semantic-launcher"
    flags: Qt.FramelessWindowHint

    height: contentColumn.implicitHeight + 28

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
            id: contentColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 14
            spacing: 12

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
                model: searchEngine.results
                clip: true
                currentIndex: 0
                implicitHeight: contentHeight
                height: contentHeight

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 44
                    radius: 10
                    color: ListView.isCurrentItem ? "#2e3440" : "transparent"

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        width: parent.width - 24
                        text: Name
                        color: "#f4f4f5"
                        font.pixelSize: 18
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}