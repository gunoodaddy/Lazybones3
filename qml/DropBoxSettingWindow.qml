import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 1.1
import gunoodaddy 1.0

Window {
	width: 300
	height: 100

	Column {
		anchors.centerIn: parent
		spacing: 5

		TextField {
			width: parent.width
			enabled: !lazybones.dropBox.running
			placeholderText: "Dropbox Folder"

			onTextChanged: {
				if(text.length <= 0) {
					text = lazybones.dropBox.defaultDropBoxPath;
				} else {
					lazybones.dropBox.dropBoxPath = text
				}
			}

			Component.onCompleted: {
				text = lazybones.dropBox.dropBoxPath;
			}
		}

		Row {
			spacing: 10
			height: 30
			Row {
				height: parent.height
				spacing: 5
				Text {
					text: "Interval msec"
					anchors.verticalCenter: parent.verticalCenter
				}
				SpinBox {
					anchors.verticalCenter: parent.verticalCenter
					width: 50
					onValueChanged: {
						lazybones.dropBox.screenShotIntervalSec = value
					}
					Component.onCompleted: {
						value = lazybones.dropBox.screenShotIntervalSec
					}
				}
			}
			Row {
				height: parent.height
				spacing: 5
				Text {
					text: "Rotate Count"
					anchors.verticalCenter: parent.verticalCenter
				}

				SpinBox {
					anchors.verticalCenter: parent.verticalCenter
					width: 50
					onValueChanged: {
						lazybones.dropBox.screenShotFileRotateCount = value
					}
					Component.onCompleted: {
						value = lazybones.dropBox.screenShotFileRotateCount
					}
				}
			}
		}

//		Button {
//			text: lazybones.dropBox.running ? "Stop" : "Start"
//			width: parent.width

//			onClicked: {
//				lazybones.dropBox.running = !lazybones.dropBox.running
//			}
//		}
	}
}
