import QtQuick 2.0

Item
{
    property string itemSource
    property double squareSize: 10
    property int cellNumber
    property bool setFocus: false
    property string itemDirection
    property bool itemVisible: true

    signal moveUp(string sDirection)
    signal moveDown(string sDirection)
    signal moveLeft(string sDirection)
    signal moveRight(string sDirection)
    signal makeShot(string sDirection)

    id: ic
    width: squareSize
    height: squareSize
    focus: setFocus

    Keys.onPressed:
    {
        if(event.key == Qt.Key_Up)
        {
            ic.moveUp("UP")
        }
        if(event.key == Qt.Key_Left)
        {
            ic.moveLeft("LEFT")
        }
        if(event.key == Qt.Key_Right)
        {
            ic.moveRight("RIGHT")
        }
        if(event.key == Qt.Key_Down)
        {
            ic.moveDown("DOWN")
        }
        if(event.key == Qt.Key_0)
            ic.makeShot("SHOT")
    }

    Rectangle
    {
        width: ic.width
        height: ic.height;
        border.color: "transparent"
        border.width: 0
        color: "transparent"
        //opacity: itemVisible

        Image
        {
            id: imageContainer
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            width: ic.width
            height: ic.height
            source: itemSource
            fillMode: Image.PreserveAspectFit
            visible: itemVisible
        }

        Text
        {
            anchors.centerIn: parent
            //text: cellNumber
            color: "yellow"
            font.pixelSize: 8
        }
    }
}
