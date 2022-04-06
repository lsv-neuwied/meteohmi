import QtQuick 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.0

ColumnLayout {
    id: root

    property real minimumValue: 0.0
    property real maximumValue: 0.0
    property real value: 0.0
    property string suffix: ''
    property string label: 'label'
    property string unit: 'unit'
    property int decimals: 1

    Text {
        Layout.alignment: Qt.AlignHCenter
        font.pixelSize: 24
        color: "white"
        text: label
    }

    Gauge {
        Layout.alignment: Qt.AlignHCenter
        Layout.fillHeight: true
        minimumValue: root.minimumValue
        maximumValue: root.maximumValue
        value: root.value
    }

    Text {
        Layout.alignment: Qt.AlignHCenter
        font.pixelSize: 24
        color: "yellow"
        text: value.toFixed(decimals) + suffix
    }

    Text {
        Layout.alignment: Qt.AlignHCenter
        font.pixelSize: 24
        color: "white"
        text: unit
    }
}
