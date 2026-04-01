import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material as ControlsMaterial
import QtQuick.Layouts
import QtQuick3D

Item {
    id: root
    Connections {
        target: game2048

        function onSendGameData3D(gameMode, sizeInfo, flatData) {
            // console.log("C++Server Send 3D Game Data to QML... mode=", gameMode, ", size=", sizeInfo)
            if (sizeInfo && sizeInfo.length >= 3) {
                root.boardSize = Number(sizeInfo[0]);
                root.boardDepth = Number(sizeInfo[2]);
            }
            if (flatData !== undefined && flatData !== null) {
                root.values = flatData;
            }
        }

        function onSendMoveTrace3D(gameMode, sizeInfo, flatData, moves, merges, spawn) {
            if (sizeInfo && sizeInfo.length >= 3) {
                root.boardSize = Number(sizeInfo[0]);
                root.boardDepth = Number(sizeInfo[2]);
            }

            root.pendingValues = flatData;
            root.animating = true;
            root.mergeValueByTo = ({});
            if (merges) {
                for (var i = 0; i < merges.length; i++) {
                    var me = merges[i];
                    root.mergeValueByTo[String(me.to)] = Number(me.newValue);
                }
            }

            animModel.clear();
            if (moves) {
                for (var j = 0; j < moves.length; j++) {
                    var m = moves[j];
                    animModel.append({
                        from: Number(m.from),
                        to: Number(m.to),
                        value: Number(m.value),
                        merged: Boolean(m.merged),
                        primary: Boolean(m.primary),
                        mergeNewValue: root.mergeValueByTo[String(m.to)] !== undefined ? Number(root.mergeValueByTo[String(m.to)]) : Number(m.value)
                    });
                }
            }

            root.spawnIndex = (spawn && spawn.index !== undefined) ? Number(spawn.index) : -1;
            root.spawnValue = (spawn && spawn.value !== undefined) ? Number(spawn.value) : 0;
            commitTimer.restart();
        }
    }
    focus: true

    property int boardSize: 4
    property int boardDepth: 4
    property var values: [] // length = size*size*depth
    property string gameMode: "Static"
    property bool animating: false
    property var pendingValues: null
    property int spawnIndex: -1
    property int spawnValue: 0
    property var mergeValueByTo: ({})
    property int moveDuration: 160
    property int mergePopDelay: 160
    property int mergePopDuration: 120
    property int spawnDuration: 160

    ListModel {
        id: animModel
    }
    Timer {
        id: commitTimer
        interval: root.moveDuration + root.mergePopDelay + root.mergePopDuration
        repeat: false
        onTriggered: {
            if (root.pendingValues !== undefined && root.pendingValues !== null) {
                root.values = root.pendingValues;
            }
            root.pendingValues = null;
            root.spawnIndex = -1;
            root.spawnValue = 0;
            root.animating = false;
        }
    }

    signal moveRequested(string direction) // left/right/forward/back/up/down

    readonly property int safeSize: Math.max(1, boardSize)
    readonly property int safeDepth: Math.max(1, boardDepth)
    readonly property int cellCount: safeSize * safeSize * safeDepth

    function valueAt(i) {
        if (!values || i < 0 || i >= values.length)
            return 0;
        var v = values[i];
        return (v === undefined || v === null) ? 0 : v;
    }

    function seedValues() {
        if (game2048 && game2048.on_ResetGame3D_emitted) {
            game2048.on_ResetGame3D_emitted(gameMode, [safeSize, safeSize, safeDepth]);
            return;
        }
        // fallback: local deterministic seed
        var n = cellCount;
        var arr = new Array(n);
        for (var i = 0; i < n; i++)
            arr[i] = 0;
        function idx(x, y, z) {
            return z * safeSize * safeSize + y * safeSize + x;
        }
        arr[idx(0, 0, 0)] = 2;
        values = arr;
    }

    Component.onCompleted: {
        // Main.qml will call seedValues() after Loader.Ready.
        // Avoid triggering an extra backend reset here.
        forceActiveFocus();
    }

    Keys.onPressed: function (event) {
        switch (event.key) {
        case Qt.Key_Left:
        case Qt.Key_A:
            moveRequested("left");
            if (game2048 && game2048.on_Left3D_operated)
                game2048.on_Left3D_operated(gameMode, [safeSize, safeSize, safeDepth]);
            event.accepted = true;
            break;
        case Qt.Key_Right:
        case Qt.Key_D:
            moveRequested("right");
            if (game2048 && game2048.on_Right3D_operated)
                game2048.on_Right3D_operated(gameMode, [safeSize, safeSize, safeDepth]);
            event.accepted = true;
            break;
        case Qt.Key_Up:
        case Qt.Key_W:
            moveRequested("forward");
            if (game2048 && game2048.on_Forward3D_operated)
                game2048.on_Forward3D_operated(gameMode, [safeSize, safeSize, safeDepth]);
            event.accepted = true;
            break;
        case Qt.Key_Down:
        case Qt.Key_S:
            moveRequested("back");
            if (game2048 && game2048.on_Back3D_operated)
                game2048.on_Back3D_operated(gameMode, [safeSize, safeSize, safeDepth]);
            event.accepted = true;
            break;
        case Qt.Key_Q:
            moveRequested("down");
            if (game2048 && game2048.on_Down3D_operated)
                game2048.on_Down3D_operated(gameMode, [safeSize, safeSize, safeDepth]);
            event.accepted = true;
            break;
        case Qt.Key_E:
            moveRequested("up");
            if (game2048 && game2048.on_Up3D_operated)
                game2048.on_Up3D_operated(gameMode, [safeSize, safeSize, safeDepth]);
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
                        text: "3D 立方体"
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        color: "#e5e7eb"
                    }
                    Text {
                        Layout.fillWidth: true
                        text: safeSize + "×" + safeSize + "×" + safeDepth
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
                            text: "拖拽旋转 / 滚轮缩放 / WASD+QE"
                            font.pixelSize: 11
                            color: "#cbd5e1"
                        }
                    }
                }
            }
        }

        Item {
            id: viewport
            Layout.fillWidth: true
            Layout.fillHeight: true

            property real yaw: 35
            property real pitch: 20
            property real distance: 520

            readonly property vector3d target: Qt.vector3d(0, 0, 0)

            function updateCamera() {
                var yawRad = yaw * Math.PI / 180.0;
                var pitchRad = pitch * Math.PI / 180.0;

                var x = distance * Math.sin(yawRad) * Math.cos(pitchRad);
                var y = distance * Math.sin(pitchRad);
                var z = distance * Math.cos(yawRad) * Math.cos(pitchRad);

                camera.position = Qt.vector3d(x, y, z);
                camera.eulerRotation = Qt.vector3d(-pitch, yaw, 0);
            }

            Component.onCompleted: updateCamera()
            onYawChanged: updateCamera()
            onPitchChanged: updateCamera()
            onDistanceChanged: updateCamera()

            View3D {
                id: view3d
                anchors.fill: parent

                environment: SceneEnvironment {
                    clearColor: "#0b1220"
                    backgroundMode: SceneEnvironment.Color
                    antialiasingMode: SceneEnvironment.MSAA
                    antialiasingQuality: SceneEnvironment.High
                }

                PerspectiveCamera {
                    id: camera
                    clipNear: 1
                    clipFar: 4000
                    fieldOfView: 50
                }

                DirectionalLight {
                    eulerRotation.x: -35
                    eulerRotation.y: 25
                    brightness: 1.2
                }
                PointLight {
                    position: Qt.vector3d(0, 320, 320)
                    brightness: 220
                    color: "#a78bfa"
                }

                Node {
                    id: sceneRoot
                    readonly property int safeMax: Math.max(root.safeSize, root.safeDepth)
                    readonly property real step: Math.max(18, 160 / safeMax)
                    readonly property real cubeEdge: step * 0.76
                    readonly property real cubeScale: cubeEdge / 100.0
                    readonly property real labelScale: cubeEdge / 100.0
                    readonly property real labelEpsilon: 0.6
                    readonly property real boardHalfX: (root.safeSize - 1) / 2.0 * step
                    readonly property real boardHalfY: (root.safeSize - 1) / 2.0 * step
                    readonly property real boardHalfZ: (root.safeDepth - 1) / 2.0 * step

                    function idxToPos(idx) {
                        var xId = idx % root.safeSize;
                        var yId = Math.floor(idx / root.safeSize) % root.safeSize;
                        var zId = Math.floor(idx / (root.safeSize * root.safeSize));
                        var cx = (xId - (root.safeSize - 1) / 2.0) * step;
                        var cy = (yId - (root.safeSize - 1) / 2.0) * step;
                        var cz = (zId - (root.safeDepth - 1) / 2.0) * step;
                        return Qt.vector3d(cx, cy, cz);
                    }

                    PrincipledMaterial {
                        id: emptyMat
                        baseColor: "#111827"
                        roughness: 0.55
                        metalness: 0.0
                        opacity: 0.70
                    }

                    PrincipledMaterial {
                        id: filledMat
                        baseColor: "#6d28d9"
                        roughness: 0.40
                        metalness: 0.0
                        opacity: 0.88
                    }

                    // 坐标轴提示：放在棋盘侧边，随视角(相机轨道)一起呈现旋转效果
                    Node {
                        id: axisHint
                        position: Qt.vector3d(sceneRoot.boardHalfX + sceneRoot.step * 1.2, -sceneRoot.boardHalfY - sceneRoot.step * 1.2, -sceneRoot.boardHalfZ - sceneRoot.step * 1.2)
                        visible: true

                        readonly property real axisLen: sceneRoot.step * 0.9
                        readonly property real shaftR: Math.max(0.6, sceneRoot.step * 0.04)
                        readonly property real headLen: sceneRoot.step * 0.22
                        readonly property real headR: Math.max(1.0, sceneRoot.step * 0.07)

                        DefaultMaterial {
                            id: axisMat
                            lighting: DefaultMaterial.NoLighting
                            cullMode: Material.NoCulling
                            diffuseColor: "#e5e7eb"
                            opacity: 0.92
                        }
                        DefaultMaterial {
                            id: axisMatZ
                            lighting: DefaultMaterial.NoLighting
                            cullMode: Material.NoCulling
                            diffuseColor: "#9ca3af"
                            opacity: 0.88
                        }

                        Node {
                            id: axisX
                            Model {
                                source: "#Cylinder"
                                eulerRotation: Qt.vector3d(0, 0, -90)
                                position: Qt.vector3d(axisHint.axisLen * 0.45, 0, 0)
                                scale: Qt.vector3d(axisHint.shaftR / 50.0, axisHint.axisLen / 100.0, axisHint.shaftR / 50.0)
                                materials: [axisMat]
                            }
                            Model {
                                source: "#Cone"
                                eulerRotation: Qt.vector3d(0, 0, -90)
                                position: Qt.vector3d(axisHint.axisLen + axisHint.headLen * 0.15, 0, 0)
                                scale: Qt.vector3d(axisHint.headR / 50.0, axisHint.headLen / 100.0, axisHint.headR / 50.0)
                                materials: [axisMat]
                            }
                            Node {
                                eulerRotation: camera.eulerRotation
                                Model {
                                    source: "#Rectangle"
                                    position: Qt.vector3d(axisHint.axisLen + axisHint.headLen * 0.55, 0, sceneRoot.cubeEdge / 2.0 + sceneRoot.labelEpsilon)
                                    scale: Qt.vector3d(sceneRoot.labelScale * 0.55, sceneRoot.labelScale * 0.55, 1)
                                    materials: [
                                        DefaultMaterial {
                                            lighting: DefaultMaterial.NoLighting
                                            cullMode: Material.NoCulling
                                            opacity: 0.98
                                            diffuseMap: Texture {
                                                minFilter: Texture.Linear
                                                magFilter: Texture.Linear
                                                generateMipmaps: true
                                                sourceItem: Rectangle {
                                                    width: 96
                                                    height: 96
                                                    radius: 18
                                                    color: "#0f172a"
                                                    border.color: "#2a3446"
                                                    border.width: 2
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: "X"
                                                        font.pixelSize: 48
                                                        font.weight: Font.DemiBold
                                                        color: "#e5e7eb"
                                                    }
                                                }
                                            }
                                        }
                                    ]
                                }
                            }
                        }

                        Node {
                            id: axisY
                            Model {
                                source: "#Cylinder"
                                position: Qt.vector3d(0, axisHint.axisLen * 0.45, 0)
                                scale: Qt.vector3d(axisHint.shaftR / 50.0, axisHint.axisLen / 100.0, axisHint.shaftR / 50.0)
                                materials: [axisMat]
                            }
                            Model {
                                source: "#Cone"
                                position: Qt.vector3d(0, axisHint.axisLen + axisHint.headLen * 0.15, 0)
                                scale: Qt.vector3d(axisHint.headR / 50.0, axisHint.headLen / 100.0, axisHint.headR / 50.0)
                                materials: [axisMat]
                            }
                            Node {
                                eulerRotation: camera.eulerRotation
                                Model {
                                    source: "#Rectangle"
                                    position: Qt.vector3d(0, axisHint.axisLen + axisHint.headLen * 0.55, sceneRoot.cubeEdge / 2.0 + sceneRoot.labelEpsilon)
                                    scale: Qt.vector3d(sceneRoot.labelScale * 0.55, sceneRoot.labelScale * 0.55, 1)
                                    materials: [
                                        DefaultMaterial {
                                            lighting: DefaultMaterial.NoLighting
                                            cullMode: Material.NoCulling
                                            opacity: 0.98
                                            diffuseMap: Texture {
                                                minFilter: Texture.Linear
                                                magFilter: Texture.Linear
                                                generateMipmaps: true
                                                sourceItem: Rectangle {
                                                    width: 96
                                                    height: 96
                                                    radius: 18
                                                    color: "#0f172a"
                                                    border.color: "#2a3446"
                                                    border.width: 2
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: "Y"
                                                        font.pixelSize: 48
                                                        font.weight: Font.DemiBold
                                                        color: "#e5e7eb"
                                                    }
                                                }
                                            }
                                        }
                                    ]
                                }
                            }
                        }

                        Node {
                            id: axisZ
                            Model {
                                source: "#Cylinder"
                                eulerRotation: Qt.vector3d(90, 0, 0)
                                position: Qt.vector3d(0, 0, axisHint.axisLen * 0.45)
                                scale: Qt.vector3d(axisHint.shaftR / 50.0, axisHint.axisLen / 100.0, axisHint.shaftR / 50.0)
                                materials: [axisMatZ]
                            }
                            Model {
                                source: "#Cone"
                                eulerRotation: Qt.vector3d(90, 0, 0)
                                position: Qt.vector3d(0, 0, axisHint.axisLen + axisHint.headLen * 0.15)
                                scale: Qt.vector3d(axisHint.headR / 50.0, axisHint.headLen / 100.0, axisHint.headR / 50.0)
                                materials: [axisMatZ]
                            }
                            Node {
                                eulerRotation: camera.eulerRotation
                                Model {
                                    source: "#Rectangle"
                                    position: Qt.vector3d(0, 0, axisHint.axisLen + axisHint.headLen * 0.55 + sceneRoot.cubeEdge / 2.0 + sceneRoot.labelEpsilon)
                                    scale: Qt.vector3d(sceneRoot.labelScale * 0.55, sceneRoot.labelScale * 0.55, 1)
                                    materials: [
                                        DefaultMaterial {
                                            lighting: DefaultMaterial.NoLighting
                                            cullMode: Material.NoCulling
                                            opacity: 0.98
                                            diffuseMap: Texture {
                                                minFilter: Texture.Linear
                                                magFilter: Texture.Linear
                                                generateMipmaps: true
                                                sourceItem: Rectangle {
                                                    width: 96
                                                    height: 96
                                                    radius: 18
                                                    color: "#0f172a"
                                                    border.color: "#2a3446"
                                                    border.width: 2
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: "Z"
                                                        font.pixelSize: 48
                                                        font.weight: Font.DemiBold
                                                        color: "#e5e7eb"
                                                    }
                                                }
                                            }
                                        }
                                    ]
                                }
                            }
                        }
                    }

                    Repeater3D {
                        id: cubeRep
                        model: root.cellCount

                        delegate: Node {
                            readonly property int idx: index
                            readonly property int xId: idx % root.safeSize
                            readonly property int yId: Math.floor(idx / root.safeSize) % root.safeSize
                            readonly property int zId: Math.floor(idx / (root.safeSize * root.safeSize))

                            readonly property real cx: (xId - (root.safeSize - 1) / 2.0) * sceneRoot.step
                            readonly property real cy: (yId - (root.safeSize - 1) / 2.0) * sceneRoot.step
                            readonly property real cz: (zId - (root.safeDepth - 1) / 2.0) * sceneRoot.step

                            position: Qt.vector3d(cx, cy, cz)

                            readonly property int v: root.valueAt(idx)
                            readonly property bool showFilled: (!root.animating) && v > 0

                            Model {
                                source: "#Cube"
                                scale: Qt.vector3d(sceneRoot.cubeScale, sceneRoot.cubeScale, sceneRoot.cubeScale)
                                materials: [showFilled ? filledMat : emptyMat]
                            }

                            Node {
                                visible: showFilled
                                eulerRotation: camera.eulerRotation

                                Model {
                                    source: "#Rectangle"
                                    position: Qt.vector3d(0, 0, sceneRoot.cubeEdge / 2.0 + sceneRoot.labelEpsilon)
                                    scale: Qt.vector3d(sceneRoot.labelScale, sceneRoot.labelScale, 1)
                                    materials: [
                                        DefaultMaterial {
                                            lighting: DefaultMaterial.NoLighting
                                            cullMode: Material.NoCulling
                                            opacity: 0.98
                                            diffuseMap: Texture {
                                                minFilter: Texture.Linear
                                                magFilter: Texture.Linear
                                                generateMipmaps: true
                                                sourceItem: Rectangle {
                                                    width: 128
                                                    height: 128
                                                    radius: 24
                                                    color: "#0f172a"
                                                    border.color: "#a78bfa"
                                                    border.width: 2
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: v
                                                        font.pixelSize: 56
                                                        font.weight: Font.DemiBold
                                                        color: "#e5e7eb"
                                                    }
                                                }
                                            }
                                        }
                                    ]
                                }
                            }
                        }
                    }
                }

                // 动画 overlay：使用 trace 的 from/to 做真实位移 + 合并强调 + 生成弹出
                Node {
                    id: animRoot
                    visible: root.animating
                    Repeater3D {
                        model: animModel
                        delegate: Node {
                            id: tileNode
                            property int fromIdx: from
                            property int toIdx: to
                            property int value0: value
                            property bool isMerged: merged
                            property bool isPrimary: primary
                            property int mergeValue: mergeNewValue
                            property real tileScale: sceneRoot.cubeScale

                            position: sceneRoot.idxToPos(fromIdx)

                            Model {
                                source: "#Cube"
                                scale: Qt.vector3d(tileScale, tileScale, tileScale)
                                materials: [filledMat]
                            }

                            Node {
                                visible: value0 > 0
                                eulerRotation: camera.eulerRotation
                                Model {
                                    source: "#Rectangle"
                                    position: Qt.vector3d(0, 0, sceneRoot.cubeEdge / 2.0 + sceneRoot.labelEpsilon)
                                    scale: Qt.vector3d(sceneRoot.labelScale, sceneRoot.labelScale, 1)
                                    materials: [
                                        DefaultMaterial {
                                            lighting: DefaultMaterial.NoLighting
                                            cullMode: Material.NoCulling
                                            opacity: 0.98
                                            diffuseMap: Texture {
                                                minFilter: Texture.Linear
                                                magFilter: Texture.Linear
                                                generateMipmaps: true
                                                sourceItem: Rectangle {
                                                    width: 128
                                                    height: 128
                                                    radius: 24
                                                    color: "#0f172a"
                                                    border.color: "#a78bfa"
                                                    border.width: 2
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: (isMerged && isPrimary) ? mergeValue : value0
                                                        font.pixelSize: 56
                                                        font.weight: Font.DemiBold
                                                        color: "#e5e7eb"
                                                    }
                                                }
                                            }
                                        }
                                    ]
                                }
                            }

                            ParallelAnimation {
                                running: root.animating
                                PropertyAnimation {
                                    target: tileNode
                                    property: "position"
                                    from: sceneRoot.idxToPos(fromIdx)
                                    to: sceneRoot.idxToPos(toIdx)
                                    duration: root.moveDuration
                                    easing.type: Easing.InOutQuad
                                }
                                SequentialAnimation {
                                    PauseAnimation {
                                        duration: (isMerged && isPrimary) ? (root.moveDuration + root.mergePopDelay) : 0
                                    }
                                    NumberAnimation {
                                        target: tileNode
                                        property: "tileScale"
                                        to: (isMerged && isPrimary) ? (sceneRoot.cubeScale * 1.18) : sceneRoot.cubeScale
                                        duration: (isMerged && isPrimary) ? (root.mergePopDuration / 2) : 0
                                        easing.type: Easing.OutQuad
                                    }
                                    NumberAnimation {
                                        target: tileNode
                                        property: "tileScale"
                                        to: sceneRoot.cubeScale
                                        duration: (isMerged && isPrimary) ? (root.mergePopDuration / 2) : 0
                                        easing.type: Easing.InQuad
                                    }
                                }
                            }
                        }
                    }

                    // spawn tile pop
                    Node {
                        id: spawnNode
                        visible: root.animating && root.spawnIndex >= 0
                        property real spawnScale: 0.0
                        position: sceneRoot.idxToPos(root.spawnIndex)
                        Model {
                            source: "#Cube"
                            scale: Qt.vector3d(sceneRoot.cubeScale * spawnNode.spawnScale, sceneRoot.cubeScale * spawnNode.spawnScale, sceneRoot.cubeScale * spawnNode.spawnScale)
                            materials: [filledMat]
                        }
                        Node {
                            visible: root.spawnValue > 0
                            eulerRotation: camera.eulerRotation
                            Model {
                                source: "#Rectangle"
                                position: Qt.vector3d(0, 0, sceneRoot.cubeEdge / 2.0 + sceneRoot.labelEpsilon)
                                scale: Qt.vector3d(sceneRoot.labelScale, sceneRoot.labelScale, 1)
                                materials: [
                                    DefaultMaterial {
                                        lighting: DefaultMaterial.NoLighting
                                        cullMode: Material.NoCulling
                                        opacity: 0.98
                                        diffuseMap: Texture {
                                            minFilter: Texture.Linear
                                            magFilter: Texture.Linear
                                            generateMipmaps: true
                                            sourceItem: Rectangle {
                                                width: 128
                                                height: 128
                                                radius: 24
                                                color: "#0f172a"
                                                border.color: "#a78bfa"
                                                border.width: 2
                                                Text {
                                                    anchors.centerIn: parent
                                                    text: root.spawnValue
                                                    font.pixelSize: 56
                                                    font.weight: Font.DemiBold
                                                    color: "#e5e7eb"
                                                }
                                            }
                                        }
                                    }
                                ]
                            }
                        }
                        SequentialAnimation {
                            running: root.animating && root.spawnIndex >= 0
                            PauseAnimation {
                                duration: root.moveDuration
                            }
                            NumberAnimation {
                                target: spawnNode
                                property: "spawnScale"
                                to: 1.0
                                duration: root.spawnDuration
                                easing.type: Easing.OutBack
                            }
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton

                    property real lastX: 0
                    property real lastY: 0
                    property bool dragging: false

                    onPressed: function (mouse) {
                        dragging = true;
                        lastX = mouse.x;
                        lastY = mouse.y;
                    }
                    onReleased: dragging = false
                    onPositionChanged: function (mouse) {
                        if (!dragging)
                            return;
                        var dx = mouse.x - lastX;
                        var dy = mouse.y - lastY;
                        lastX = mouse.x;
                        lastY = mouse.y;

                        viewport.yaw = viewport.yaw - dx * 0.35;
                        viewport.pitch = Math.max(-80, Math.min(80, viewport.pitch + dy * 0.35));
                    }
                    onWheel: function (wheel) {
                        var delta = wheel.angleDelta.y / 120.0;
                        viewport.distance = Math.max(220, Math.min(1200, viewport.distance - delta * 40));
                        wheel.accepted = true;
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
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

            Button {
                Layout.preferredWidth: 140
                text: "重置视角"
                onClicked: {
                    viewport.yaw = 35;
                    viewport.pitch = 20;
                    viewport.distance = 520;
                }
                ControlsMaterial.Material.background: "#0f172a"
                ControlsMaterial.Material.foreground: "#e5e7eb"
            }
        }
    }

    onMoveRequested: function (dir) {
        lastMove.text = "Last move: " + dir;
    }
}
