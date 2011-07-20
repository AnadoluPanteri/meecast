import Qt 4.7
import MeeGo.Components 0.1
import MeeGo.Panels 0.1
import MeeGo.Sharing 0.1
import MeeGo.Media 0.1

FlipPanel {
    id: container

    Translator {
        catalog: "meego-ux-panels-omweather"
    }

    front: SimplePanel {
        id: frontPanel
        panelTitle: qsTr("OMWeather")
        leftIconSource: "image://theme/panels/pnl_icn_photos"
        panelComponent:Flickable {
            anchors.fill: parent
            interactive: (height < contentHeight)
            onInteractiveChanged: {
                if (!interactive)
                    contentY = 0;
            }
            contentHeight: frontItemOmweather.height
            clip: true

            Item {
                id: frontItemOmweather
                //anchors.fill: parent
                width: parent.width
                height: toppanel.height

                /*
                Loader {
                    id: uiloader
                    anchors.fill: parent
                    function handleGoBack()
                    {
                        source = "";
                        columnlist.visible = true;
                    }
                    onItemChanged: {
                        if (item && item.goBack)
                            item.goBack.connect(handleGoBack);
                    }
                }
                */

                FrontPanelExpandableContent {
                    id: toppanel
                    anchors.top: parent.top
                    text: "omweather"
                    collapsible: false
                    contents: toppanelcont
                    visible: true
                }
                /*
                property int current_station: 0;

                Column {
                    id: columnlist
                    anchors.fill: parent

                    Rectangle {
                    }

                    ListView {
                        id: currentlist
                        model: currentxmlModel
                        delegate: itemDelegateCurrent
                        //anchors.top: parent.top
                        //anchors.left: parent.left
                        width: parent.width
                        height: parent.height * 0.4
                    }
                    GridView {
                        id: list
                        anchors.top: currentlist.bottom
                        cellWidth: parent.width / 3 -1
                        cellHeight: 200
                        model: xmlModel
                        delegate: itemDelegate
                        width: parent.width
                        height: parent.height * 0.55
                        //anchors.fill: parent
                    }
                }*/
                //height: topAppSection


            }
        }
    }
    Component {
        id: toppanelcont
        Item {
            width: parent.width
            Text {
                text: "11111111111111111111111"
            }
        }
    }

    Component {
        id: itemDelegate
        Item {
            id: day
            width: list.cellWidth
            //height: list.cellHeight


            //Text {text:  id_item}
            Column {
                Text {
                    id: day_name
                    text: dayname
                    color: "white"
                    font.pointSize: 14
                    //anchors.left: parent.left
                    //anchors.leftMargin: 20
                    //anchors.centerIn: parent
                    //anchors.top: parent.top
                }

                Image {
                    id: forecast_icon
                    source: icon
                    width: 120
                    height: 120
                    //anchors.top: day_name.bottom
                    //anchors.topMargin: 20
                    //anchors.leftMargin: 5
                    //anchors.left: parent.left
                }
                Text {
                    id: temp
                    text: (temperature) ? temperature : (temperature_low + " - " + temperature_high)
                    //anchors.top: forecast_icon.top; anchors.topMargin: 10
                    //anchors.left: forecast_icon.right; anchors.leftMargin: 10
                    color: "white"
                    font.pointSize: 14
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    uiloader.source = "Details.qml";
                    //list.visible = false;
                    columnlist.visible = false
                    uiloader.item.item_id = id_item;
                    //uiloader.item.width = frontItemOmweather.width
                    //uiloader.item.height = frontItemOmweather.height
                }
                hoverEnabled: true

            }

        }

    }

    Component {
        id: itemDelegateCurrent
        Item {
            anchors.fill: parent
            Row {
                anchors.fill: parent
                spacing: 20
                Column {
                    width: parent.width * 0.5
                    height: parent.height
                    spacing: 20
                    Image {
                        id: forecast_icon
                        source: icon
                        width: parent.width*0.5
                        height: parent.width*0.5
                    }
                    Text {
                        text: dayname
                        color: "white"
                        font.pointSize: 18
                    }
                }
                Column {
                    spacing: 10

                    Text {
                        font.pointSize: 36
                        color: "white"
                        text: (temperature) ? temperature : (temperature_low + " - " + temperature_high)
                    }
                    Text {
                        text: description
                        color: "white"
                        font.pointSize: 16
                    }
                    Text {
                        text: qsTr("Humidity") + ": " + humidity
                        color: "white"
                        font.pointSize: 16
                    }
                    Text {
                        text: qsTr("Wind") + ": " + wind_direction
                        color: "white"
                        font.pointSize: 16
                    }
                    Text {
                        text: qsTr("Speed") + ": " + wind_speed
                        color: "white"
                        font.pointSize: 16
                    }
                    /*
                    Text {text: (wind_gust != "N/A") ?
                          (qsTr("Wind gust") + ": " + wind_gust) : ""}

                    Text {text: (ppcp != "N/A") ?
                          (qsTr("Ppcp") + ": " + ppcp) : ""}
                    Text {text: (pressure != "N/A") ?
                          (qsTr("Pressure") + ": " + pressure) : ""}
                    */
                }
            }
        }
    }

    XmlListModel {
        id: stationModel
        source: "/home/meego/.config/com.meecast.omweather/qmldata.xml"
        query: "/data/station"

        XmlRole {name:  "id"; query: "@id/string()"}
        XmlRole {name:  "name"; query: "@name/string()"}

    }
    XmlListModel {
        id: currentxmlModel
        source: "/home/meego/.config/com.meecast.omweather/qmldata.xml"
        query: "/data/station[1]/item[@current='true']"

        XmlRole {name: "dayname"; query: "dayname/string()"}
        XmlRole {name: "temperature_low"; query: "temperature_low/string()"}
        XmlRole {name: "temperature_high"; query: "temperature_hi/string()"}
        XmlRole {name: "temperature"; query: "temperature/string()"}
        XmlRole {name: "icon"; query: "icon/string()"}
        XmlRole {name: "wind_speed"; query: "wind_speed/string()"}
        XmlRole {name: "wind_direction"; query: "wind_direction/string()"}
        XmlRole {name: "humidity"; query: "humidity/string()"}
        XmlRole {name: "description"; query: "description/string()"}
        XmlRole {name: "id_item"; query: "@id/number()"}
        //XmlRole {name: "current"; query:  "@current/boolean()"}
    }

    XmlListModel {
        id: xmlModel
        source: "/home/meego/.config/com.meecast.omweather/qmldata.xml"
        query: "/data/station[1]/item[not(@current)]"

        XmlRole {name: "dayname"; query: "dayname/string()"}
        XmlRole {name: "temperature_low"; query: "temperature_low/string()"}
        XmlRole {name: "temperature_high"; query: "temperature_hi/string()"}
        XmlRole {name: "temperature"; query: "temperature/string()"}
        XmlRole {name: "icon"; query: "icon/string()"}
        XmlRole {name: "id_item"; query: "@id/number()"}
    }

    back: BackPanelGeneric {
        id: backPanel
        panelTitle: qsTr("OMWeather back")
        bpContent: backPanelContent
        isBackPanel: true
        leftIconSource: "image://theme/panels/pnl_icn_photos"
    }
    Component {
        id: backPanelContent
        Text {
            anchors.fill: parent
            text: "back omweather"
        }
    }
}