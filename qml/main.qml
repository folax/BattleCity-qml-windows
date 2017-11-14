import CppStates 1.0
import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
//import com.tankobject 1.0

ApplicationWindow {
    id: root
    visible: true
    width: Screen.width / 3
    height: Screen.height / 1.7

    //var data
    property int cellsNumber: 2704
    property var cellArr: []
    property int player1TankLevel: 1
    //c++ ai array size
    property int aiArrSize: 0

    //scoreboard data
    property int player1LifeNumber: 2
    property int aiNumbers: 20
    property int gameLevel: 1
    property bool gameOverStatus: false

    property var respawnCoords:
        [
        0, 1, 2, 3, 52, 53, 54, 55, 104, 105, 106, 107, 156, 157, 158, 159,
        24, 25, 26, 27, 76, 77, 78, 79, 128, 129, 130, 131, 180, 181, 182, 183,
        48, 49, 50, 51, 100, 101, 102, 103, 152, 153, 154, 155, 204, 205, 206, 207,
        2512, 2513, 2514, 2515,
        2564, 2565, 2566, 2567,
        2616, 2617, 2618, 2619,
        2668, 2669, 2670, 2671,
        2528, 2529, 2530, 2531,
        2580, 2581, 2582, 2583,
        2632, 2633, 2634, 2635,
        2684, 2685, 2686, 2687
    ];

    property var brickwallCoords:
        [
        2414, 2415, 2416, 2417, 2418, 2419, 2420, 2421,
        2466, 2467, 2468, 2469, 2470, 2471, 2472, 2473,
        2518, 2519, 2524, 2525,
        2570, 2571, 2576, 2577,
        2622, 2623, 2628, 2629,
        2674, 2675, 2680, 2681
    ];

    property var baseCoords:
        [ 2520, 2521, 2522, 2523, 2572, 2573, 2574, 2575, 2624, 2625, 2626, 2627, 2676, 2677, 2678, 2679 ]


    property var tankCoords:
        [
        2512, 2513, 2514, 2515,
        2564, 2565, 2566, 2567,
        2616, 2617, 2618, 2619,
        2668, 2669, 2670, 2671
    ];

    property var imgSources:
        [
        "qrc:/images/img/brick.png",                    //0
        "qrc:/images/img/iron.png",                     //1
        "qrc:/images/img/forest.png",                   //2
        "qrc:/images/img/ice.png",                      //3
        "qrc:/images/img/river.png",                    //4
        "qrc:/images/img/empty.png",                    //5
        "qrc:/images/img/respawn.png",                  //6
        "qrc:/images/img/eagle.png",                    //7
        "qrc:/images/img/shell.png",                    //8
        "qrc:/images/img/tank-level-1.png",             //9
        "qrc:/images/img/tank-level-2.png",             //10
        "qrc:/images/img/tank-level-3.png",             //11
        "qrc:/images/img/tank-level-4.png",             //12
        "qrc:/images/img/ai-tank-level-1.png",          //13
        "qrc:/images/img/ai-tank-level-1-bonus.png",    //14
        "qrc:/images/img/ai-tank-level-2.png",          //15
        "qrc:/images/img/ai-tank-level-2-bonus.png",    //16
        "qrc:/images/img/ai-tank-level-3.png",           //17
        "qrc:/images/img/ai-tank-level-3-bonus.png"      //18
    ];

    Connections {
        target: cppObject
        onSigTankMoved: drawNewPlayerTankPosition(direction)
        onBonusCreateSignal: makeBonus(position, bonus)
        onBonusEraseSignal: makeBonus(-1, -1)
        onTankLevelSignal: player1TankLevel = level
        onGameOver: gameOverStatus = true

        //new variant
        onSigDrawShells: drawShells(sCoords)
        onSigDrawEmptyCell: drawEmptyCells(eCoords)
        onSigDrawAiTanksQml: drawAiTanksNew(dataAiCoordsDirectionLevelArr, dataAiArrSize, totalAiSize)
    }

    function drawAiTanksNew(dataAiCoordsDirectionLevelArr, dataAiArrSize, totalAiSize)
    {
        scoreBoardAiRepeater.model = (20 - totalAiSize)
        var aiDirectionsArr = []
        var aiCoodrsArr = []
        var aiTanksLevelsArr = []
        var counter = 0
        var tmpArr = []
        // Get data from c++, store it, then manipiulate; TIP: coords, direction, level;
        for(var i = 0; i <= dataAiCoordsDirectionLevelArr.length; i++)
        {
            if(counter == 18)
            {
                aiCoodrsArr.push(tmpArr[0])
                aiDirectionsArr.push(tmpArr[16])
                aiTanksLevelsArr.push(tmpArr[17])
                tmpArr = []
                counter = 0
            }
            tmpArr.push(dataAiCoordsDirectionLevelArr[i])
            counter++
        }

        // Manipulate with derived c++ data;
        var aiTankImageSource = ""
        aiTankRepeater.model = aiTanksLevelsArr.length
        for(var k = 0; k < totalAiSize; k++)
        {
            // Set tank image relatively by the level;
            if(aiTanksLevelsArr[k] === 1)
                aiTankImageSource = imgSources[13]
            else if(aiTanksLevelsArr[k] === 2)
                aiTankImageSource = imgSources[13]
            else if(aiTanksLevelsArr[k] === 3)
                aiTankImageSource = imgSources[17]

            // Set image direction;
            if(aiDirectionsArr[k] === 0)
                aiTankRepeater.itemAt(k).rotation = 0;
            else if(aiDirectionsArr[k] === 1)
                aiTankRepeater.itemAt(k).rotation = 180;
            else if(aiDirectionsArr[k] === 3)
                aiTankRepeater.itemAt(k).rotation = 270;
            else if(aiDirectionsArr[k] === 2)
                aiTankRepeater.itemAt(k).rotation = 90;

            // Set image overlap;
            aiTankRepeater.itemAt(k).source = aiTankImageSource
            aiTankRepeater.itemAt(k).x = cellsRepeater.itemAt(aiCoodrsArr[k]).x
            aiTankRepeater.itemAt(k).y = cellsRepeater.itemAt(aiCoodrsArr[k]).y
        }
    }

    function firstInit()
    {
        var dataArr  = cppObject.getData();
        for (var i = 0; i < cellsNumber; i++)
        {
            cellsRepeater.itemAt(i).itemSource = imgSources[dataArr[i]];
        }
        //base overlap
        baseImage.x = cellsRepeater.itemAt(2520).x
        baseImage.y = cellsRepeater.itemAt(2520).y
        //player1 tank overlap
        playerTank.x = cellsRepeater.itemAt(2512).x
        playerTank.y = cellsRepeater.itemAt(2512).y
    }

    function drawNewPlayerTankPosition(direction)
    {
        if(player1TankLevel == 1)
            playerTank.source = "qrc:/images/img/tank-level-1.png"
        else if(player1TankLevel == 2)
            playerTank.source = "qrc:/images/img/tank-level-2.png"
        else if(player1TankLevel == 3)
            playerTank.source = "qrc:/images/img/tank-level-3.png"
        else if(player1TankLevel == 4)
            playerTank.source = "qrc:/images/img/tank-level-4.png"

        //set new player tank coords;
        var stepToDelete;
        var tmpArr = cppObject.getTankCoords();
        for(var i = 0; i < tmpArr.length; i++)
        {
            //delete previous tank coords
            if(direction === 0)
                stepToDelete = 52;
            else if(direction === 1)
                stepToDelete = -52;
            else if(direction === 2)
                stepToDelete = -1;
            else if(direction === 3)
                stepToDelete = 1;
            //delete previous coord
            if(tmpArr[i] + stepToDelete > 0 && tmpArr[i] + stepToDelete < 2704)
            {
                if(cellsRepeater.itemAt(tmpArr[i] + stepToDelete).itemSource === imgSources[9])
                    cellsRepeater.itemAt(tmpArr[i] + stepToDelete).itemSource = imgSources[5]
            }
            //draw current tank coord
            cellsRepeater.itemAt(tmpArr[i]).itemSource = imgSources[9]
        }
        //overlap tank coords
        playerTank.x = cellsRepeater.itemAt(tmpArr[0]).x
        playerTank.y = cellsRepeater.itemAt(tmpArr[0]).y
        if(direction === 0)
            playerTankRotation.angle = 0;
        else if(direction === 1)
            playerTankRotation.angle = 180;
        else if(direction === 3)
            playerTankRotation.angle = 270;
        else if(direction === 2)
            playerTankRotation.angle = 90;
    }

    function makeBonus(position, bonus)
    {
        if(position == -1)
            bonusImage.visible = false
        else{
            bonusImage.x = cellsRepeater.itemAt(position).x
            bonusImage.y = cellsRepeater.itemAt(position).y
            if(bonus == 0)
                bonusImage.source = "qrc:/images/img/bonus-bomb.png"
            else if(bonus == 1)
                bonusImage.source = "qrc:/images/img/bonus-life.png"
            else if(bonus == 2)
                bonusImage.source = "qrc:/images/img/bonus-shovel.png"
            else if(bonus == 3)
                bonusImage.source = "qrc:/images/img/bonus-star.png"
            else if(bonus == 4)
                bonusImage.source = "qrc:/images/img/bonus-time.png"
        }
    }

    function drawEmptyCells(cCoords)
    {
        for(var i = 0; i < cCoords.length; i++)
            cellsRepeater.itemAt(cCoords[i]).itemSource = imgSources[5]
    }

    function drawShells(sCoords)
    {
        for(var i = 0; i < sCoords.length; i++)
            cellsRepeater.itemAt(sCoords[i]).itemSource = imgSources[8]
        //gameDebug()
    }

    function gameDebug()
    {
        var dataArr = cppObject.getData();
        for(var i = 0; i < dataArr.length; i++)
        {
            cellsRepeater.itemAt(i).itemSource = imgSources[dataArr[i]]
        }
    }

    Rectangle
    {
        id: appBackgroud
        anchors.fill: parent
        gradient: Gradient
        {
            GradientStop { position: 0.0; color: "#666666" }
            GradientStop { position: 1.0; color: "#262626" }
        }

        Grid
        {
            id: dropGrid
            rows: 52
            columns: 52
            anchors.centerIn: parent

            Repeater
            {
                id: cellsRepeater
                model: cellsNumber

                ImageContainer
                {
                    itemSource: imgSources[5]
                    cellNumber: index
                    onMoveUp:
                    {
                        if(!gameOverStatus)
                        {
                            cppObject.movePlayerTank(0);
                        }
                    }
                    onMoveDown:
                    {
                        if(!gameOverStatus)
                        {
                            cppObject.movePlayerTank(1);
                        }
                    }
                    onMoveRight:
                    {
                        if(!gameOverStatus)
                        {
                            cppObject.movePlayerTank(2);
                        }
                    }
                    onMoveLeft:
                    {
                        if(!gameOverStatus)
                        {
                            cppObject.movePlayerTank(3);
                        }
                    }
                    onMakeShot:
                    {
                        if(!gameOverStatus)
                        {
                            cppObject.makeShot();
                        }
                    }
                }
            }

            Component.onCompleted:
            {
                for (var i = 0; i < tankCoords.length; i++)
                {
                    cellsRepeater.itemAt(tankCoords[i]).itemSource = imgSources[9]
                    cellsRepeater.itemAt(tankCoords[i]).setFocus = true;
                }
                firstInit();
                cppObject.makeBonus();
            }

            Image
            {
                id: playerTank
                visible: true
                source: "qrc:/images/img/tank-level-1.png"
                width: 40
                height: 40
                transform: Rotation
                {
                    id: playerTankRotation
                    origin.x: 20
                    origin.y: 20
                    angle: 0
                }
            }

            Image
            {
                id: baseImage
                visible: true
                source: "qrc:/images/img/eagle.png"
                width: 40
                height: 40
            }

            Image
            {
                id: bonusImage
                source: "qrc:/images/img/bonus-life.png"
                width: 40
                height: 40
            }

            //draw ai tanks


            Repeater {
                id: aiTankRepeater
                model: aiArrSize
                Image
                {
                    source: imgSources[5]
                    width: 40
                    height: 40
                }
            }

        }

        Item
        {
            id: scoreBoard
            anchors.left: dropGrid.right
            anchors.top: dropGrid.top
            anchors.leftMargin: 20

            Text
            {
                id: scoreBoardAiText
                text: "AI"
                color: "white"
                font.pointSize: 12
                font.weight: Font.Bold

                Grid
                {
                    id: scoreBoardAiGrid
                    rows: 10
                    columns: 2
                    anchors.horizontalCenter: scoreBoardAiText.horizontalCenter
                    anchors.top: scoreBoardAiText.bottom
                    height: 200

                    Repeater
                    {
                        id: scoreBoardAiRepeater
                        model: aiNumbers

                        Image
                        {
                            source: "qrc:/images/img/scoreboard-ai.png"
                            width: 20
                            height: 20
                        }
                    }
                }

                Text
                {
                    id: scoreBoardPlayerText
                    text: "Lives\n" + player1LifeNumber
                    color: "white"
                    font.pointSize: 10
                    font.weight: Font.Bold
                    anchors.topMargin: 20
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: scoreBoardAiGrid.horizontalCenter
                    anchors.top: scoreBoardAiGrid.bottom
                }

                Text
                {
                    id: scoreBoardLevelText
                    text: "Level\n" + gameLevel
                    color: "white"
                    font.pointSize: 10
                    font.weight: Font.Bold
                    anchors.topMargin: 20
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: scoreBoardPlayerText.horizontalCenter
                    anchors.top: scoreBoardPlayerText.bottom
                }
            }
        }
        Text
        {
            text: "GAME OVER"
            color: "red"
            anchors.horizontalCenter: appBackgroud.horizontalCenter
            anchors.verticalCenter: appBackgroud.verticalCenter
            font.pointSize: 50
            font.weight: Font.Bold
            style: Text.Outline; styleColor: "white"
            visible: gameOverStatus
        }

    }
}
