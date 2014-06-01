import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import QtQuick.Dialogs 1.1
import gunoodaddy 1.0

Window {
	id: screenSettingWindow
	width: 450
	height: 400

	property GameScreenData screenData: null

	function checkInputValue() {
		if(textName.length <= 0) {
			messagePopup.text = "Input Screen Name."
			messagePopup.visible = true
			return false;
		}
		if(textPixelX.length <= 0) {
			messagePopup.text = "Input Pixel X."
			messagePopup.visible = true
			return false;
		}
		if(textPixelY.length <= 0) {
			messagePopup.text = "Input Pixel Y."
			messagePopup.visible = true
			return false;
		}
		if(textColor.length <= 0) {
			messagePopup.text = "Input Color text."
			messagePopup.visible = true
			return false;
		}
		return true;
	}

	MessageDialog {
		id: messagePopup
	}

	property CaptureWindow captureWindow: CaptureWindow {
		visible: false

		onVisibleChanged: {
			if(!visible) {
				screenSettingWindow.raise()
				screenSettingWindow.requestActivate()
			}
		}

		onClicked: {
			textPixelX.text = Math.round(captureWindow.mouseX)
			textPixelY.text = Math.round(captureWindow.mouseY)
			textColor.text = captureWindow.color
		}
	}

	GroupBox {
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.margins: 5

		id: inputLayer
		title: "Game Screen Info."

		Column {
			spacing: 5
			Row {
				spacing: 5
				TextField {
					id: textName
					width: 150
					font.pixelSize: 11
					placeholderText: "Screen Name"
				}

				TextField {
					id: textPixelX
					width: 60
					font.pixelSize: 11
					placeholderText: "Pixel X"
				}

				TextField {
					id: textPixelY
					width: textPixelX.width
					font.pixelSize: 11
					placeholderText: "Pixel Y"
				}

				TextField {
					id: textColor
					width: 60
					placeholderText: "Color"
				}

				Rectangle {
					border.color: "black"
					border.width: 1
					color: textColor.text
					width : 18
					height: 18
				}
			}

			Row {
				spacing: 5
				Button {
					text: "Select Pixel"
					onClicked: {
						captureWindow.show()
					}
				}

				Button {
					text: "Add"
					onClicked: {
						if(!checkInputValue())
							return;

						var screenData = lazybones.createGameScreenData();
						screenData.name = textName.text;
						screenData.pixelX = textPixelX.text;
						screenData.pixelY = textPixelY.text;
						screenData.color = textColor.text;
						if(!lazybones.gameScreenManager.addScreenData(screenData)) {
							messagePopup.text = "Failed to add. maybe same screen data exists.";
							messagePopup.visible = true;
						}
					}
				}
				Button {
					text: "Update"

					onClicked: {
						if(screenData === null) {
							messagePopup.text = "Please select data.";
							messagePopup.visible = true;
							return;
						}
						if(!checkInputValue())
							return;

						screenData.name = textName.text;
						screenData.pixelX = textPixelX.text;
						screenData.pixelY = textPixelY.text;
						screenData.color = textColor.text;
					}
				}

				Button {
					text: "Pick Color"
					onClicked: {
						textColor.text = lazybones.pixelColor(textPixelX.text, textPixelY.text)
					}
				}
			}
		}
	}

	ScrollView {
		anchors.top: inputLayer.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		anchors.topMargin: 10

		flickableItem.interactive: true
		//frameVisible: true

		ListView {
			id: listView
			anchors.fill: parent
			clip: true

			onContentHeightChanged: {
				var tempContentY = contentHeight - height
				if(tempContentY < 0)
					tempContentY = 0;
				contentY = tempContentY;
			}

			model: lazybones.gameScreenManager.screenModel
			delegate: Rectangle {
				width: parent.width
				height: 25

				Rectangle {
					id: colorBox
					anchors.left: parent.left
					anchors.leftMargin: 5
					anchors.verticalCenter: parent.verticalCenter
					border.color: "black"
					border.width: 1
					color: itemData.color
					width : 18
					height: 18
				}

				Text {
					anchors.left: colorBox.right
					anchors.leftMargin: 5
					anchors.verticalCenter: parent.verticalCenter
					text: itemData.name + " : " + itemData.pixelX + "x" + itemData.pixelY + ", color : " + itemData.color;
				}

				MouseArea {
					anchors.fill: parent
					onClicked: {
						textName.text = itemData.name
						textPixelX.text = itemData.pixelX
						textPixelY.text = itemData.pixelY
						textColor.text = itemData.color

						screenData = itemData
					}
				}

				// Delete Button
				Rectangle {
					id: delButton
					radius: width / 2
					width: 18
					height: parent.height - 6
					border.width: 1
					border.color: "black"
					color: "red"
					anchors.right: parent.right
					anchors.rightMargin: 5
					anchors.verticalCenter: parent.verticalCenter

					Text {
						text: "X"
						font.bold: true
						color: "white";
						anchors.centerIn: parent
					}

					MouseArea {
						anchors.fill: parent
						onClicked: {
							lazybones.gameScreenManager.removeScreenData(index)
						}
					}
				}

				// Bottom Line
				Rectangle {
					color: "#A0A0A0"
					width: parent.width
					height: 1
					anchors.bottom: parent.bottom
				}
			}
		}
	}
}
