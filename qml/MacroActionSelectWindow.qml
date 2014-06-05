import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import QtQuick.Dialogs 1.1
import gunoodaddy 1.0

Window {
	id: inputWindow
	width: 500
	height: 650
	property alias insertIndex: spinInsertIndex.value
	property int __initIndex: -1

	function updateData() {
		var idx = lazybones.gameScreenManager.screenList.indexOf(macroAction.conditionScreen);
		if(idx >= 0)
			comboScreenName.currentIndex = idx
		else
			comboScreenName.currentIndex = 0

		for(var i = 0; i < actionTypeModel.count; i++) {
			if(actionTypeModel.get(i).type === macroAction.actionType) {
				comboActionType.currentIndex = i;
				break;
			}
		}

		for(i = 0; i < conditionTypeModel.count; i++) {
			if(conditionTypeModel.get(i).type === macroAction.conditionType) {
				comboConditionType.currentIndex = i;
				break;
			}
		}
	}

	property MacroAction macroAction: null
	property bool __addMode: false

	signal actionAdded();
	signal actionUpdated(MacroAction action, int from, int to);

	onVisibleChanged: {
		if(visible) {
			if(macroAction === null) {
				__addMode = true
				macroAction = lazybones.createMacroAction()
			} else {
				__addMode = false
			}

			__initIndex = insertIndex

			updateData()
		}
		macroAction.active = visible;
	}

	property MessageDialog messagePopup: MessageDialog{
		id: messagePopup
	}

	property InputMethodSelectWindow inputActionSelectWindow: InputMethodSelectWindow {
		inputAction: macroAction ? macroAction.inputAction : null
	}

	ListModel {
		id: actionTypeModel
		ListElement {
			text: "NoneAction"
			type: MacroAction.NoneAction
		}
		ListElement {
			text: "DoInputAction"
			type: MacroAction.DoInputAction
		}
		ListElement {
			text: "DoInputActionChunk"
			type: MacroAction.DoInputActionChunk
		}
		ListElement {
			text: "CheckElapsedTime"
			type: MacroAction.CheckElapsedTime
		}
		ListElement {
			text: "CheckScreen"
			type: MacroAction.CheckScreen
		}
		ListElement {
			text: "CheckDropBoxFileExist"
			type: MacroAction.CheckDropBoxFileExist
		}
		ListElement {
			text: "LoopWaitScreenDoInputActionEnd"
			type: MacroAction.LoopWaitScreenDoInputActionEnd
		}
		ListElement {
			text: "CheckLoopIndex"
			type: MacroAction.CheckLoopIndex
		}
		ListElement {
			text: "CheckLoopIndexModulus"
			type: MacroAction.CheckLoopIndexModulus
		}
		ListElement {
			text: "CheckDropBoxFileExist"
			type: MacroAction.CheckDropBoxFileExist
		}
		ListElement {
			text: "CheckPixelColor"
			type: MacroAction.CheckPixelColor
		}
		ListElement {
			text: "WaitScreen"
			type: MacroAction.WaitScreen
		}
		ListElement {
			text: "Sleep"
			type: MacroAction.Sleep
		}
		ListElement {
			text: "EatLegendaryItem"
			type: MacroAction.EatLegendaryItem
		}
		ListElement {
			text: "EatAllItem"
			type: MacroAction.EatAllItem
		}
		ListElement {
			text: "SaveScreenShot"
			type: MacroAction.SaveScreenShot
		}
		ListElement {
			text: "Goto"
			type: MacroAction.Goto
		}
		ListElement {
			text: "DisassembleAllItemWithoutLegendary"
			type: MacroAction.DisassembleAllItemWithoutLegendary
		}
		ListElement {
			text: "MoveItemsToStash"
			type: MacroAction.MoveItemsToStash
		}
	}

	ListModel {
		id: conditionTypeModel
		ListElement {
			text: "NoneCondition"
			type: MacroAction.None
		}
		ListElement {
			text: "LoopIfStart"
			type: MacroAction.LoopIfStart
		}
		ListElement {
			text: "LoopIfNotStart"
			type: MacroAction.LoopIfNotStart
		}
		ListElement {
			text: "LoopEnd"
			type: MacroAction.LoopEnd
		}
		ListElement {
			text: "IfStart"
			type: MacroAction.IfStart
		}
		ListElement {
			text: "IfNotStart"
			type: MacroAction.IfNotStart
		}
		ListElement {
			text: "IfEnd"
			type: MacroAction.IfEnd
		}
		ListElement {
			text: "Else"
			type: MacroAction.Else
		}
	}

	Column {
		id: topLayer
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.leftMargin: 5
		anchors.rightMargin: 5
		anchors.topMargin: 5

		spacing: 5

		Row {
			spacing: 5
			Text {
				text: "Insert Index"
				anchors.verticalCenter: parent.verticalCenter
			}

			SpinBox {
				id: spinInsertIndex
				anchors.verticalCenter: parent.verticalCenter
				minimumValue: 0
				maximumValue: macroListView.count
			}
		}

		ComboBox {
			id: comboConditionType
			width: 250
			model: conditionTypeModel
		}

		ComboBox {
			id: comboActionType
			width: 250
			model: actionTypeModel
		}

		ComboBox {
			id: comboScreenName
			width: 250
			model: lazybones.gameScreenManager.screenList
		}

		Item {
			width: parent.width
			height: textDesc.implicitHeight
			TextField {
				anchors.verticalCenter: parent.verticalCenter
				id: textDesc
				width: parent.width - buttonInputSelect.width - buttonRecord.width - 10
				anchors.left: parent.left
				readOnly: true
				font.pixelSize: 11
				text: macroAction ? macroAction.inputAction.description : "None"
			}

			Button {
				anchors.verticalCenter: parent.verticalCenter
				id: buttonInputSelect
				anchors.top: textDesc.top
				anchors.topMargin: -2
				anchors.right: buttonRecord.left
				anchors.rightMargin: 5
				width: 60
				text: "Select"
				onClicked: {
					inputActionSelectWindow.visible = true
					inputActionSelectWindow.raise()
				}
			}

			Button {
				anchors.verticalCenter: parent.verticalCenter
				id: buttonRecord
				anchors.top: textDesc.top
				anchors.topMargin: -2
				anchors.right: parent.right
				width: 140
				text: macroAction && macroAction.recording ? "Record Stop(F8)" : "Record Start(F7)"
				onClicked: {
					macroAction.recording = !macroAction.recording
				}
			}
		}

		TextField {
			id: textSleepMsec
			placeholderText: "Sleep msec"
			width: 150
			text: {
				if(macroAction === undefined || macroAction === null)
					return "";
				return macroAction.sleepMsec > 0 ? macroAction.sleepMsec : "";
			}
		}

		TextField {
			id: textMacroIndex
			placeholderText: "Macro Index"
			width: 150
			text: {
				if(macroAction === undefined || macroAction === null)
					return "";
				return macroAction.macroIndex >= 0 ? macroAction.macroIndex : "";
			}
		}

		TextField {
			id: textLoopIndex
			placeholderText: "Loop Index"
			width: 150
			text: {
				if(macroAction === undefined || macroAction === null)
					return "";
				return macroAction.loopIndex >= 0 ? macroAction.loopIndex : "";
			}
		}

		TextField {
			id: textfilePath
			placeholderText: "Dropbox File Name"
			width: parent.width
			text: {
				if(macroAction === undefined || macroAction === null)
					return "";
				return macroAction.filePath.length >= 0 ? macroAction.filePath : "";
			}
		}

		Row {
			spacing: 5
			TextField {
				id: textColorName
				placeholderText: "Color Name"
				width: 100
				text: {
					if(macroAction === undefined || macroAction === null)
						return "";
					return macroAction.color;
				}
			}

			TextField {
				id: textColorOffset
				placeholderText: "Color Offset"
				width: 90
				text: {
					if(macroAction === undefined || macroAction === null)
						return "";
					return macroAction.colorOffset > 0 ? macroAction.colorOffset : "";
				}
			}

			TextField {
				id: textColorRect
				placeholderText: "Color Rect : x, y, w, h"
				width: 150
				text: {
					if(macroAction === undefined || macroAction === null)
						return "";
					return lazybones.toRectString(macroAction.colorSearchRect);
				}
			}
		}

		// spacer..
		Rectangle {
			color: "transparent"
			width: 1
			height: 10
		}

		TextField {
			id: textComment
			placeholderText: "Comment"
			width: parent.width
			text: macroAction ? macroAction.comment : ""
			font.pixelSize: 11
		}

		Row {
			spacing: 10
			Button {
				anchors.verticalCenter: parent.verticalCenter
				text: "Test"
				onClicked: {
					macroAction.recording = false;
					macroAction.filePath = textfilePath.text
					macroAction.conditionType = conditionTypeModel.get(comboConditionType.currentIndex).type;
					macroAction.actionType = actionTypeModel.get(comboActionType.currentIndex).type;
					macroAction.test();
				}
			}
			Button {
				anchors.verticalCenter: parent.verticalCenter
				text: "Action Chunk Clear"
				onClicked: {
					macroAction.inputActionChunk.clear();
				}
			}

			Text {
				id: recordingTime
				anchors.verticalCenter: parent.verticalCenter
				font.bold: true
				font.pixelSize: 14
				text: {
					if(!macroAction)
						return "00:00";
					return macroWindow.toMMss(macroAction.inputActionChunk.totalTimeMsec);
				}
			}
		}
	}

	ScrollView {
		id: actionList
		anchors.top: topLayer.bottom
		anchors.bottom: bottomLayer.top
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.topMargin: 5
		anchors.leftMargin: 5
		anchors.rightMargin: 5
		anchors.bottomMargin: 5

		flickableItem.interactive: true
		frameVisible: true
		ListView {
			id: listView
			anchors.fill: parent
			clip: true

			onContentHeightChanged: {
				if(macroAction === null || !macroAction.recording)
					return;
				var tempContentY = contentHeight - height
				if(tempContentY < 0)
					tempContentY = 0;
				contentY = tempContentY;
			}

			model: macroAction ? macroAction.inputActionChunk : undefined
			delegate: Rectangle {
				width: parent.width
				height: 25

				color: {
					if(itemData.actionType === InputAction.KeyPress || itemData.actionType === InputAction.KeyRelease || itemData.actionType === InputAction.KeyInput)
						return "PaleGreen";
					if(itemData.actionType === InputAction.MouseLeftClick)
						return "Orange";
					if(itemData.actionType === InputAction.MouseLeftPress)
						return "#F75D59";
					if(itemData.actionType === InputAction.MouseLeftRelease)
						return "Coral";
					if(itemData.actionType === InputAction.MouseRightClick)
						return "LightSkyBlue";
					if(itemData.actionType === InputAction.MouseRightPress)
						return "LightCyan";
					if(itemData.actionType === InputAction.MouseRightRelease)
						return "Lavender";
					return "white"
				}

				// Action Title
				Text {
					anchors.left: parent.left
					anchors.leftMargin: 5
					anchors.verticalCenter: parent.verticalCenter
					font.pixelSize: 10
					text: {
						var str = itemData.description
						return str;
					}
				}

				MouseArea {
					anchors.fill: parent
					onClicked: {
						inputActionSelectWindow.inputAction = itemData
						inputActionSelectWindow.show()
						inputActionSelectWindow.raise()
					}
				}

				// Delete Button
				Rectangle {
					id: delButton
					radius: 3
					width: 60
					height: parent.height - 4
					border.width: 1
					border.color: "#BBBDBF"
					anchors.right: parent.right
					anchors.rightMargin: 5
					anchors.verticalCenter: parent.verticalCenter

					Text {
						text: "Del"
						anchors.centerIn: parent
					}

					MouseArea {
						anchors.fill: parent
						onClicked: {
							macroAction.removeInputActionChunkAt(index)
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

	//-----------------------------
	// Bottom Button
	//-----------------------------

	Row {
		id: bottomLayer
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottom: parent.bottom
		spacing: 10
		anchors.bottomMargin: 10
		Button {
			id: okButton
			text: __addMode ? "OK" : "Update"
			onClicked: {
				macroAction.recording = false;
				macroAction.actionType = actionTypeModel.get(comboActionType.currentIndex).type;
				macroAction.conditionType = conditionTypeModel.get(comboConditionType.currentIndex).type;
				macroAction.conditionScreen = comboScreenName.editText;
				macroAction.macroIndex = textMacroIndex.text.length > 0 ? textMacroIndex.text : 0;
				macroAction.sleepMsec = textSleepMsec.text.length > 0 ? textSleepMsec.text : 0;
				macroAction.comment = textComment.text;
				macroAction.loopIndex = textLoopIndex.text.length > 0 ? textLoopIndex.text : -1;
				macroAction.color = textColorName.text;
				macroAction.colorOffset = textColorOffset.text.length > 0 ? textColorOffset.text : 0;
				macroAction.colorSearchRect = lazybones.toStringRect(textColorRect.text);
				macroAction.filePath = textfilePath.text

				if((macroAction.blockEndType() === false && macroAction.conditionType !== MacroAction.Else) && macroAction.actionType <= 0) {
					messagePopup.text = "Select a action type."
					messagePopup.visible = true
					return
				}

				if(macroAction.conditionScreen.length <= 0 &&
						(macroAction.actionType === MacroAction.CheckScreen
						 && macroAction.actionType === MacroAction.WaitScreen
						 && macroAction.actionType === MacroAction.LoopWaitScreenDoInputActionEnd)) {
					messagePopup.text = "Select a screen name."
					messagePopup.visible = true
					return
				}

				if(macroAction.actionType === MacroAction.DoInputAction && macroAction.inputAction.actionType <= 0) {
					messagePopup.text = "Select any input action."
					messagePopup.visible = true
					return
				}

				if(macroAction.actionType === MacroAction.DoInputActionChunk && macroAction.inputActionChunk.count <= 0) {
					messagePopup.text = "Input Action Chunk is empty. please push record button."
					messagePopup.visible = true
					return
				}

				if(macroAction.actionType === MacroAction.Sleep && macroAction.sleepMsec <= 0) {
					messagePopup.text = "Input sleep msec."
					messagePopup.visible = true
					return
				}

				if(macroAction.actionType === MacroAction.Goto && macroAction.macroIndex < 0) {
					messagePopup.text = "Input a macro index."
					messagePopup.visible = true
					return
				}

				if((macroAction.actionType === MacroAction.CheckLoopIndex || macroAction.actionType === MacroAction.CheckLoopIndexModulus)&& macroAction.loopIndex < 0) {
					messagePopup.text = "Input a loop index."
					messagePopup.visible = true
					return
				}

				if((macroAction.actionType === MacroAction.CheckDropBoxFileExist
					|| macroAction.actionType === MacroAction.SaveScreenShot)
						&& textfilePath.text.length < 0) {
					messagePopup.text = "Input any file path"
					messagePopup.visible = true
					return
				}

				if(macroAction.actionType === MacroAction.LoopWaitScreenDoInputActionEnd &&
						(macroAction.inputAction.actionType <= 0
						 || macroAction.conditionScreen.length <= 0
						 )) {
					messagePopup.text = "Select any input action or input screen name."
					messagePopup.visible = true
					return
				}

				if(__addMode)
					actionAdded();
				else
				{
					actionUpdated(macroAction, __initIndex, insertIndex);
				}

				hide();
			}
		}

		Button {
			id: cancelButton
			text: "Cancel"
			onClicked: {
				macroAction.recording = false;
				inputWindow.hide()
			}
		}
	}
}

