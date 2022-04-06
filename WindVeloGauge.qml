import QtQuick 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4

Item {
    id: root

    property real peak: 0.0
    property real current: 0.0

    CircularGauge {
        id: peakGauge
        anchors.fill: parent

        value: peak
        minimumValue: 0.0
        maximumValue: 50.0

        style: CircularGaugeStyle {
            labelStepSize: 5.0
            tickmarkStepSize : 5.0

            tickmarkLabel: Text {
                text: styleData.value
                visible: styleData.value < 359.9
                font.pixelSize: outerRadius * 0.12
                color: "yellow"
            }

            needle: Rectangle {
                y: -outerRadius * 0.9
                implicitWidth: outerRadius * 0.03
                implicitHeight: outerRadius * 0.1
                antialiasing: true
                color: "orangered"
            }

            background: GaugeBackground {
                radius: outerRadius
            }

            foreground: null
        }
    }

    CircularGauge {
        id: currGauge
        anchors.fill: parent

        value: current
        minimumValue: 0.0
        maximumValue: 50.0

        style: CircularGaugeStyle {
            tickmark: null
            minorTickmark: null
            tickmarkLabel: null

            background: GaugeText {
                label: "Wind speed"
                unit: "[knots]"
                radius: outerRadius
                value: root.current
            }
        }
    }
}
