import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4
import QtQuick.Window 2.2

ApplicationWindow {
    visible: true
    color: "black"

    ColumnLayout {
        anchors.fill: parent

        GridLayout {
            columns: 4
            Layout.fillHeight: true
            Layout.fillWidth: true

            WindDirGauge {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 5

                runway: meteo.runway
                current: meteo.windDir
                average: meteo.windDirAvg
            }


            LinearGauge {
                Layout.fillHeight: true
                Layout.margins: 10

                value: meteo.airTemp

                minimumValue: -20.0
                maximumValue: 50.0

                label: "Temp"
                unit: "°C"
            }

            LinearGauge {
                Layout.fillHeight: true
                Layout.margins: 10

                value: meteo.airPress
                suffix: meteo.airPressTrend

                minimumValue: 950.0
                maximumValue: 1050.0

                label: "QNH"
                unit: "hPa"
            }

            WindVeloGauge {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 5

                current: meteo.windVelo
                peak: meteo.windVeloPeak
            }
        }

        Text {
            Layout.margins: 10
            Layout.alignment: Qt.AlignHCenter

            text: meteo.time
            font.pixelSize: 48
            color: "yellow"
        }
    }
}
