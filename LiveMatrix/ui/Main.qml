import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtMultimedia 6.0 
import QtQuick.Controls.Material 2.15

Window {
    id: root
    width: 1024
    height: 640
    visible: true
    title: qsTr("LiveMatrix RTSP Professional")

    Material.theme: Material.Dark
    Material.accent: Material.Teal

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1a1a1a" }
            GradientStop { position: 1.0; color: "#0d0d0d" }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // שימוש ב-Pane במקום Rectangle כדי לתמוך ב-elevation (צל)
        Pane {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            Material.elevation: 4
            padding: 0
            
            background: Rectangle { color: "#252525" }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 12

                TextField {
                    id: urlField
                    Layout.fillWidth: true
                    placeholderText: "Enter RTSP URL..."
                    selectByMouse: true
                    background: Rectangle {
                        color: "#333333"
                        radius: 20
                        border.color: urlField.activeFocus ? Material.accent : "transparent"
                    }
                }

                Button {
                    text: qsTr("PLAY")
                    highlighted: true
                    onClicked: streamController.play(urlField.text)
                }

                Button {
                    text: qsTr("STOP")
                    flat: true
                    onClicked: streamController.stop()
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 20

            Rectangle {
                anchors.fill: parent
                color: "black"
                radius: 12
                clip: true
                border.color: "#333333"
                border.width: 1

                VideoOutput {
                    id: videoOutput
                    anchors.fill: parent
                    fillMode: VideoOutput.PreserveAspectFit
                    
                    // חשוב מאוד: קישור ה-Sink ל-C++ עם עליית הרכיב
                    Component.onCompleted: {
                        if (streamController && videoOutput.videoSink) {
                            streamController.setVideoSink(videoOutput.videoSink)
                        }
                    }

                    Text {
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.margins: 16
    
    text: "● " + (streamController.status ? streamController.status : "READY")
    
    color: (streamController.status === "PLAYING") ? "#00ff00" : "#ffaa00"
    font.pixelSize: 12
    font.bold: true
}
                }
            }
        }
    }
}