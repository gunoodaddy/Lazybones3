import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import gunoodaddy 1.0

Window {
	width: 600
	height: 400

	Button {
		anchors.left: parent.left
		anchors.top: parent.top
		text : "Update"
		onClicked: {
			lazybones.currentMacroPreset.codeText = editor.text;
		}
	}

	TextArea {
		id: editor
		anchors.fill: parent
		anchors.topMargin: 30
	}

	onVisibleChanged: {
		editor.text = lazybones.currentMacroPreset.codeText;
	}
}
