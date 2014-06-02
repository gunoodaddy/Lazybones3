import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Window 2.1
import gunoodaddy 1.0

Window {
	id: captureWindow
	width: Screen.desktopAvailableWidth
	height: Screen.desktopAvailableHeight
	flags: Qt.FramelessWindowHint | Qt.Window | Qt.WindowStaysOnTopHint
	//modality: Qt.ApplicationModal

	property real mouseX: -1
	property real mouseY: -1
	property color color
	property int mouseButton: Qt.LeftButton

	property real __intitialEffectOpacity: 1
	property int __captureId: 0

	signal clicked()

	Image {
		id: bgImage
		anchors.fill: parent
		source: "image://capture/"  + __captureId
		cache: false
	}

	onVisibleChanged: {
		if(visible) {
			raise();
			__captureId ++
			bgImage.source = "image://capture/" + __captureId;

			shutterEffect.visible = false
			shutterEffect.state = ""
			helloTextRect.opacity = 1
			timerText.start()
			positionEffect.opacity = __intitialEffectOpacity
			timerSprite.start()
			requestActivate();
		}
		console.log("onVisibleChanged : " + visible)
	}

	Rectangle {
		id: helloTextRect
		anchors.centerIn: parent

		color: "transparent"
		Text {
			text: "Click any position"
			color: "#D24A25"
			font.bold: true
			font.pixelSize: 70
			anchors.centerIn: parent
		}

		Behavior on opacity {
			NumberAnimation { properties:"opacity"; duration: 700 }
		}
	}

	Rectangle {
		x: mouseX - (width/2)
		y: mouseY - (height/2)
		radius: width / 2
		height: 5
		width: 5
		color: "red"
	}

	Rectangle {
		id: positionEffect
		x: mouseX - (width/2)
		y: mouseY - (height/2)
		opacity: __intitialEffectOpacity
		radius: width / 2
		height: 50
		width: 50
		color: "yellow"
		Behavior on opacity {
			NumberAnimation { properties:"opacity"; duration: 200 }
		}
	}

	Timer {
		id: timerText
		interval: 500
		repeat: false
		onTriggered: {
			helloTextRect.opacity = 0
		}
	}

	Timer {
		id: timerSprite
		interval: 150
		repeat: false
		onTriggered: {
			positionEffect.opacity = 0
		}
	}

	Rectangle {
		id: shutterEffect
		visible: false
		color: "white"
		anchors.fill: parent
		opacity: 0.7

		onOpacityChanged: {
			if(opacity === 0)
			{
				visibility = Window.Windowed
				captureWindow.hide()
			}
		}

		states: [
			State {
				name: "shutter"
				PropertyChanges { target: shutterEffect; opacity: 0 }
			}

		]

		transitions: [
			Transition {
				NumberAnimation { properties: "opacity"; easing.type: Easing.InOutQuad; duration: 200 }
			}
		]
	}

	MouseArea {
		anchors.fill: parent
		hoverEnabled: true
		onClicked: {
			captureWindow.mouseX = mouse.x
			captureWindow.mouseY = mouse.y
			captureWindow.mouseButton = mouse.button
			captureWindow.color = lazybones.pixelColor(Math.round(captureWindow.mouseX), Math.round(captureWindow.mouseY))
			positionEffect.opacity = __intitialEffectOpacity
			timerSprite.start()
			captureWindow.clicked()
			shutterEffect.visible = true
			shutterEffect.state = "shutter"
		}

		onPositionChanged: {

		}
	}
}
