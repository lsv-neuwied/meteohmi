import QtQuick 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4

Item {
    function getTickmarkLabel(a) {
        if (Math.abs(a - 0.0) < 0.1) {
            return 'N';
        }
        if (Math.abs(a - 90.0) < 0.1) {
            return 'E';
        }
        if (Math.abs(a - 180.0) < 0.1) {
            return 'S';
        }
        if (Math.abs(a - 270.0) < 0.1) {
            return 'W';
        }
        if (Math.abs(a - 360.0) < 0.1) {
            return '';
        }
        var s = (a * 0.1).toFixed(0);
        if (s.length < 2) {
            return '0' + s;
        }
        return s;
    }

    id: root

    property real runway: 0.0
    property real average: 0.0
    property real current: 0.0

    CircularGauge {
        id: runwayGauge
        anchors.fill: parent

        value: runway

        minimumValue: 0.0
        maximumValue: 360.0

        style: CircularGaugeStyle {
            minimumValueAngle: 0.0
            maximumValueAngle: 360.0

            needle: Rectangle {
                y: outerRadius * 0.7
                implicitWidth: outerRadius * 0.05
                implicitHeight: outerRadius * 1.4
                antialiasing: true
                color: "darkgray"
            }

            background: GaugeBackground {
                radius: outerRadius
            }

            tickmarkLabel: null
            tickmark: null
            minorTickmark: null
            foreground: null
        }
    }


    CircularGauge {
        id: avgGauge
        anchors.fill: parent

        value: average

        minimumValue: 0.0
        maximumValue: 360.0

        style: CircularGaugeStyle {
            minimumValueAngle: 0.0
            maximumValueAngle: 360.0

            labelStepSize: 30.0

            tickmarkStepSize : 10.0

            tickmarkLabel: Text {
                text: getTickmarkLabel(styleData.value)
                font.pixelSize: outerRadius * 0.12
                color: "yellow"
            }

            needle: Rectangle {
                implicitWidth: outerRadius * 0.02
                implicitHeight: outerRadius * 0.65
                antialiasing: true
                color: "blue"
            }

            background: GaugeText {
                label: "Wind direction"
                unit: "[°]"
                radius: outerRadius
                value: root.current
            }

            foreground: null
        }
    }

    CircularGauge {
        id: currGauge
        anchors.fill: parent

        value: current
        minimumValue: 0.0
        maximumValue: 360.0

        style: CircularGaugeStyle {
            minimumValueAngle: 0.0
            maximumValueAngle: 360.0

            tickmark: null
            minorTickmark: null
            tickmarkLabel: null
        }
    }
}
