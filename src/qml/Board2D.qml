import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material as ControlsMaterial
import QtQuick.Layouts

Item {
	id: root
	focus: true

	property int rows: 4
	property int columns: 4
	property var values: [] // length = rows*columns; 0 means empty

	signal moveRequested(string direction) // up/down/left/right

	readonly property int cellCount: Math.max(0, rows) * Math.max(0, columns)
	readonly property int safeRows: Math.max(1, rows)
	readonly property int safeColumns: Math.max(1, columns)

	function valueAt(i) {
		if (!values || i < 0 || i >= values.length) return 0
		var v = values[i]
		return (v === undefined || v === null) ? 0 : v
	}

	function seedValues() {
		var n = cellCount
		var arr = new Array(n)
		for (var i = 0; i < n; i++) arr[i] = 0

		function setAt(r, c, v) {
			if (r < 0 || c < 0 || r >= rows || c >= columns) return
			arr[r * columns + c] = v
		}

		setAt(0, 0, 2)
		setAt(1, 1, 4)
		setAt(2, 2, 8)
		setAt(rows - 1, columns - 1, 16)
		values = arr
	}

	Component.onCompleted: {
		seedValues()
		forceActiveFocus()
	}

	onRowsChanged: seedValues()
	onColumnsChanged: seedValues()

	Keys.onPressed: function(event) {
		switch (event.key) {
		case Qt.Key_Up:
		case Qt.Key_W:
			moveRequested("up")
			event.accepted = true
			break
		case Qt.Key_Down:
		case Qt.Key_S:
			moveRequested("down")
			event.accepted = true
			break
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

			readonly property real pad: 10
			readonly property real maxGridWidth: width
			readonly property real maxGridHeight: height - footer.implicitHeight - 10

			readonly property real spacing: Math.max(6, Math.floor(Math.min(maxGridWidth, maxGridHeight) / 64))
			readonly property real cellSize: {
				var w = (maxGridWidth - spacing * (safeColumns - 1)) / safeColumns
				var h = (maxGridHeight - spacing * (safeRows - 1)) / safeRows
				return Math.max(18, Math.floor(Math.min(w, h)))
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
									GradientStop { position: 0.0; color: "#0f172a" }
									GradientStop { position: 1.0; color: "#0b1220" }
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
									var v = root.valueAt(index)
									return v > 0 ? v : ""
								}
								font.pixelSize: Math.max(12, Math.floor(width * 0.34))
								font.weight: Font.DemiBold
								color: "#e5e7eb"
							}
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

	onMoveRequested: function(dir) {
		lastMove.text = "Last move: " + dir
	}
}
