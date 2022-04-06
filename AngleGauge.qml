import QtQuick 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4

CircularGauge {
    minimumValue: 0.0
    maximumValue: 360.0

    style: CircularGaugeStyle {
        minimumValueAngle: 0.0
        maximumValueAngle: 360.0

        labelStepSize: 30.0
        tickmarkStepSize : 10.0

        tickmarkLabel: Text {
            text: styleData.value
            visible: styleData.value < 359.9
            font.pixelSize: outerRadius * 0.12
            color: "yellow"
        }
    }
}
