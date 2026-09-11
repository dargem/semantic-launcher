import QtQuick
import QtQuick.Controls
import QtQuick.Window

Window {
    id: root
    width: 700
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
                spacing: 8

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 64
                    radius: 14
                    border.width: 1
                    border.color: ListView.isCurrentItem ? "#5e81ac" : (hoverArea.containsMouse ? "#4c566a" : "#2e3440")
                    color: ListView.isCurrentItem ? "#263040" : (hoverArea.containsMouse ? "#212733" : "transparent")

                    MouseArea {
                        id: hoverArea
                        anchors.fill: parent
                        hoverEnabled: true
                    }

                    Column {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: badgeColumn.left
                        anchors.rightMargin: 12
                        spacing: 8

                        Text {
                            id: nameText
                            text: Name
                            color: "#f4f4f5"
                            font.pixelSize: 18
                            font.weight: Font.Medium
                        }

                        Text {
                            width: parent.width
                            text: Description.length > 0 ? Description : "No description available"
                            color: "#9ca3af"
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }

                    Column {
                        id: badgeColumn
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        Rectangle {
                            width: 58
                            height: 26
                            radius: 13
                            color: Number(Score) >= 0.75 ? "#2f855a" : (Number(Score) >= 0.5 ? "#805ad5" : "#4c566a")
                            border.color: Qt.lighter(color, 1.2)

                            Text {
                                anchors.centerIn: parent
                                text: Math.round(Number(Score) * 100) + "%"
                                color: "#f8fafc"
                                font.pixelSize: 12
                                font.weight: Font.Bold
                            }
                        }
                    }
                }
            }
        }
    }
}