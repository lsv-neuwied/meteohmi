import QtQuick 2.0

Item {
    id: root

    property string label: 'label'
    property string unit: 'unit'
    property real value: 0.0
    property real radius: 0.0

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: root.radius * 0.43
        text: label
        font.pixelSize: root.radius * 0.1
        color: "white"
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.radius * 0.6
        text: unit
        font.pixelSize: root.radius * 0.1
        color: "white"
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.radius * 1.23

        implicitHeight: root.radius * 0.35
        implicitWidth: root.radius * 0.9
        radius: root.radius * 0.02

        color: "grey"

        Text {
            anchors.centerIn: parent
            text: value.toFixed(1)
            font.pixelSize: root.radius * 0.28
            color: "black"
        }
    }
}
