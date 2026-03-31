import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15 as ControlsMaterial
import QtQuick.Layouts 1.15
import QtQuick3D

Item {
	id: root
	focus: true

	property int size: 4
	property int depth: 4
	property var values: [] // length = size*size*depth

	signal moveRequested(string direction) // left/right/forward/back/up/down

	readonly property int safeSize: Math.max(1, size)
	readonly property int safeDepth: Math.max(1, depth)
	readonly property int cellCount: safeSize * safeSize * safeDepth

	function valueAt(i) {
		if (!values || i < 0 || i >= values.length) return 0
		var v = values[i]
		return (v === undefined || v === null) ? 0 : v
	}

	function seedValues() {
		var n = cellCount
		var arr = new Array(n)
		for (var i = 0; i < n; i++) arr[i] = 0

		function idx(x, y, z) { return z * safeSize * safeSize + y * safeSize + x }

		arr[idx(0, 0, 0)] = 2
		arr[idx(1, 1, 0)] = 4
		arr[idx(2, 2, 1)] = 8
		arr[idx(safeSize - 1, safeSize - 1, safeDepth - 1)] = 16
		arr[idx(Math.floor(safeSize / 2), 0, Math.floor(safeDepth / 2))] = 32

		values = arr
	}

	Component.onCompleted: {
		seedValues()
		forceActiveFocus()
	}

	onSizeChanged: seedValues()
	onDepthChanged: seedValues()

	Keys.onPressed: function(event) {
		switch (event.key) {
		case Qt.Key_Left:
		case Qt.Key_A:
			moveRequested("left")
			event.accepted = true
			break
		case Qt.Key_Right:
		case Qt.Key_D:
			moveRequested("right")
			event.accepted = true
			break
		case Qt.Key_Up:
		case Qt.Key_W:
			moveRequested("forward")
			event.accepted = true
			break
		case Qt.Key_Down:
		case Qt.Key_S:
			moveRequested("back")
			event.accepted = true
			break
		case Qt.Key_Q:
			moveRequested("down")
			event.accepted = true
			break
		case Qt.Key_E:
			moveRequested("up")
			event.accepted = true
			break
		default:
			break
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
				var yawRad = yaw * Math.PI / 180.0
				var pitchRad = pitch * Math.PI / 180.0

				var x = distance * Math.sin(yawRad) * Math.cos(pitchRad)
				var y = distance * Math.sin(pitchRad)
				var z = distance * Math.cos(yawRad) * Math.cos(pitchRad)

				camera.position = Qt.vector3d(x, y, z)
				camera.eulerRotation = Qt.vector3d(-pitch, yaw, 0)
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

							Model {
								source: "#Cube"
								scale: Qt.vector3d(sceneRoot.cubeScale, sceneRoot.cubeScale, sceneRoot.cubeScale)
								materials: [v > 0 ? filledMat : emptyMat]
							}

							Node {
								visible: v > 0
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

				MouseArea {
					anchors.fill: parent
					hoverEnabled: true
					acceptedButtons: Qt.LeftButton

					property real lastX: 0
					property real lastY: 0
					property bool dragging: false

					onPressed: function(mouse) {
						dragging = true
						lastX = mouse.x
						lastY = mouse.y
					}
					onReleased: dragging = false
					onPositionChanged: function(mouse) {
						if (!dragging) return
						var dx = mouse.x - lastX
						var dy = mouse.y - lastY
						lastX = mouse.x
						lastY = mouse.y

						viewport.yaw = viewport.yaw - dx * 0.35
						viewport.pitch = Math.max(-80, Math.min(80, viewport.pitch + dy * 0.35))
					}
					onWheel: function(wheel) {
						var delta = wheel.angleDelta.y / 120.0
						viewport.distance = Math.max(220, Math.min(1200, viewport.distance - delta * 40))
						wheel.accepted = true
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
					viewport.yaw = 35
					viewport.pitch = 20
					viewport.distance = 520
				}
				ControlsMaterial.Material.background: "#0f172a"
				ControlsMaterial.Material.foreground: "#e5e7eb"
			}
		}
	}

	onMoveRequested: function(dir) {
		lastMove.text = "Last move: " + dir
	}
}
