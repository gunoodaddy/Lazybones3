import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 1.1
import QtQuick.Dialogs 1.1
import gunoodaddy 1.0

Window {
	id: macroWindow
	width: 660
	height: 800
	visible: true
	title: "Lazybones3 ver 1.5 - gunoodaddy 2014"

	onYChanged: {
		if(y < 0)
			y = 30;
	}

	onRunningIndexChanged: {
		if(lazybones.currentMacroPreset.running)
			macroListView.positionViewAtIndex(runningIndex, ListView.Center);
	}

	Lazybones {
		id: lazybones
	}

	property int runningIndex: lazybones.currentMacroPreset.currentIndex
	property int __deletedIndex: -1
	property int __statSize: 13

	property DropBoxSettingWindow dropboxWindow: DropBoxSettingWindow {
		onVisibleChanged: {
			if(!visible) {
				macroWindow.raise()
			}
		}
	}

	property GameScreenSettingWindow gameScreenWindow: GameScreenSettingWindow {
		onVisibleChanged: {
			if(!visible) {
				macroWindow.raise()
			}
		}
	}

	property GameScreenListWindow gameScreenListWindow: GameScreenListWindow {
		onVisibleChanged: {
			if(!visible) {
				macroWindow.raise()
			}
		}
	}

	property MacroActionSelectWindow selectWindow: MacroActionSelectWindow {
		onActionAdded: {
			lazybones.currentMacroPreset.insertAction(selectWindow.insertIndex, selectWindow.macroAction)
		}

		onActionUpdated: {
			if(from !== to) {
				lazybones.currentMacroPreset.moveAction(to, action);
			}
		}

		onVisibleChanged: {
			if(!visible) {
				macroWindow.raise()
			}
		}
	}

	property MessageDialog messagePopup: MessageDialog {
		text: "Are you sure you want to delete it?"
		standardButtons: StandardButton.Yes | StandardButton.No
		onYes: {
			if(__deletedIndex >= 0) {
				lazybones.currentMacroPreset.removeAction(__deletedIndex)
			}
		}
	}

	property MessageDialog warningPopup: MessageDialog {
	}

	property TextEditorWindow textEditor: TextEditorWindow {
	}

	function saveSetting() {
		if(comboPreset.editText.length > 0) {
			lazybones.currentMacroPreset.title = comboPreset.editText;
		}
		lazybones.save();
	}

	function toMMss(msec) {
		if(msec <= 0)
			return "00:00";
		var sec = msec / 1000;
		var MM = Math.floor(sec / 60);
		var ss = Math.floor(sec % 60);

		var strMM = MM < 10 ? ("0" + MM) : ("" + MM);
		var strss = ss < 10 ? ("0" + ss) : ("" + ss);
		return strMM + ":" + strss;
	}

	function toHHMMss(msec) {
		if(msec <= 0)
			return "00:00:00";
		var sec = msec / 1000;
		var HH = Math.floor((sec / 60) / 60);
		var MM = Math.floor((sec / 60) % 60);
		var ss = Math.floor(sec % 60);

		var strHH = HH < 10 ? ("0" + HH) : ("" + HH);
		var strMM = MM < 10 ? ("0" + MM) : ("" + MM);
		var strss = ss < 10 ? ("0" + ss) : ("" + ss);
		return strHH + ":" + strMM + ":" + strss;
	}

	Column {
		id: topLayer
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.leftMargin: 5
		anchors.topMargin: 5
		anchors.rightMargin: 5

		spacing: 5

		Row {
			spacing: 5

			ComboBox {
				id: comboPreset
				anchors.verticalCenter: parent.verticalCenter
				width: 250
				model: lazybones.macroPresetList
				editable: true
				onCurrentIndexChanged: {
					lazybones.currentMacroPresetIndex = currentIndex
				}
				onActiveFocusChanged: {
					if(!activeFocus) {
						if(editText.length > 0) {
							if(!lazybones.findMacroPreset(editText))
								lazybones.currentMacroPreset.title = editText
							else {
								warningPopup.text = "Already same title exists.";
								warningPopup.visible = true;
								editText = lazybones.currentMacroPreset.title;
							}
						}
					}
				}
			}
			Button {
				focus: true
				text: "New"
				anchors.verticalCenter: parent.verticalCenter
				onClicked: {
					lazybones.addNewMacroPreset(lazybones.makeNewMacroTitle());
					comboPreset.currentIndex = comboPreset.count - 1
				}
			}
			Button {
				focus: true
				text: "Save"
				anchors.verticalCenter: parent.verticalCenter
				onClicked: {
					saveSetting();
				}
			}
			Button {
				focus: true
				text: "Copy"
				anchors.verticalCenter: parent.verticalCenter
				onClicked: {
					lazybones.copyMacroPreset(comboPreset.currentIndex, comboPreset.editText + "_Copy");
					comboPreset.currentIndex = comboPreset.count - 1
				}
			}

			Button {
				focus: true
				text: "Remove"
				anchors.verticalCenter: parent.verticalCenter
				onClicked: {
					lazybones.removeMacroPreset(comboPreset.currentIndex);
					comboPreset.currentIndex = 0;
				}
			}

			Button {
				focus: true
				text: "Editor"
				anchors.verticalCenter: parent.verticalCenter
				onClicked: {
					textEditor.show();
				}
			}
		}

		Row {
			spacing: 5
			Button {
				text: "Dropbox Setting"
				onClicked: {
					dropboxWindow.show()
					dropboxWindow.requestActivate()
					dropboxWindow.raise()
				}
			}

			Button {
				text: "Screen Setting"
				onClicked: {
					gameScreenWindow.visible = true
					gameScreenWindow.requestActivate()
				}
			}

			Button {
				text: "Screen List"
				onClicked: {
					gameScreenListWindow.show()
					gameScreenListWindow.requestActivate()
					gameScreenListWindow.raise()
				}
			}
		}

		GroupBox {
			title: "Setting"
			width: parent.width
			Column {
				spacing: 5

				Row {
					spacing: 5
					TextField {
						id: textWinTitle
						width: 100
						font.pixelSize: 11
						text: lazybones.currentMacroPreset.windowTitle
						anchors.verticalCenter: parent.verticalCenter
					}
					TextField {
						id: textWinX
						width: 45
						text: lazybones.currentMacroPreset.windowRect.x
						anchors.verticalCenter: parent.verticalCenter
					}
					TextField {
						id: textWinY
						width: 45
						text: lazybones.currentMacroPreset.windowRect.y
						anchors.verticalCenter: parent.verticalCenter
					}
					TextField {
						id: textWinWidth
						width: 45
						text: lazybones.currentMacroPreset.windowRect.width
						anchors.verticalCenter: parent.verticalCenter
					}
					TextField {
						id: textWinHeight
						width: 45
						text: lazybones.currentMacroPreset.windowRect.height
						anchors.verticalCenter: parent.verticalCenter
					}
					Button {
						text: "Set"
						anchors.verticalCenter: parent.verticalCenter
						onClicked: {
							lazybones.currentMacroPreset.windowTitle = textWinTitle.text
							lazybones.currentMacroPreset.windowRect = Qt.rect(textWinX.text, textWinY.text, textWinWidth.text, textWinHeight.text);
							lazybones.currentMacroPreset.applyWindowRect();
						}
					}
				}


				Row {
					spacing: 5
					Text {
						anchors.verticalCenter: parent.verticalCenter
						text: "Hangup"
					}

					TextField {
						id: textHangupSec
						width: 45
						font.bold: true
						font.pixelSize: 14
						enabled: !lazybones.currentMacroPreset.running
						text: lazybones.currentMacroPreset.remainHangUpSeconds
						anchors.verticalCenter: parent.verticalCenter
					}

					Text {
						anchors.verticalCenter: parent.verticalCenter
						text: "sec"
					}

					Button {
						text: "Set"
						anchors.verticalCenter: parent.verticalCenter
						onClicked: {
							lazybones.currentMacroPreset.hangUpSeconds = textHangupSec.text;
							saveSetting();
						}
					}
				}
			}
		}

		Row {
			spacing: 5
			Button {
				text: "Add Action"
				anchors.verticalCenter: parent.verticalCenter
				onClicked: {
					selectWindow.insertIndex = macroListView.count
					selectWindow.macroAction = null
					gc()
					selectWindow.visible = true
					selectWindow.requestActivate()
				}
			}

			Button {
				text: lazybones.currentMacroPreset.running ? "Stop" : "Start"
				anchors.verticalCenter: parent.verticalCenter
				onClicked: {
					if(!lazybones.currentMacroPreset.running)
						lazybones.currentMacroPreset.clearStatistics();

					lazybones.currentMacroPreset.currentIndex = spinFromStart.value
					lazybones.currentMacroPreset.running = !lazybones.currentMacroPreset.running;
					saveSetting();
				}
			}

			SpinBox {
				anchors.verticalCenter: parent.verticalCenter
				id: spinFromStart
				value: lazybones.currentMacroPreset.running ? runningIndex : 0
				minimumValue: 0
				maximumValue: macroListView.count - 1
			}

			// Statistics
			Row {
				anchors.verticalCenter: parent.verticalCenter
				spacing: 10
				Text {
					font.weight: Font.Black
					font.pixelSize: __statSize
					color: "blue"
					style: Text.Raised; styleColor: "black"
					text: "LOOP : " + lazybones.currentMacroPreset.statsLoopCount
				}

				Text {
					font.weight: Font.Black
					font.pixelSize: __statSize
					color: "red"
					style: Text.Raised; styleColor: "black"
					text: "HANG : " + lazybones.currentMacroPreset.statsHangCount
				}

				Text {
					font.weight: Font.Black
					font.pixelSize: __statSize
					color: "#FF8000"
					style: Text.Raised; styleColor: "black"
					text: "ITEM : " + lazybones.currentMacroPreset.statsLegendaryCount
				}

				Text {
					font.weight: Font.Black
					font.pixelSize: __statSize
					color: "#000000"
					style: Text.Raised; styleColor: "black"
					text: {
						var str = "TIME : ";
						str += toHHMMss(lazybones.currentMacroPreset.statsStartRunningTime);
						str += " - ";
						str += toMMss(lazybones.currentMacroPreset.statsCurrLoopTime);
						str += "(" + toMMss(lazybones.currentMacroPreset.statsAvgLoopTime) + ")";
						return str;
					}
				}
			}
		}
	}

	Rectangle {
		anchors.fill: actionList
		visible: macroListView.count <= 0
		Text {
			text: "Empty Macro."
			font.pixelSize: 15
			anchors.centerIn: parent
		}
	}

	ScrollView {
		id: actionList
		anchors.top: topLayer.bottom
		anchors.margins: 5
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.right: parent.right

		flickableItem.interactive: true
		frameVisible: true
		ListView {
			id: macroListView
			anchors.fill: parent
			clip: true

			//			onContentHeightChanged: {
			//				var tempContentY = contentHeight - height
			//				if(tempContentY < 0)
			//					tempContentY = 0;
			//				contentY = tempContentY;
			//			}

			model: lazybones.currentMacroPreset.model

			delegate: Rectangle {
				width: parent.width - 1
				height: 25

				color: {
					// http://html-color-codes.info/
					switch(itemData.actionType) {
					case MacroAction.DoInputAction:	return "#E6F8E0";
					case MacroAction.DoInputActionChunk:	return "#CEECF5";
					}

					if(itemData.blockStartType())
						return "#F5A9A9";

					if(itemData.blockEndType())
						return "#F6CECE";

					if(itemData.blockRestartType())
						return "#F5A9A9";
					return "white"
				}

				Row {
					spacing: 10
					anchors.left: parent.left
					anchors.leftMargin: 5
					anchors.verticalCenter: parent.verticalCenter

					Text {
						text: {
							if(itemData === undefined)
								return "";
							var text = index + " : " + itemData.description;
							if(itemData.actionType === MacroAction.DoInputActionChunk) {
								text += ", Progress : ";
								text += itemData.inputActionChunk.currentIndex + "/" + itemData.inputActionChunk.count;
								text += " " + macroWindow.toMMss(itemData.inputActionChunk.currentTimeMsec);
							}

							return text;
						}
						font.pixelSize: 9
					}

					Text {
						text: itemData !== undefined ? itemData.comment : ""
						font.bold: true
						font.italic: true
						font.pixelSize: 9
						color: "darkgreen"
					}
				}

				MouseArea {
					anchors.fill: parent
					onClicked: {
						selectWindow.insertIndex = index
						selectWindow.macroAction = itemData
						selectWindow.show()
					}
				}

				// Delete Button
				Rectangle {
					id: delButton
					radius: width / 2
					width: 22
					height: parent.height - 8
					border.width: 1
					border.color: "#A4A4A4"
					color: "#F7D358"
					anchors.right: parent.right
					anchors.rightMargin: 5
					anchors.verticalCenter: parent.verticalCenter

					Text {
						text: "X"
						font.bold: true
						color: "#585858";
						anchors.centerIn: parent
					}

					MouseArea {
						anchors.fill: parent
						onClicked: {
							__deletedIndex = index
							messagePopup.visible = true
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

				// Focus rect
				Rectangle {
					visible: lazybones.currentMacroPreset.running && (index === lazybones.currentMacroPreset.currentIndex)
					anchors.fill: parent
					border.width: 3
					border.color: "red"
					color: "transparent"
					z: 99999
				}
			}
		}
	}
}
