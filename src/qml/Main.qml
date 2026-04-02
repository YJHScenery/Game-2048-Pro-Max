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

    // Model 为 MVC 设计模式中的“M”
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

    // 根据棋盘的维度显示不同的提示（2D or 3D）
    function chipText(dims, size, depth) {
        if (!dims || !size)
            return "—";
        return dims === 2 ? (size + "×" + size + " 2D") : (size + "×" + size + "×" + depth + " 3D");
    }

    // 根据模式选择的 index 获取游戏模式
    function modeForIndex(idx) {
        if (idx < 0 || idx >= modeModel.count)
            return null;
        return modeModel.get(idx);
    }

    //! 初始化棋盘基本属性
    function syncBoardFromMode() {
        if (root.isCustomMode) // 后期拓展的自定义格式
            return;
        var mode = root.modeForIndex(root.selectedIndex);
        if (!mode) // 当 index < 0 或 >= modelModel.count 时触发
            return;

        //! @brief @ref boardLoader: 用于动态加载棋盘。如果其尚未初始化或初始化失败，则无返回值
        if (!boardLoader || !boardLoader.item)
            return;
        if (mode.dims === 2) {

            // ListElement 内的数据属性 .size（以及后续的 .depth）
            boardLoader.item.rows = mode.size;
            boardLoader.item.columns = mode.size;
        } else {
            boardLoader.item.boardSize = mode.size;
            boardLoader.item.boardDepth = mode.depth;
        }

        // 安全机制，防止调用 null
        if (boardLoader.item.seedValues)
            //! @note seedValues() 函数在 @file Board2D 和 @file Board3D 中被定义。
            boardLoader.item.seedValues();
        if (boardLoader.item.forceActiveFocus)
            // 自带标准方法，作用是：强制抢占键盘焦点
            boardLoader.item.forceActiveFocus();
    }

    // 此为自定义 property int selectedIndex 发生改变的信号。QML 中使用名字解析来创建信号。
    onSelectedIndexChanged: Qt.callLater(root.syncBoardFromMode)

    // 定义渐变色。
    // 此 Rectangle 使用 anchors.fill，填充整个父容器。
    // GradientStop 是渐变色锚点。通过定义渐变色锚点的颜色和位置创建良好效果的渐变色。
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

    // 半透明的黑色遮罩层，覆盖整个父容器
    Rectangle {
        anchors.fill: parent
        color: "#000000" // 纯黑色
        opacity: 0.12 // 12% 不透明度

        /*
        这行代码启用了 QML 的“离屏渲染”功能。它将这个 Rectangle 及其所有子元素渲染到一个独立的图像层上，然后再将这个图像层合成到主场景中。
        作用：启用 layer 对于 opacity 属性在某些复杂场景下（尤其是当组件内部包含其他半透明元素或动画时）能确保视觉效果的一致性和正确性。
        注意：虽然对于简单的纯色半透明矩形，不写 layer.enabled: true 通常也能看到效果，但加上它是一个更稳健和推荐的做法，可以避免一些潜在的渲染问题。
        */
        layer.enabled: true
    }

    // 水平排列的组件布局管理
    RowLayout {
        anchors.fill: parent
        anchors.margins: 22 // 22 像素的外边距
        spacing: 18 // 18 像素

        // 左侧面板，定义模式选择与滑动条
        Rectangle {
            id: leftPanel
            Layout.preferredWidth: 420 // 基准线为“最优”，可以变小直到 minimum，也可以变大直到 maximum
            Layout.fillHeight: true // 占满高度，可参考 QWidget 中的布局设置
            radius: 18 // 圆角半径
            color: "#111827"
            border.color: "#2a3446"
            border.width: 1 // 边框

            // 垂直排列的组件布局管理
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 14

                // 对应左上侧的标题栏
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

                // 用于模拟分隔线
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#1f2937"
                }

                // 一段文字栏
                Text {
                    Layout.fillWidth: true
                    text: "模式"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    color: "#cbd5e1"
                }

                // 列表视图
                ListView {
                    id: modeList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true //! @note 当列表内容超出 ListView 自身的边界时，将超出部分“裁剪”掉，确保不会显示在 ListView 区域之外。
                    spacing: 10
                    model: modeModel //! @note MVC 设计模式，model 是标准属性，用于定义数据来源。
                    currentIndex: root.selectedIndex //! @note 与 root.selectedIndex 双向同步绑定
                    interactive: true // 是否可以与 ListView 进行交互
                    boundsBehavior: Flickable.DragOverBounds // 移动应用中常见的弹性滚动效果；Flickable.StopAtBounds：自动停止。
                    ScrollIndicator.vertical: ScrollIndicator {} // 添加了一个垂直的滚动指示器

                    /*
                    delegate 主要负责两件事：
                    外观呈现 (Visuals)：决定数据如何显示。例如，是显示为一行简单的文字，还是一个包含头像、标题和摘要的复杂卡片。
                    用户交互 (Interaction)：决定用户如何操作这一项。例如，点击这一项会发生什么，或者这一项内部是否包含按钮、开关等控件。
                    */
                    delegate: Rectangle {
                        id: tile
                        width: modeList.width
                        height: 86
                        radius: 14
                        color: (ListView.isCurrentItem ? "#1f2a44" : "#0f172a") // 选中与未选中呈现不同的颜色
                        border.width: 1
                        border.color: (ListView.isCurrentItem ? "#6d28d9" : "#23304a")

                        property bool hovered: false // 用来记录鼠标是否悬停在这个卡片上。

                        // 检测鼠标是否悬停在目标区域。
                        HoverHandler {
                            id: hover
                            onHoveredChanged: tile.hovered = hovered
                        }

                        // 检测点击（触摸）事件。
                        TapHandler {
                            onTapped: root.selectedIndex = index // index: 当前卡片索引
                        }

                        //! @note QML 中强大的动画效果器
                        /*
                          对于 on color 和 on border.color，效果为在 140mm 内平滑过渡颜色
                        */
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

                        // 垂直位置变化时，平滑过渡
                        Behavior on y {
                            NumberAnimation {
                                duration: 140
                                easing.type: Easing.OutCubic //! @note 使用了三次方减速曲线，让移动效果看起来更有物理惯性（先快后慢）
                            }
                        }

                        // 依旧是渐变色填充器
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
                                spacing: 4 // 定义布局容器内各个子元素之间的间隔距离
                                Text {
                                    Layout.fillWidth: true
                                    text: title //! @ref ListModel
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

        // 右侧面板，提供游戏数据预览和棋盘加载器
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

                // 依旧模拟分割线
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

                                // 核心：棋盘加载器
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
                                        // 加载失败则触发 Error，此时下方的 Rectangle 的 visible 就会设置为 true
                                        return mode.dims === 2 ? "qrc:/qml/Board2D.qml" : "qrc:/qml/Board3D.qml";
                                    }
                                    onLoaded: Qt.callLater(root.syncBoardFromMode)
                                    onStatusChanged: {
                                        if (status === Loader.Ready)
                                            Qt.callLater(root.syncBoardFromMode);
                                    }
                                }

                                // 棋盘加载失败时触发
                                // 当 Loader 不触发 Error 时不可见（visible = false）
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

                // 选中自定义时触发的 Component
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
