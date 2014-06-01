import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import QtQuick.Dialogs 1.1
import gunoodaddy 1.0

Window {
	id: screenListWindow
	width: 350
	height: 500

//	Button {
//		anchors.left: parent.left
//		anchors.leftMargin: 5
//		text: "Check All"
//		onClicked: {
//			var count = lazybones.gameScreenManager.screenList.count();
//			for(var i = 0; i < count; i++) {
//				var name = lazybones.gameScreenManager.screenList.get(i);
//				lazybones.gameScreenManager.setScreenMatchResult(name, lazybones.gameScreenManager.isCurrentScreen(name, true));
//				lazybones.gameScreenManager.screenList.updateAll();
//				listView.forceLayout()
//			}
//		}
//	}

	ScrollView {
//		anchors.topMargin: 30
		anchors.fill: parent
		flickableItem.interactive: true

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

			model: lazybones.gameScreenManager.screenList
			delegate: Rectangle {
				width: parent.width
				height: 25
				color: "white"

				function applyMatchResult() {
					var matched = lazybones.gameScreenManager.screenMatchResult(modelData)
					textMatch.text = matched  === 1 ? "Success" : matched === 0 ? "Failed" : "None"
					textMatch.color = matched  === 1 ? "green" : matched === 0 ? "red" : "black"
				}

				Text {
					anchors.left: parent.left
					anchors.leftMargin: 5
					anchors.verticalCenter: parent.verticalCenter
					text: modelData
				}

				Text {
					id: textMatch
					anchors.right: parent.right
					anchors.rightMargin: 5
					anchors.verticalCenter: parent.verticalCenter
					font.bold: true
					font.pixelSize: 14
					Component.onCompleted: {
						applyMatchResult();
					}
				}

				MouseArea {
					anchors.fill: parent
					onClicked: {
						lazybones.gameScreenManager.setScreenMatchResult(modelData, lazybones.gameScreenManager.isCurrentScreen(modelData, true));
						applyMatchResult();
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
