import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtMultimedia 6.6
import QtQuick.Controls.Material 2.15


Window {
    id: root
    width: 960
    height: 540
    visible: true
    title: qsTr("RTSP Viewer (MVP)")

    Material.theme: Material.Dark
    Material.accent: Material.Teal

    color: "#202020"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: urlField
                Layout.fillWidth: true
                placeholderText: qsTr("rtsp://user:password@host:port/stream")
                text: ""
            }

            Button {
                text: qsTr("Play")
                onClicked: {
                    streamController.play(urlField.text)
                }
            }

            Button {
                text: qsTr("Stop")
                onClicked: {
                    streamController.stop()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Text {
                id: statusText
                Layout.fillWidth: true
                color: "#CCCCCC"
                text: qsTr("Status: ") + (streamController ? "" : "Not ready")

                Connections {
                    target: streamController
                    function onStatusChanged() { 
                        statusText.text = qsTr("Status: ") + streamController.status
                         }                
                }
            }
        }

        Text {
            id: errorText
            Layout.fillWidth: true
            color: "#FF6666"
            visible: text.length > 0

            Connections {
                target: streamController
                function onErrorOccurred(code, message) {
                    if (message && message.length > 0) {
                        errorText.text = qsTr("Error (%1): %2").arg(code).arg(message)
                    } else {
                        errorText.text = ""
                    }
                }
            }
        }

        Item {
            id: videoContainer
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                anchors.fill: parent
                color: "#101010"
                radius: 4

                VideoOutput {
                    id: videoOutput
                    anchors.fill: parent
                    anchors.margins: 4
                    fillMode: VideoOutput.PreserveAspectFit
                    Component.onCompleted: {
                        if (streamController && videoOutput.videoSink) {
                            streamController.setVideoSink(videoOutput.videoSink)
                        }
                    }
                }
            }
        }
    }
}
