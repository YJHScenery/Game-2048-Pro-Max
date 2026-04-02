import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material as ControlsMaterial
import QtQuick.Layouts

Item {
    id: root
    focus: true

    Connections {
        target: game2048 // C++ 端游戏对象

        // C++ 端信号：void sendGameData(const QString &gameMode, const QVariantList &sizeInfo, const QVariantList &flatData);
        function onSendGameData(gameMode, sizeInfo, flatData) {
            if (flatData === undefined || flatData === null)
                return;
            // reset / non-animated update
            root.isAnimating = false;
            root.activeMoves = [];
            root.activeMerges = [];
            root.pendingValues = null;
            root.spawnInfo = null;
            root._animPending = 0;
            root.values = flatData;
        }

        function onSendMoveTrace2D(gameMode, sizeInfo, flatData, moves, merges, spawn) {
            if (flatData === undefined || flatData === null)
                return;
            if (flatData.length !== root.cellCount) {
                root.values = flatData;
                return;
            }

            // Avoid overlapping animations
            if (root.isAnimating)
                return;

            // Fast no-op detection
            var same = true;
            if (root.values && root.values.length === flatData.length) {
                for (var i = 0; i < flatData.length; i++) {
                    if (Number(root.values[i]) !== Number(flatData[i])) {
                        same = false;
                        break;
                    }
                }
            } else {
                same = false;
            }
            if (same)
                return;
            var mvs = moves || [];
            var filtered = [];
            for (var j = 0; j < mvs.length; j++) {
                var m = mvs[j];
                var from = Number(m.from);
                var to = Number(m.to);
                var merged = !!m.merged;
                if (merged || from !== to)
                    filtered.push(m);
            }

            if (filtered.length === 0) {
                root.values = flatData;
                return;
            }

            root.activeMerges = merges || [];
            root.pendingValues = flatData;
            root.spawnInfo = (spawn && spawn.index !== undefined) ? spawn : null;
            root.activeMoves = filtered;
            root._animPending = filtered.length;
            root.isAnimating = true;
        }
    }

    property int rows: 4
    property int columns: 4
    property var values: [] // length = rows*columns; 0 means empty
    property string gameMode: "Static"

    property bool isAnimating: false
    property var pendingValues: null
    property var activeMoves: []
    property var activeMerges: []
    property var spawnInfo: null
    property int animDurationMs: 120
    property int _animPending: 0

    signal moveRequested(string direction) // up/down/left/right

    readonly property int cellCount: Math.max(0, rows) * Math.max(0, columns)
    readonly property int safeRows: Math.max(1, rows)
    readonly property int safeColumns: Math.max(1, columns)

    function valueAt(i) {
        if (!values || i < 0 || i >= values.length)
            return 0;
        var v = values[i];
        return (v === undefined || v === null) ? 0 : v;
    }

    function posForIndex(i) {
        var idx = Math.max(0, Math.floor(i));
        var c = idx % safeColumns;
        var r = Math.floor(idx / safeColumns);
        return {
            x: c * (center.cellSize + center.spacing),
            y: r * (center.cellSize + center.spacing)
        };
    }

    function mergeNewValueAt(toIndex) {
        var arr = activeMerges || [];
        var t = Number(toIndex);
        for (var k = 0; k < arr.length; k++) {
            var me = arr[k];
            if (Number(me.to) === t)
                return Number(me.newValue);
        }
        return 0;
    }

    function _onOneTileDone() {
        root._animPending = Math.max(0, root._animPending - 1);
        if (root._animPending > 0)
            return;
        if (root.pendingValues !== null)
            root.values = root.pendingValues;
        root.pendingValues = null;
        root.activeMoves = [];
        root.activeMerges = [];
        root.isAnimating = false;

        if (root.spawnInfo && root.spawnInfo.index !== undefined) {
            spawnPop.index = Number(root.spawnInfo.index);
            spawnPop.value = Number(root.spawnInfo.value);
            spawnPop.visible = true;
            spawnAnim.stop();
            spawnAnim.start();
        }
        root.spawnInfo = null;
    }

    function seedValues() {
        if (game2048 && game2048.resetGame_emitted) {
            game2048.resetGame_emitted(gameMode, [rows, columns]);
            return;
        }
        // fallback: keep a deterministic local seed if C++ object isn't available
        var n = cellCount;
        var arr = new Array(n);
        for (var i = 0; i < n; i++)
            arr[i] = 0;
        arr[0] = 2;
        if (n > 5)
            arr[5] = 4;
        values = arr;
    }

    Component.onCompleted: {
        seedValues();
        forceActiveFocus();
    }

    onRowsChanged: seedValues()
    onColumnsChanged: seedValues()

    Keys.onPressed: function (event) {
        if (root.isAnimating) {
            event.accepted = true;
            return;
        }
        switch (event.key) {
        case Qt.Key_Up:
        case Qt.Key_W:
            moveRequested("up");
            if (game2048 && game2048.up_operated)
                game2048.up_operated(gameMode, [rows, columns]);
            event.accepted = true;
            break;
        case Qt.Key_Down:
        case Qt.Key_S:
            moveRequested("down");
            if (game2048 && game2048.down_operated)
                game2048.down_operated(gameMode, [rows, columns]);
            event.accepted = true;
            break;
        case Qt.Key_Left:
        case Qt.Key_A:
            moveRequested("left");
            if (game2048 && game2048.left_operated)
                game2048.left_operated(gameMode, [rows, columns]);
            event.accepted = true;
            break;
        case Qt.Key_Right:
        case Qt.Key_D:
            moveRequested("right");
            if (game2048 && game2048.right_operated)
                game2048.right_operated(gameMode, [rows, columns]);
            event.accepted = true;
            break;
        default:
            break;
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: 18
        color: "#0b1220"
        border.width: 1
        border.color: "#22304a"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                radius: 12
                color: "#0f172a"
                border.color: "#2a3446"
                border.width: 1
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10
                    Text {
                        text: "2D 棋盘"
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        color: "#e5e7eb"
                    }
                    Text {
                        Layout.fillWidth: true
                        text: rows + "×" + columns
                        font.pixelSize: 12
                        color: "#9ca3af"
                        elide: Text.ElideRight
                    }
                    Rectangle {
                        Layout.preferredHeight: 22
                        radius: 11
                        color: "#111827"
                        border.color: "#2a3446"
                        border.width: 1
                        Layout.preferredWidth: hint.implicitWidth + 16
                        Text {
                            id: hint
                            anchors.centerIn: parent
                            text: "WASD / ↑↓←→"
                            font.pixelSize: 11
                            color: "#cbd5e1"
                        }
                    }
                }
            }
        }

        Item {
            id: center
            Layout.fillWidth: true
            Layout.fillHeight: true

            readonly property real maxGridWidth: width
            readonly property real maxGridHeight: height - footer.implicitHeight - 10

            readonly property real spacing: Math.max(6, Math.floor(Math.min(maxGridWidth, maxGridHeight) / 64))
            readonly property real cellSize: {
                var w = (maxGridWidth - spacing * (safeColumns - 1)) / safeColumns;
                var h = (maxGridHeight - spacing * (safeRows - 1)) / safeRows;
                return Math.max(18, Math.floor(Math.min(w, h)));
            }

            Item {
                id: gridWrap
                width: safeColumns * center.cellSize + center.spacing * (safeColumns - 1)
                height: safeRows * center.cellSize + center.spacing * (safeRows - 1)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top

                Grid {
                    id: grid
                    anchors.fill: parent
                    columns: safeColumns
                    rows: safeRows
                    spacing: center.spacing

                    Repeater {
                        model: root.cellCount
                        delegate: Rectangle {
                            width: center.cellSize
                            height: center.cellSize
                            radius: Math.max(8, width * 0.18)
                            color: "#111827"
                            border.width: 1
                            border.color: "#2a3446"

                            Rectangle {
                                anchors.fill: parent
                                radius: parent.radius
                                gradient: Gradient {
                                    GradientStop {
                                        position: 0.0
                                        color: "#0f172a"
                                    }
                                    GradientStop {
                                        position: 1.0
                                        color: "#0b1220"
                                    }
                                }
                                opacity: 0.95
                            }

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: Math.max(6, Math.floor(width * 0.14))
                                radius: Math.max(6, parent.radius - 4)
                                color: "#0b1220"
                                border.width: 1
                                border.color: "#22304a"
                                opacity: 0.65
                            }

                            Text {
                                anchors.centerIn: parent
                                text: {
                                    var v = root.valueAt(index);
                                    return v > 0 ? v : "";
                                }
                                font.pixelSize: Math.max(12, Math.floor(width * 0.34))
                                font.weight: Font.DemiBold
                                color: "#e5e7eb"
                                opacity: root.isAnimating ? 0 : 1
                            }
                        }
                    }
                }

                Item {
                    id: overlay
                    anchors.fill: parent
                    z: 10

                    Repeater {
                        model: root.activeMoves
                        delegate: Rectangle {
                            id: tile
                            width: center.cellSize
                            height: center.cellSize
                            radius: Math.max(8, width * 0.18)
                            color: "#111827"
                            border.width: 1
                            border.color: "#2a3446"
                            antialiasing: true

                            property int fromIndex: Number(modelData.from)
                            property int toIndex: Number(modelData.to)
                            property real fromX: root.posForIndex(fromIndex).x
                            property real fromY: root.posForIndex(fromIndex).y
                            property real toX: root.posForIndex(toIndex).x
                            property real toY: root.posForIndex(toIndex).y
                            property bool merged: !!modelData.merged
                            property bool primary: (modelData.primary === undefined) ? true : !!modelData.primary
                            property int startValue: Number(modelData.value)
                            property int mergeValue: root.mergeNewValueAt(toIndex)

                            x: fromX
                            y: fromY
                            scale: 1.0
                            opacity: 1.0

                            Rectangle {
                                anchors.fill: parent
                                radius: parent.radius
                                gradient: Gradient {
                                    GradientStop {
                                        position: 0.0
                                        color: "#0f172a"
                                    }
                                    GradientStop {
                                        position: 1.0
                                        color: "#0b1220"
                                    }
                                }
                                opacity: 0.95
                            }

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: Math.max(6, Math.floor(width * 0.14))
                                radius: Math.max(6, parent.radius - 4)
                                color: "#0b1220"
                                border.width: 1
                                border.color: "#22304a"
                                opacity: 0.65
                            }

                            Text {
                                anchors.centerIn: parent
                                text: (merged && primary && mergeValue > 0) ? mergeValue : startValue
                                font.pixelSize: Math.max(12, Math.floor(width * 0.34))
                                font.weight: Font.DemiBold
                                color: "#e5e7eb"
                            }

                            ParallelAnimation {
                                id: moveAnim
                                running: false
                                NumberAnimation {
                                    target: tile
                                    property: "x"
                                    to: tile.toX
                                    duration: root.animDurationMs
                                    easing.type: Easing.InOutQuad
                                }
                                NumberAnimation {
                                    target: tile
                                    property: "y"
                                    to: tile.toY
                                    duration: root.animDurationMs
                                    easing.type: Easing.InOutQuad
                                }
                                onStopped: {
                                    if (tile.merged && tile.primary) {
                                        mergeBounce.start();
                                    } else if (tile.merged && !tile.primary) {
                                        fadeOut.start();
                                    } else {
                                        root._onOneTileDone();
                                    }
                                }
                            }

                            SequentialAnimation {
                                id: mergeBounce
                                running: false
                                NumberAnimation {
                                    target: tile
                                    property: "scale"
                                    to: 1.12
                                    duration: 70
                                    easing.type: Easing.OutQuad
                                }
                                NumberAnimation {
                                    target: tile
                                    property: "scale"
                                    to: 1.0
                                    duration: 80
                                    easing.type: Easing.InOutQuad
                                }
                                onStopped: root._onOneTileDone()
                            }

                            NumberAnimation {
                                id: fadeOut
                                target: tile
                                property: "opacity"
                                to: 0.0
                                duration: 60
                                easing.type: Easing.OutQuad
                                onStopped: root._onOneTileDone()
                            }

                            Component.onCompleted: moveAnim.start()
                        }
                    }

                    Rectangle {
                        id: spawnPop
                        visible: false
                        width: center.cellSize
                        height: center.cellSize
                        radius: Math.max(8, width * 0.18)
                        color: "#111827"
                        border.width: 1
                        border.color: "#2a3446"
                        property int index: -1
                        property int value: 0
                        x: (index >= 0) ? root.posForIndex(index).x : 0
                        y: (index >= 0) ? root.posForIndex(index).y : 0
                        scale: 0.6
                        opacity: 0.0

                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            gradient: Gradient {
                                GradientStop {
                                    position: 0.0
                                    color: "#0f172a"
                                }
                                GradientStop {
                                    position: 1.0
                                    color: "#0b1220"
                                }
                            }
                            opacity: 0.95
                        }

                        Text {
                            anchors.centerIn: parent
                            text: spawnPop.value > 0 ? spawnPop.value : ""
                            font.pixelSize: Math.max(12, Math.floor(width * 0.34))
                            font.weight: Font.DemiBold
                            color: "#e5e7eb"
                        }
                    }

                    SequentialAnimation {
                        id: spawnAnim
                        running: false
                        ParallelAnimation {
                            NumberAnimation {
                                target: spawnPop
                                property: "opacity"
                                to: 1.0
                                duration: 90
                                easing.type: Easing.OutQuad
                            }
                            NumberAnimation {
                                target: spawnPop
                                property: "scale"
                                to: 1.0
                                duration: 120
                                easing.type: Easing.OutBack
                            }
                        }
                        PauseAnimation {
                            duration: 120
                        }
                        NumberAnimation {
                            target: spawnPop
                            property: "opacity"
                            to: 0.0
                            duration: 80
                            easing.type: Easing.InQuad
                        }
                        onStopped: {
                            spawnPop.visible = false;
                            spawnPop.index = -1;
                            spawnPop.value = 0;
                            spawnPop.scale = 0.6;
                        }
                    }
                }
            }

            Item {
                id: footer
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: implicitHeight

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.min(parent.width, 260)
                    height: 48
                    radius: 14
                    color: "#0f172a"
                    border.color: "#2a3446"
                    border.width: 1

                    Text {
                        id: lastMove
                        anchors.centerIn: parent
                        text: "Last move: —"
                        font.pixelSize: 12
                        color: "#9ca3af"
                    }
                }
            }
        }
    }

    onMoveRequested: function (dir) {
        lastMove.text = "Last move: " + dir;
    }
}
