import QtQuick 6.9
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window

Popup {
    id: root

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    width: 420
    padding: 0
    anchors.centerIn: Overlay.overlay

    // Popup opens on Overlay; parent may change, so use attached ApplicationWindow reference.
    readonly property var hostWindow: ApplicationWindow.window
    readonly property int currentScore: (hostWindow && hostWindow.currentScore !== undefined) ? hostWindow.currentScore : 0
    readonly property int maxScore: (hostWindow && hostWindow.maxScore !== undefined) ? hostWindow.maxScore : 0

    // Match the in-game dark palette for visual consistency.
    readonly property color panelColor: "#111827"
    readonly property color panelBorder: "#2a3446"
    readonly property color titleColor: "#e5e7eb"
    readonly property color subColor: "#9ca3af"
    readonly property color accentColor: "#a78bfa"

    background: Rectangle {
        radius: 16
        color: root.panelColor
        border.color: root.panelBorder
        border.width: 1
    }

    contentItem: ColumnLayout {
        spacing: 14

        Item {
            Layout.preferredHeight: 6
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: "游戏结束"
            color: root.titleColor
            font.pixelSize: 28
            font.bold: true
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: "别灰心，再试一次就能破纪录。"
            color: root.subColor
            font.pixelSize: 14
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.leftMargin: 18
            Layout.rightMargin: 18
            Layout.topMargin: 4
            height: 88
            radius: 12
            color: "#0f172a"
            border.color: root.panelBorder
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 6

                Label {
                    Layout.fillWidth: true
                    text: "本局分数: " + root.currentScore
                    color: root.titleColor
                    font.pixelSize: 18
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    Layout.fillWidth: true
                    text: "历史最高: " + root.maxScore
                    color: root.accentColor
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        Item {
            Layout.preferredHeight: 2
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 18
            Layout.rightMargin: 18
            Layout.topMargin: 10
            Layout.bottomMargin: 14

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: "我知道了"
                highlighted: true
                Material.accent: Material.DeepPurple
                Accessible.name: "关闭游戏结束对话框"
                onClicked: root.close()
            }
        }
    }
}
