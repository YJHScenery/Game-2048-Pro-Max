import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    id: root
    width: 1280
    height: 720
    visible: true
    title: "2048 Quick"

    Material.theme: Material.Dark
    Material.primary: Material.BlueGrey
    Material.accent: Material.DeepPurple

    property int selectedIndex: 0
    readonly property bool isCustomMode: selectedIndex < 0
    readonly property var selectedMode: (!isCustomMode && selectedIndex >= 0 && selectedIndex < modeModel.count) ? modeModel.get(selectedIndex) : null

    ListModel {
        id: modeModel
        ListElement {
            key: "4x4"
            title: "4×4"
            subtitle: "经典模式 / 2D"
            dims: 2
            size: 4
            depth: 1
        }
        ListElement {
            key: "6x6"
            title: "6×6"
            subtitle: "进阶模式 / 2D"
            dims: 2
            size: 6
            depth: 1
        }
        ListElement {
            key: "8x8"
            title: "8×8"
            subtitle: "硬核模式 / 2D"
            dims: 2
            size: 8
            depth: 1
        }
        ListElement {
            key: "4x4x4"
            title: "4×4×4"
            subtitle: "立体模式 / 3D"
            dims: 3
            size: 4
            depth: 4
        }
        ListElement {
            key: "6x6x6"
            title: "6×6×6"
            subtitle: "立体模式 / 3D"
            dims: 3
            size: 6
            depth: 6
        }
        ListElement {
            key: "8x8x8"
            title: "8×8×8"
            subtitle: "立体模式 / 3D"
            dims: 3
            size: 8
            depth: 8
        }
    }

    function chipText(dims, size, depth) {
        if (!dims || !size)
            return "—";
        return dims === 2 ? (size + "×" + size + " 2D") : (size + "×" + size + "×" + depth + " 3D");
    }

    function modeForIndex(idx) {
        if (idx < 0 || idx >= modeModel.count)
            return null;
        return modeModel.get(idx);
    }

    function syncBoardFromMode() {
        if (root.isCustomMode)
            return;
        var mode = root.modeForIndex(root.selectedIndex);
        if (!mode)
            return;
        if (!boardLoader || !boardLoader.item)
            return;
        if (mode.dims === 2) {
            boardLoader.item.rows = mode.size;
            boardLoader.item.columns = mode.size;
        } else {
            boardLoader.item.boardSize = mode.size;
            boardLoader.item.boardDepth = mode.depth;
        }
        if (boardLoader.item.seedValues)
            boardLoader.item.seedValues();
        if (boardLoader.item.forceActiveFocus)
            boardLoader.item.forceActiveFocus();
    }

    onSelectedIndexChanged: Qt.callLater(root.syncBoardFromMode)

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "#0b0f19"
            }
            GradientStop {
                position: 0.55
                color: "#0a1020"
            }
            GradientStop {
                position: 1.0
                color: "#060912"
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.12
        layer.enabled: true
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 22
        spacing: 18

        Rectangle {
            id: leftPanel
            Layout.preferredWidth: 420
            Layout.fillHeight: true
            radius: 18
            color: "#111827"
            border.color: "#2a3446"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 14

                Item {
                    Layout.fillWidth: true
                    height: 62
                    Text {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        text: "2048 Quick"
                        font.pixelSize: 28
                        font.weight: Font.DemiBold
                        color: "#e5e7eb"
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.bottom: parent.bottom
                        text: "选择模式"
                        font.pixelSize: 13
                        color: "#9ca3af"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#1f2937"
                }

                Text {
                    Layout.fillWidth: true
                    text: "模式"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    color: "#cbd5e1"
                }

                ListView {
                    id: modeList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 10
                    model: modeModel
                    currentIndex: root.selectedIndex
                    interactive: true
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollIndicator.vertical: ScrollIndicator {}

                    delegate: Rectangle {
                        id: tile
                        width: modeList.width
                        height: 86
                        radius: 14
                        color: (ListView.isCurrentItem ? "#1f2a44" : "#0f172a")
                        border.width: 1
                        border.color: (ListView.isCurrentItem ? "#6d28d9" : "#23304a")

                        property bool hovered: false
                        HoverHandler {
                            id: hover
                            onHoveredChanged: tile.hovered = hovered
                        }
                        TapHandler {
                            onTapped: root.selectedIndex = index
                        }

                        Behavior on color {
                            ColorAnimation {
                                duration: 140
                            }
                        }
                        Behavior on border.color {
                            ColorAnimation {
                                duration: 140
                            }
                        }
                        Behavior on y {
                            NumberAnimation {
                                duration: 140
                                easing.type: Easing.OutCubic
                            }
                        }

                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            gradient: Gradient {
                                GradientStop {
                                    position: 0.0
                                    color: tile.hovered ? "#131d34" : "#0f172a"
                                }
                                GradientStop {
                                    position: 1.0
                                    color: tile.hovered ? "#0b1223" : "#0a1020"
                                }
                            }
                            opacity: 0.95
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 12

                            Rectangle {
                                Layout.preferredWidth: 46
                                Layout.preferredHeight: 46
                                radius: 12
                                color: ListView.isCurrentItem ? "#6d28d9" : "#111827"
                                border.width: 1
                                border.color: ListView.isCurrentItem ? "#a78bfa" : "#2a3446"
                                Text {
                                    anchors.centerIn: parent
                                    text: (dims === 2) ? "2D" : "3D"
                                    font.pixelSize: 14
                                    font.weight: Font.Bold
                                    color: "#e5e7eb"
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4
                                Text {
                                    Layout.fillWidth: true
                                    text: title
                                    font.pixelSize: 18
                                    font.weight: Font.DemiBold
                                    color: "#e5e7eb"
                                    elide: Text.ElideRight
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8
                                    Text {
                                        Layout.fillWidth: true
                                        text: subtitle
                                        font.pixelSize: 13
                                        color: "#9ca3af"
                                        elide: Text.ElideRight
                                    }
                                    Rectangle {
                                        Layout.preferredHeight: 22
                                        radius: 11
                                        color: ListView.isCurrentItem ? "#2e1065" : "#111827"
                                        border.width: 1
                                        border.color: ListView.isCurrentItem ? "#a78bfa" : "#2a3446"
                                        width: chip.implicitWidth + 16
                                        Text {
                                            id: chip
                                            anchors.centerIn: parent
                                            text: root.chipText(dims, size, depth)
                                            font.pixelSize: 11
                                            color: "#cbd5e1"
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Button {
                    id: customBtn
                    Layout.fillWidth: true
                    text: "自定义尺寸"
                    onClicked: root.selectedIndex = -1
                    Material.background: "#0f172a"
                    Material.foreground: "#e5e7eb"
                }
            }
        }

        Rectangle {
            id: rightPanel
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 18
            color: "#0b1220"
            border.color: "#273244"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 14

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Text {
                            Layout.fillWidth: true
                            text: root.isCustomMode ? "自定义模式" : (root.selectedMode ? ("模式预览 · " + root.selectedMode.title) : "模式预览")
                            font.pixelSize: 20
                            font.weight: Font.DemiBold
                            color: "#e5e7eb"
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            text: root.isCustomMode ? "这里可以放置自定义尺寸输入UI" : (root.selectedMode ? root.selectedMode.subtitle : "—")
                            font.pixelSize: 13
                            color: "#9ca3af"
                            elide: Text.ElideRight
                        }
                    }

                    Rectangle {
                        Layout.preferredHeight: 34
                        radius: 17
                        color: "#0f172a"
                        border.color: "#2a3446"
                        border.width: 1
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 8
                            Rectangle {
                                Layout.preferredWidth: 8
                                Layout.preferredHeight: 8
                                radius: 4
                                color: root.isCustomMode ? "#f59e0b" : "#22c55e"
                            }
                            Text {
                                text: root.isCustomMode ? "待配置" : "就绪"
                                font.pixelSize: 12
                                color: "#cbd5e1"
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#1f2937"
                }

                Rectangle {
                    id: previewCard
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 18
                    color: "#0f172a"
                    border.color: "#273244"
                    border.width: 1

                    Item {
                        anchors.fill: parent
                        anchors.margins: 18

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 14

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 84
                                    radius: 16
                                    color: "#0b1220"
                                    border.color: "#22304a"
                                    border.width: 1
                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 14
                                        spacing: 14

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 4
                                            Text {
                                                text: "展示信息"
                                                font.pixelSize: 12
                                                color: "#9ca3af"
                                            }
                                            Text {
                                                text: root.isCustomMode ? "自定义尺寸" : (root.selectedMode ? root.chipText(root.selectedMode.dims, root.selectedMode.size, root.selectedMode.depth) : "—")
                                                font.pixelSize: 18
                                                font.weight: Font.DemiBold
                                                color: "#e5e7eb"
                                            }
                                        }

                                        ColumnLayout {
                                            spacing: 4
                                            Layout.preferredWidth: 220
                                            Text {
                                                text: "Score / Best"
                                                font.pixelSize: 12
                                                color: "#9ca3af"
                                            }
                                            RowLayout {
                                                spacing: 10
                                                Rectangle {
                                                    radius: 12
                                                    color: "#111827"
                                                    border.color: "#2a3446"
                                                    border.width: 1
                                                    Layout.preferredHeight: 34
                                                    Layout.preferredWidth: 100
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: "—"
                                                        color: "#e5e7eb"
                                                        font.pixelSize: 14
                                                    }
                                                }
                                                Rectangle {
                                                    radius: 12
                                                    color: "#111827"
                                                    border.color: "#2a3446"
                                                    border.width: 1
                                                    Layout.preferredHeight: 34
                                                    Layout.preferredWidth: 100
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: "—"
                                                        color: "#e5e7eb"
                                                        font.pixelSize: 14
                                                    }
                                                }
                                            }
                                        }

                                        KeyHintWidget {
                                            Layout.preferredWidth: 240
                                            Layout.alignment: Qt.AlignVCenter
                                            visible: !root.isCustomMode && root.selectedMode
                                            dims: root.selectedMode ? root.selectedMode.dims : 2
                                            sourceCamera: (root.selectedMode && root.selectedMode.dims === 3 && boardLoader.item && boardLoader.item.viewCamera) ? boardLoader.item.viewCamera : null
                                        }
                                    }
                                }
                            }

                            Item {
                                id: boardArea
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                Loader {
                                    id: boardLoader
                                    anchors.fill: parent
                                    active: true
                                    sourceComponent: root.isCustomMode ? customPlaceholder : null
                                    source: {
                                        if (root.isCustomMode)
                                            return "";
                                        var mode = root.modeForIndex(root.selectedIndex);
                                        if (!mode)
                                            return "";
                                        return mode.dims === 2 ? "qrc:/qml/Board2D.qml" : "qrc:/qml/Board3D.qml";
                                    }
                                    onLoaded: Qt.callLater(root.syncBoardFromMode)
                                    onStatusChanged: {
                                        if (status === Loader.Ready)
                                            Qt.callLater(root.syncBoardFromMode);
                                    }
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    visible: boardLoader.status === Loader.Error
                                    radius: 18
                                    color: "#0b1220"
                                    border.color: "#7f1d1d"
                                    border.width: 1
                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 18
                                        spacing: 10
                                        Text {
                                            text: "棋盘加载失败"
                                            font.pixelSize: 18
                                            font.weight: Font.DemiBold
                                            color: "#fecaca"
                                        }
                                        Text {
                                            Layout.fillWidth: true
                                            text: (boardLoader.errorString !== undefined && boardLoader.errorString !== null) ? String(boardLoader.errorString) : ""
                                            wrapMode: Text.WordWrap
                                            font.pixelSize: 12
                                            color: "#fca5a5"
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Component {
                    id: customPlaceholder
                    Rectangle {
                        width: 560
                        height: 360
                        radius: 18
                        color: "#0b1220"
                        border.width: 1
                        border.color: "#22304a"
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 20
                            spacing: 10
                            Text {
                                text: "自定义尺寸"
                                font.pixelSize: 20
                                font.weight: Font.DemiBold
                                color: "#e5e7eb"
                            }
                            Text {
                                text: "本次按你的要求只搭界面：这里预留自定义尺寸的输入面板位置（例如 行/列/深度）。"
                                wrapMode: Text.WordWrap
                                font.pixelSize: 13
                                color: "#9ca3af"
                            }
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 14
                                color: "#0f172a"
                                border.color: "#2a3446"
                                border.width: 1
                                Text {
                                    anchors.centerIn: parent
                                    text: "[ 这里放输入控件 / 校验 / 保存按钮 —— 当前未实现 ]"
                                    color: "#94a3b8"
                                    font.pixelSize: 13
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
