import QtQuick
import QtQuick.Layouts
import QtQuick3D

Item {
    id: root

    // 2 for 2D (WASD), 3 for 3D (WASD+QE)
    property int dims: 2

    // Optional: a QtQuick3D camera (from Board3D) whose view we follow in 3D hint mode.
    property var sourceCamera: null

    function _scaledCameraPos(pos, desiredLen) {
        if (pos === undefined || pos === null)
            return Qt.vector3d(0, 0, desiredLen);
        var x = Number(pos.x);
        var y = Number(pos.y);
        var z = Number(pos.z);
        var len = Math.sqrt(x * x + y * y + z * z);
        if (!isFinite(len) || len < 1e-6)
            return Qt.vector3d(0, 0, desiredLen);
        var s = desiredLen / len;
        return Qt.vector3d(x * s, y * s, z * s);
    }

    implicitWidth: 240
    implicitHeight: 56

    readonly property color panelColor: "#111827"
    readonly property color panelBorder: "#2a3446"
    readonly property color textMain: "#e5e7eb"
    readonly property color textSub: "#9ca3af"

    component KeyCap: Rectangle {
        required property string label
        required property bool enabled

        radius: 7
        color: root.panelColor
        border.color: root.panelBorder
        border.width: 1
        opacity: enabled ? 1.0 : 0.35

        Text {
            anchors.centerIn: parent
            text: parent.label
            color: root.textMain
            font.pixelSize: 12
            font.weight: Font.DemiBold
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        Text {
            text: root.dims === 2 ? "键位 / 轴向 (2D)" : "键位 / 轴向 (3D)"
            font.pixelSize: 12
            color: root.textSub
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 38
            spacing: 10

            Loader {
                Layout.preferredWidth: 78
                Layout.preferredHeight: 38
                sourceComponent: root.dims === 2 ? keys2D : keys3D
            }

            Loader {
                Layout.fillWidth: true
                Layout.preferredHeight: 38
                sourceComponent: root.dims === 2 ? axis2D : axis3D
            }
        }
    }

    Component {
        id: keys2D

        Item {
            implicitWidth: 78
            implicitHeight: 38

            GridLayout {
                anchors.fill: parent
                columns: 3
                rows: 2
                columnSpacing: 4
                rowSpacing: 4

                Item {
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 16
                }
                KeyCap {
                    label: "W"
                    enabled: true
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 16
                }
                Item {
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 16
                }

                KeyCap {
                    label: "A"
                    enabled: true
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 16
                }
                KeyCap {
                    label: "S"
                    enabled: true
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 16
                }
                KeyCap {
                    label: "D"
                    enabled: true
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 16
                }
            }
        }
    }

    Component {
        id: keys3D

        Item {
            implicitWidth: 78
            implicitHeight: 38

            GridLayout {
                anchors.fill: parent
                columns: 3
                rows: 2
                columnSpacing: 4
                rowSpacing: 4

                KeyCap {
                    label: "Q"
                    enabled: true
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 16
                }
                KeyCap {
                    label: "W"
                    enabled: true
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 16
                }
                KeyCap {
                    label: "E"
                    enabled: true
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 16
                }

                KeyCap {
                    label: "A"
                    enabled: true
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 16
                }
                KeyCap {
                    label: "S"
                    enabled: true
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 16
                }
                KeyCap {
                    label: "D"
                    enabled: true
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 16
                }
            }
        }
    }

    Component {
        id: axis2D

        Item {
            implicitHeight: 38

            Rectangle {
                anchors.fill: parent
                radius: 10
                color: "#0f172a"
                border.color: root.panelBorder
                border.width: 1
            }

            Item {
                anchors.fill: parent
                anchors.margins: 8

                readonly property real cx: width / 2
                readonly property real cy: height / 2

                Rectangle {
                    x: 0
                    y: cy - 1
                    width: parent.width
                    height: 2
                    radius: 1
                    color: "#cbd5e1"
                    opacity: 0.8
                }

                Rectangle {
                    x: cx - 1
                    y: 0
                    width: 2
                    height: parent.height
                    radius: 1
                    color: "#cbd5e1"
                    opacity: 0.8
                }

                Text {
                    text: "W"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: -2
                    font.pixelSize: 10
                    color: root.textMain
                }
                Text {
                    text: "S"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: -2
                    font.pixelSize: 10
                    color: root.textMain
                }
                Text {
                    text: "A"
                    anchors.left: parent.left
                    anchors.leftMargin: -2
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 10
                    color: root.textMain
                }
                Text {
                    text: "D"
                    anchors.right: parent.right
                    anchors.rightMargin: -2
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 10
                    color: root.textMain
                }

                Text {
                    text: "X"
                    anchors.right: parent.right
                    anchors.rightMargin: -2
                    anchors.top: parent.top
                    anchors.topMargin: -2
                    font.pixelSize: 10
                    color: root.textSub
                }
                Text {
                    text: "Y"
                    anchors.left: parent.left
                    anchors.leftMargin: -2
                    anchors.top: parent.top
                    anchors.topMargin: -2
                    font.pixelSize: 10
                    color: root.textSub
                }
            }
        }
    }

    Component {
        id: axis3D

        Item {
            implicitHeight: 38

            Rectangle {
                anchors.fill: parent
                radius: 10
                color: "#0f172a"
                border.color: root.panelBorder
                border.width: 1
            }

            Item {
                anchors.fill: parent
                anchors.margins: 8

                // Real 3D axis triad that follows the active 3D camera view.
                View3D {
                    anchors.fill: parent
                    camera: hintCamera

                    environment: SceneEnvironment {
                        backgroundMode: SceneEnvironment.Color
                        clearColor: Qt.rgba(0, 0, 0, 0)
                        antialiasingMode: SceneEnvironment.MSAA
                        antialiasingQuality: SceneEnvironment.Medium
                    }

                    PerspectiveCamera {
                        id: hintCamera
                        clipNear: 0.1
                        clipFar: 2000
                        fieldOfView: 45

                        position: root.sourceCamera ? root._scaledCameraPos(root.sourceCamera.position, 140) : Qt.vector3d(60, 40, 140)
                        eulerRotation: root.sourceCamera ? root.sourceCamera.eulerRotation : Qt.vector3d(-20, 35, 0)
                    }

                    DirectionalLight {
                        eulerRotation.x: -35
                        eulerRotation.y: 25
                        brightness: 1.1
                    }

                    Node {
                        id: axisRoot
                        readonly property real shaftLen: 40
                        readonly property real headLen: 15
                        readonly property real r: 1.8
                        readonly property real headR: 3.2

                        DefaultMaterial {
                            id: matX
                            lighting: DefaultMaterial.NoLighting
                            cullMode: Material.NoCulling
                            diffuseColor: "#cbd5e1"
                        }
                        DefaultMaterial {
                            id: matY
                            lighting: DefaultMaterial.NoLighting
                            cullMode: Material.NoCulling
                            diffuseColor: "#a78bfa"
                        }
                        DefaultMaterial {
                            id: matZ
                            lighting: DefaultMaterial.NoLighting
                            cullMode: Material.NoCulling
                            diffuseColor: "#6d28d9"
                        }

                        // +X
                        Node {
                            eulerRotation.z: -90
                            Model {
                                source: "#Cylinder"
                                position: Qt.vector3d(0, axisRoot.shaftLen / 2, 0)
                                scale: Qt.vector3d(axisRoot.r / 50.0, axisRoot.shaftLen / 100.0, axisRoot.r / 50.0)
                                materials: [matX]
                            }
                            Model {
                                source: "#Cone"
                                position: Qt.vector3d(0, axisRoot.shaftLen + axisRoot.headLen / 2, 0)
                                scale: Qt.vector3d(axisRoot.headR / 50.0, axisRoot.headLen / 100.0, axisRoot.headR / 50.0)
                                materials: [matX]
                            }
                        }

                        // +Y
                        Model {
                            source: "#Cylinder"
                            position: Qt.vector3d(0, axisRoot.shaftLen / 2, 0)
                            scale: Qt.vector3d(axisRoot.r / 50.0, axisRoot.shaftLen / 100.0, axisRoot.r / 50.0)
                            materials: [matY]
                        }
                        Model {
                            source: "#Cone"
                            position: Qt.vector3d(0, axisRoot.shaftLen + axisRoot.headLen / 2, 0)
                            scale: Qt.vector3d(axisRoot.headR / 50.0, axisRoot.headLen / 100.0, axisRoot.headR / 50.0)
                            materials: [matY]
                        }

                        // +Z
                        Node {
                            eulerRotation.x: 90
                            Model {
                                source: "#Cylinder"
                                position: Qt.vector3d(0, axisRoot.shaftLen / 2, 0)
                                scale: Qt.vector3d(axisRoot.r / 50.0, axisRoot.shaftLen / 100.0, axisRoot.r / 50.0)
                                materials: [matZ]
                            }
                            Model {
                                source: "#Cone"
                                position: Qt.vector3d(0, axisRoot.shaftLen + axisRoot.headLen / 2, 0)
                                scale: Qt.vector3d(axisRoot.headR / 50.0, axisRoot.headLen / 100.0, axisRoot.headR / 50.0)
                                materials: [matZ]
                            }
                        }
                    }
                }
            }
        }
    }
}
