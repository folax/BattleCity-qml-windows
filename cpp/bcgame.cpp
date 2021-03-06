#include "bcgame.h"
#include <algorithm>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QPair>
#include <QTimer>
#include <QTime>


bcGame::bcGame()
{
    this->initGame();
}

QStringList bcGame::getData()
{
    QStringList sl;
    for(int i(0); i < areaSize; ++i)
    {
        for(int j(0); j < areaSize; ++j)
        {
            sl.append(QString::number(m_statesArr[i][j]));
        };
    }
    return sl;
}

void bcGame::importDataFromQML(const QVariantList &arr)
{
    if(!arr.isEmpty())
    {
        int cnt = 0;
        for(int i(0); i < areaSize; ++i)
        {
            for(int j(0); j < areaSize; ++j)
            {
                m_statesArr[i][j] = arr.at(cnt).toInt();
                cnt++;
            }
        }
    }
}

void bcGame::movePlayerTank(const int direction)
{
    QVector<int> arr(m_pPlayer1Tank->getTankCoords());
    int cnt = arr.size() - 1, loop = 0;

    //get left and right border coords
    QVector<int> leftBorder, rightBorder, bottomBorder, topBorder;
    for(int i(0); i < areaSize; ++i)
    {
        for(int j(0); j < areaSize; ++j)
        {
            if(j == 0)
                leftBorder.push_back(loop);
            if(j == areaSize - 1)
                rightBorder.push_back(loop);
            if(i == areaSize - 1)
                bottomBorder.append(loop);
            if(i == 0)
                topBorder.append(loop);
            ++loop;
        }
    }
    if(isHaveBarrier(m_pPlayer1Tank->getTankCoords(), direction))
    {
        cnt = 15;
        for(int i(0); i < arr.size(); ++i)
        {
            if(direction == Up)
            {
                if(!topBorder.contains(arr.at(0)))
                {
                    setStateByPos(arr.at(i), Empty);
                    setStateByPos(arr.at(i) - 52, Tank);
                }
            }
            else if(direction == Left) //left
            {
                if(!leftBorder.contains(arr.at(0)))
                {
                    setStateByPos(arr.at(i), Empty);
                    setStateByPos(arr.at(i) - 1, Tank);
                }
            }

            //reverse selection
            if(direction == Down)
            {
                if(!bottomBorder.contains(arr.at(12)))
                {
                    setStateByPos(arr.at(cnt), Empty);
                    setStateByPos(arr.at(cnt) + 52, Tank);
                    --cnt;
                }
            }
            else if(direction == Right) //right
            {
                if(!rightBorder.contains(arr.at(3)))
                {
                    setStateByPos(arr.at(cnt), Empty);
                    setStateByPos(arr.at(cnt) + 1, Tank);
                    --cnt;
                }
            }
        }
    }
    //using tank class for future manipulation
    m_pPlayer1Tank->setDirection(direction);
    m_pPlayer1Tank->setCoords(&m_statesArr[0][0]);
    m_pPlayer1Tank->setSpeed(1);

    //check for bonuses
    if(m_sBonusData.active)
    {
        for(int i(0); i < m_sBonusData.coords.size(); ++i)
            if(m_pPlayer1Tank->getTankCoords().contains(m_sBonusData.coords.at(i)))
            {
                if(m_sBonusData.bonus == eBomb)
                    qDebug() << "eBomb";
                else if(m_sBonusData.bonus == eLife)
                {
                    m_pPlayer1Tank->addLife();
                    qDebug() << "eLife";
                }
                else if(m_sBonusData.bonus == eShovel)
                    qDebug() << "eShovel";
                else if(m_sBonusData.bonus == eStar)
                {
                    m_pPlayer1Tank->levelUp();
                    qDebug() << "eStar";
                }
                else if(m_sBonusData.bonus == eTime)
                    qDebug() << "eTime";

                m_sBonusData.active = false;
                emit bonusEraseSignal();
            }
    }
    emit sigTankMoved(m_pPlayer1Tank->getDirection());
}

void bcGame::makeShot()
{
    // Make new version of makeshot;
    int playerShellNumber = 0;
    if(m_pPlayer1Tank->getTankLevel() == 1 || m_pPlayer1Tank->getTankLevel() == 2)
        playerShellNumber = 1;
    else
        playerShellNumber = 2;

    if(m_pPlayer1Tank->getShellsSize() != playerShellNumber)
    {
        aiMakeShot(m_pPlayer1Tank->convertToStruct(-1));
    }
}

int bcGame::getStateByPos(const int pos)
{
    int cnt = 0;
    for(int i(0); i < areaSize; ++i)
    {
        for(int j(0); j < areaSize; ++j)
        {
            if(cnt == pos)
            {
                return m_statesArr[i][j];
            }
            ++cnt;
        }
    }
    return -1;
}

void bcGame::initGame()
{
    //timer initialization
    m_Player1ShellSpeedTimer = new QTimer(this);
    connect(m_Player1ShellSpeedTimer, &QTimer::timeout, this, &bcGame::slotTimerTimeOut);
    m_bonusTimer = new QTimer(this);
    connect(m_bonusTimer, &QTimer::timeout, this, &bcGame::eraseBonus);

   m_gameLevel = 1;

    //add path for future levels
    m_levelsPath.append(QApplication::applicationDirPath() + "/level-1.bce");

    if(!m_levelsPath.isEmpty())
    {
        //only for demp purposes
        QFile file(":/demoGameMap");
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString data(file.readAll());
        file.close();
        QVector<int> buffer;
        //simple file validation check
        if(data.contains("BCLevel-v.01"))
        {
            QJsonDocument doc(QJsonDocument::fromJson(data.toUtf8()));
            QJsonObject obj = doc.object();
            QJsonArray array = obj["CoordsData"].toArray();
            for(int i(0); i < array.size(); ++i)
                buffer.append(array.at(i).toInt());
        }
        else

        {
            qDebug() << "PAth: "<<  QDir::homePath();
            QMessageBox::warning(0, "Error", "Invalid file structure, please copy content of level-1.bce file from demo-map folder to demoGameMap resource");
        }
        int cnt = 0;
        for(int i(0); i < areaSize; ++i)
        {
            for(int j(0); j < areaSize; ++j)
            {
                m_statesArr[i][j] = buffer.at(cnt);
                ++cnt;
            }
        }
    }
    removeRespawnCells();

    // Player 1 tank initialization;
    m_pPlayer1Tank = new PlayerTankObj;

    // Ai initialization after map loading;
    m_respawnAiTanksTimer = new QTimer(this);
    connect(m_respawnAiTanksTimer, &QTimer::timeout, this, &bcGame::addAiTankToArr);
    // Set first ai tank respawn time;
    m_respawnAiTanksTimer->start(1500);

    //timers for ai tanks move
    m_pAiMoveNormalTimer = new QTimer(this);
    m_pAiMoveFastTimer = new QTimer(this);
    connect(m_pAiMoveNormalTimer, &QTimer::timeout,this, &bcGame::moveAiTank);
    //connect(m_pAiMoveFastTimer, &QTimer::timeout,this, &bcGame::moveAiTank);
    m_pAiMoveNormalTimer->start(300);
    //m_pAiMoveFastTimer->start(500);

    m_pShellMove = new QTimer(this);
    connect(m_pShellMove, &QTimer::timeout, this, &bcGame::moveAiShells);
    m_pShellMove->start(100);

    //random init
    qsrand(QTime::currentTime().msec());
}

void bcGame::setStateByPos(const int pos, const int state)
{
    int cnt = 0;
    for(int i(0); i < areaSize; ++i)
    {
        for(int j(0); j < areaSize; ++j)
        {
            if(cnt == pos)
                m_statesArr[i][j] = state;
            ++cnt;
        }
    }
}

bool bcGame::isHaveBarrier(const QVector<int> &arr, int direction)
{
    if(direction == Up)
    {
        if(getStateByPos(arr.at(0) - 52) == Empty
                && getStateByPos(arr.at(1) - 52) == Empty
                && getStateByPos(arr.at(2) - 52) == Empty
                && getStateByPos(arr.at(3) - 52) == Empty)
            return true;
    }
    else if(direction == Down)
    {
        if(getStateByPos(arr.at(12) + 52) == Empty
                && getStateByPos(arr.at(13) + 52) == Empty
                && getStateByPos(arr.at(14) + 52) == Empty
                && getStateByPos(arr.at(15) + 52) == Empty
                )
            return true;
    }
    else if(direction == Right)
    {
        if(getStateByPos(arr.at(3) + 1) == Empty
                && getStateByPos(arr.at(7) + 1) == Empty
                && getStateByPos(arr.at(11) + 1) == Empty
                && getStateByPos(arr.at(15) + 1) == Empty)
            return true;
    }
    else if(direction == Left)
    {
        if(getStateByPos(arr.at(0) - 1) == Empty
                && getStateByPos(arr.at(4) - 1) == Empty
                && getStateByPos(arr.at(8) - 1) == Empty
                && getStateByPos(arr.at(12) - 1) == Empty)
            return true;
    }
    return false;
}

bool bcGame::isAiHaveBarrier(const QVector<int> aiCoords, const int aiDirection)
{
    QVector<int> leftBorder = { 0, 52, 104, 156, 208, 260, 312, 364, 416, 468,
                                520, 572, 624, 676, 728, 780, 832, 884, 936, 988,
                                1040, 1092, 1144, 1196, 1248, 1300, 1352, 1404, 1456, 1508,
                                1560, 1612, 1664, 1716, 1768, 1820, 1872, 1924, 1976, 2028,
                                2080, 2132, 2184, 2236, 2288, 2340, 2392, 2444, 2496, 2548,
                                2600, 2652 };
    QVector<int> rightBorder = { 51, 103, 155, 207, 259, 311, 363, 415, 467, 519,
                                 571, 623, 675, 727, 779, 831, 883, 935, 987, 1039,
                                 1091, 1143, 1195, 1247, 1299, 1351, 1403, 1455, 1507, 1559,
                                 1611, 1663, 1715, 1767, 1819, 1871, 1923, 1975, 2027, 2079,
                                 2131, 2183, 2235, 2287, 2339, 2391, 2443, 2495, 2547, 2599,
                                 2651, 2703 };
    QVector<int> topBorder = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                               20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                               30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                               40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
                               50, 51 };
    QVector<int> bottomBorder = { 2652, 2653, 2654, 2655, 2656, 2657, 2658, 2659, 2660, 2661,
                                  2662, 2663, 2664, 2665, 2666, 2667, 2668, 2669, 2670, 2671,
                                  2672, 2673, 2674, 2675, 2676, 2677, 2678, 2679, 2680, 2681,
                                  2682, 2683, 2684, 2685, 2686, 2687, 2688, 2689, 2690, 2691,
                                  2692, 2693, 2694, 2695, 2696, 2697, 2698, 2699, 2700, 2701,
                                  2702, 2703 };

    if(leftBorder.contains(aiCoords.at(0)) && aiDirection == Left)
        return false;
    else if(rightBorder.contains(aiCoords.at(3)) && aiDirection == Right)
        return false;
    else if(topBorder.contains(aiCoords.at(0)) && aiDirection == Up)
        return false;
    else if(bottomBorder.contains(aiCoords.at(12)) && aiDirection == Down)
        return false;

    //clear ahead direction;
    if(aiDirection == Up)
    {
        if(getStateByPos(aiCoords.at(0) - 52) == Empty
                && getStateByPos(aiCoords.at(1) - 52) == Empty
                && getStateByPos(aiCoords.at(2) - 52) == Empty
                && getStateByPos(aiCoords.at(3) - 52) == Empty)
            return true;
    }
    else if(aiDirection == Down)
    {
        if(getStateByPos(aiCoords.at(12) + 52) == Empty
                && getStateByPos(aiCoords.at(13) + 52) == Empty
                && getStateByPos(aiCoords.at(14) + 52) == Empty
                && getStateByPos(aiCoords.at(15) + 52) == Empty
                )
            return true;
    }
    else if(aiDirection == Right)
    {
        if(getStateByPos(aiCoords.at(3) + 1) == Empty
                && getStateByPos(aiCoords.at(7) + 1) == Empty
                && getStateByPos(aiCoords.at(11) + 1) == Empty
                && getStateByPos(aiCoords.at(15) + 1) == Empty)
            return true;
    }
    else if(aiDirection == Left)
    {
        if(getStateByPos(aiCoords.at(0) - 1) == Empty
                && getStateByPos(aiCoords.at(4) - 1) == Empty
                && getStateByPos(aiCoords.at(8) - 1) == Empty
                && getStateByPos(aiCoords.at(12) - 1) == Empty)
            return true;
    }
    //direction with own shell
    if(aiDirection == Up)
    {
        if(getStateByPos(aiCoords.at(0) - 52) == Empty
                && getStateByPos(aiCoords.at(1) - 52) == Shell
                && getStateByPos(aiCoords.at(2) - 52) == Shell
                && getStateByPos(aiCoords.at(3) - 52) == Empty)
            return true;
    }
    else if(aiDirection == Down)
    {
        if(getStateByPos(aiCoords.at(12) + 52) == Empty
                && getStateByPos(aiCoords.at(13) + 52) == Shell
                && getStateByPos(aiCoords.at(14) + 52) == Shell
                && getStateByPos(aiCoords.at(15) + 52) == Empty
                )
            return true;
    }
    else if(aiDirection == Right)
    {
        if(getStateByPos(aiCoords.at(3) + 1) == Empty
                && getStateByPos(aiCoords.at(7) + 1) == Shell
                && getStateByPos(aiCoords.at(11) + 1) == Shell
                && getStateByPos(aiCoords.at(15) + 1) == Empty)
            return true;
    }
    else if(aiDirection == Left)
    {
        if(getStateByPos(aiCoords.at(0) - 1) == Empty
                && getStateByPos(aiCoords.at(4) - 1) == Shell
                && getStateByPos(aiCoords.at(8) - 1) == Shell
                && getStateByPos(aiCoords.at(12) - 1) == Empty)
            return true;
    }
    return false;
}

bool bcGame::isShellOutOfBorder(const QVector<TankShell> &arr, const int pos)
{
    if(!arr.isEmpty())
    {
        QVector<int> border;
        int loop(0);
        if(arr.at(pos).direction == Up || arr.at(pos).direction == Down)
        {
            if(arr.at(pos).position_1 < 0 || arr.at(pos).position_1 > (areaSize * areaSize))
            {
                return true;
            }
        }
        else if(arr[pos].direction == Left)
        {
            for(int i(0); i < areaSize; ++i)
            {
                for(int j(0); j < areaSize; ++j)
                {
                    if(j == 0)
                        border.push_back(loop);
                    ++loop;
                }
            }
            if(border.contains(arr.at(pos).position_1))
            {
                return true;
            }
        }
        else if(arr[pos].direction == Right)
        {
            for(int i(0); i < areaSize; ++i)
            {
                for(int j(0); j < areaSize; ++j)
                {
                    if(j == areaSize - 1)
                        border.push_back(loop);
                    ++loop;
                }
            }
            if(border.contains(arr.at(pos).position_1))
            {
                return true;
            }
        }
    }
    return false;
}

int bcGame::whereIsTank(const AiTankStruct &aiTankData)
{
    //Return values:
    //top left corner = 0 | top right corner = 1,
    //bottom left corner = 2 | bottom right corner = 3,
    //------------------------------------------------
    //top border = 4 | right border = 5,
    //bottom border = 6 | left border = 7,
    //no border = -1;

    QVector<int> leftBorder, rightBorder, bottomBorder, topBorder;
    QVector<int> cornerCoords = { 0, 51, 2652, 2703 };
    int loop = 0;
    for(int i(0); i < areaSize; ++i)
    {
        for(int j(0); j < areaSize; ++j)
        {
            if(j == 0)
            {
                //remove corner coords;
                if(!cornerCoords.contains(loop))
                    leftBorder.push_back(loop);
            }
            if(j == areaSize - 1)
            {
                if(!cornerCoords.contains(loop))
                    rightBorder.push_back(loop);
            }
            if(i == areaSize - 1)
            {
                if(!cornerCoords.contains(loop))
                    bottomBorder.append(loop);
            }
            if(i == 0)
            {
                if(!cornerCoords.contains(loop))
                    topBorder.append(loop);
            }
            ++loop;
        }
    }
    //check corner coords;
    if(aiTankData.coords.contains(cornerCoords.at(0)))
        return 0;
    if(aiTankData.coords.contains(cornerCoords.at(1)))
        return 1;
    if(aiTankData.coords.contains(cornerCoords.at(2)))
        return 2;
    if(aiTankData.coords.contains(cornerCoords.at(3)))
        return 3;
    //check border coords;
    QVector<int> tankPosSign;
    if(aiTankData.direction == Up)
        tankPosSign = { 0, 1, 2, 3 };
    if(aiTankData.direction == Right)
        tankPosSign = { 3, 7, 11, 15 };
    if(aiTankData.direction == Down)
        tankPosSign = { 12, 13, 14, 15 };
    if(aiTankData.direction == Left)
        tankPosSign = { 0, 4, 8, 12 };

    if(topBorder.contains(tankPosSign.at(0)) && topBorder.contains(tankPosSign.at(1)) && topBorder.contains(tankPosSign.at(2)) && topBorder.contains(tankPosSign.at(3)))
        return 4;
    if(rightBorder.contains(tankPosSign.at(0)) && rightBorder.contains(tankPosSign.at(1)) && rightBorder.contains(tankPosSign.at(2)) && rightBorder.contains(tankPosSign.at(3)))
        return 5;
    if(bottomBorder.contains(tankPosSign.at(0)) && bottomBorder.contains(tankPosSign.at(1)) && bottomBorder.contains(tankPosSign.at(2)) && bottomBorder.contains(tankPosSign.at(3)))
        return 6;
    if(leftBorder.contains(tankPosSign.at(0)) && leftBorder.contains(tankPosSign.at(1)) && leftBorder.contains(tankPosSign.at(2)) && leftBorder.contains(tankPosSign.at(3)))
        return 7;

    return -1;
}

bool bcGame::isShellOnBorder(const TankShell &tankShell)
{
    QVector<int> leftBorder, rightBorder, bottomBorder, topBorder;
    int loop = 0;
    for(int i(0); i < areaSize; ++i)
    {
        for(int j(0); j < areaSize; ++j)
        {
            if(j == 0)
                leftBorder.push_back(loop);
            if(j == areaSize - 1)
                rightBorder.push_back(loop);
            if(i == areaSize - 1)
                bottomBorder.append(loop);
            if(i == 0)
                topBorder.append(loop);
            ++loop;
        }
    }
    if(tankShell.direction == Up)
    {
        if(topBorder.contains(tankShell.position_1))
        {
            return true;
        }
    }
    else if(tankShell.direction == Down)
    {
        if(bottomBorder.contains(tankShell.position_1))
        {
            return true;
        }
    }
    else if(tankShell.direction == Left)
    {
        if(leftBorder.contains(tankShell.position_1))
        {
            return true;
        }
    }
    else if(tankShell.direction == Right)
    {
        if(rightBorder.contains(tankShell.position_1))
        {
            return true;
        }
    }
    return false;
}

void bcGame::makeBonus()
{
    //функция для генерирования бонусов !нужно сделать проверку чтобы бонус не попадал в броню
    int loop = 0;
    QVector<int> cells, border;
    for(int i(0); i < areaSize; ++i)
    {
        for(int j(0); j < areaSize; ++j)
        {
            cells.push_back(loop);
            if(j == 0 || j == 1 || j == 2)
                border.push_back(loop);
            if(j == areaSize - 1 || j == areaSize - 2 || j == areaSize - 3)
                border.push_back(loop);
            if(i == areaSize - 1 || i == areaSize - 2 || i == areaSize - 3 || i == areaSize - 4)
                border.append(loop);
            if(i == 0 || i == 1 || i == 2)
                border.append(loop);
            ++loop;
        }
    }
    for(int i(0); i < border.size(); ++i)
    {
        for(int j(0); j < cells.size(); ++j)
        {
            if(cells.at(j) == border.at(i))
                cells.removeAt(j);
        }
    }
    //set bonus coordinates
    m_sBonusData.coords.clear();
    m_sBonusData.coords.push_back(cells.at(rand() % cells.size() + 0));
    m_sBonusData.coords.push_back(m_sBonusData.coords.at(0) + 1);
    m_sBonusData.coords.push_back(m_sBonusData.coords.at(0) + 25);
    m_sBonusData.coords.push_back(m_sBonusData.coords.at(2) + 1);
    //set bonus bonus
    int bonus = rand() % 4 + 0;
    if(bonus == 0)
        m_sBonusData.bonus = eBomb;
    else if(bonus == 1)
        m_sBonusData.bonus = eLife;
    else if(bonus == 2)
        m_sBonusData.bonus = eShovel;
    else if(bonus == 3)
        m_sBonusData.bonus = eStar;
    else if(bonus == 4)
        m_sBonusData.bonus = eTime;
    //set enabled
    m_sBonusData.active = true;
    //transmit data to qml
    emit bonusCreateSignal(m_sBonusData.coords.at(0), m_sBonusData.bonus);
    //bonus enabled only 20 seconds
    if(!m_bonusTimer->isActive())
        m_bonusTimer->start(20000);
}

QVector<int> bcGame::getTankCoords()
{
    return m_pPlayer1Tank->getTankCoords();
}

void bcGame::addAiTankToArr()
{
    QVector<int> sigDataToQmlTransfer;
    // Set normal default time of ai tanks respawn timer;
    m_respawnAiTanksTimer->setInterval(5000);
    //set ai levels in depends of level; 1 = normal tank, 2 = apc, 3 = heavy tank;
    //random ai tank level;
    if(m_totalCurrentAiSize == 20)
    {
        m_respawnAiTanksTimer->stop();
    }
    else
    {
        //create new struct with base values
        AiTankStruct tankStruct;
        tankStruct.level = rand() % 3 + 1;
        tankStruct.direction = Down;
        tankStruct.shell = 0;

        static uint positionsCounter = 0;
        if(positionsCounter == 3)
            positionsCounter = 0;

        QVector<int> coordsArr;
        if(positionsCounter == 0)
        {
            coordsArr  =  { 24, 25, 26, 27,
                            76, 77, 78, 79,
                            128, 129, 130, 131,
                            180, 181, 182, 183
                          };
        }
        else if(positionsCounter == 1)
        {
            coordsArr  = { 48, 49, 50, 51,
                           100, 101, 102, 103,
                           152, 153, 154, 155,
                           204, 205, 206, 207
                         };
        }
        else if(positionsCounter == 2)
        {
            coordsArr  = { 0, 1, 2, 3,
                           52, 53, 54, 55,
                           104, 105, 106, 107,
                           156, 157, 158, 159
                         };
        }
        //check for empty respawn spot
        if(checkForEmptyRespawn(coordsArr))
        {
            tankStruct.coords = coordsArr;
            //set cells state
            for(int i(0); i < coordsArr.size(); ++i)
                setStateByPos(coordsArr.at(i), AiTank);
            ++positionsCounter;
            //set unique id for ai tank structure;
            tankStruct.id = m_totalCurrentAiSize;
            m_AiTanksArray.append(tankStruct);
            ++m_totalCurrentAiSize;
            sigDataToQmlTransfer.append(tankStruct.coords);
            sigDataToQmlTransfer.append(tankStruct.direction);
            sigDataToQmlTransfer.append(tankStruct.level);
        }
        if(!sigDataToQmlTransfer.isEmpty())
            emit sigDrawAiTanksQml(sigDataToQmlTransfer, m_AiTanksArray.size(), this->m_totalCurrentAiSize);
    }
}

void bcGame::moveAiTank()
{
    QVector<int> sigDataArray;
    if(!m_AiTanksArray.isEmpty())
    {
        //generate SIZE of AI array signal to qml
        emit sigSizeOfAiArray(m_AiTanksArray.size());

        //calculate variants block
        for(int i(0); i < m_AiTanksArray.size(); ++i)
        {
            QVector<int> availableDirection;
            availableDirection.append(m_AiTanksArray[i].direction);
            if(!isAiHaveBarrier(m_AiTanksArray.at(i).coords, m_AiTanksArray.at(i).direction))
            {
                availableDirection.clear();
                if(isAiHaveBarrier(m_AiTanksArray.at(i).coords, Up))
                    availableDirection.append(Up);
                if(isAiHaveBarrier(m_AiTanksArray.at(i).coords, Down))
                    availableDirection.append(Down);
                if(isAiHaveBarrier(m_AiTanksArray.at(i).coords, Left))
                    availableDirection.append(Left);
                if(isAiHaveBarrier(m_AiTanksArray.at(i).coords, Right))
                    availableDirection.append(Right);
            }
            //qDebug() << "Directions: " << availableDirection;
            if(availableDirection.size() >= 1)
                m_AiTanksArray[i].direction = availableDirection[qrand() % (availableDirection.size()) + 0];

            //move block
            if(!availableDirection.isEmpty())
            {
                int cnt = 15;
                for(int j(0); j < m_AiTanksArray.at(i).coords.size(); ++j)
                {
                    if(m_AiTanksArray.at(i).direction == Up)
                    {
                        //qDebug() << "Move Up";
                        setStateByPos((m_AiTanksArray.at(i).coords[j]), Empty);
                        setStateByPos(m_AiTanksArray.at(i).coords[j] - 52, AiTank);
                        m_AiTanksArray[i].coords[j] -= 52;
                    }
                    else if(m_AiTanksArray.at(i).direction == Left) //left
                    {
                        //qDebug() << "Move left";
                        setStateByPos(m_AiTanksArray[i].coords[j], Empty);
                        setStateByPos(m_AiTanksArray[i].coords[j] - 1, AiTank);
                        m_AiTanksArray[i].coords[j] -= 1;
                    }

                    //reverse selection
                    else if(m_AiTanksArray.at(i).direction == Down)
                    {
                        //qDebug() << "Move Down";
                        setStateByPos(m_AiTanksArray[i].coords[cnt], Empty);
                        setStateByPos(m_AiTanksArray[i].coords[cnt] + 52, AiTank);
                        m_AiTanksArray[i].coords[cnt] += 52;
                        --cnt;
                    }
                    else if(m_AiTanksArray.at(i).direction == Right) //right
                    {
                        //qDebug() << "Move Right";
                        setStateByPos(m_AiTanksArray[i].coords[cnt], Empty);
                        setStateByPos(m_AiTanksArray[i].coords[cnt] + 1, AiTank);
                        m_AiTanksArray[i].coords[cnt] += 1;
                        --cnt;
                    }
                }
            }
            //using tank class for future manipulation
            sigDataArray.append(m_AiTanksArray.at(i).coords);
            sigDataArray.append(m_AiTanksArray.at(i).direction);
            sigDataArray.append(m_AiTanksArray.at(i).level);
            this->aiMakeShot(m_AiTanksArray[i]);
        }
    }
    //emit sigDrawAiTanks(sigDataArray);
    emit sigDrawAiTanksQml(sigDataArray, m_AiTanksArray.size(), this->m_totalCurrentAiSize);
}

bool bcGame::checkForEmptyRespawn(const QVector<int> coords)
{
    for(int i(0); i < coords.size(); ++i)
    {
        if(this->getStateByPos(coords.at(i)) != Empty)
        {
            qDebug() << "State: " << coords.at(i);
            return false;
        }
    }
    return true;
}

void bcGame::removeRespawnCells()
{
    for(int i(0); i < areaSize; ++i)
    {
        for(int j(0); j < areaSize; ++j)
        {
            //delete system mark(respawn) from map
            if(m_statesArr[i][j] == Respawn)
                m_statesArr[i][j] = Empty;
        };
    }
    //my respawn-1 place
    this->setStateByPos(2512, Tank);
    this->setStateByPos(2513, Tank);
    this->setStateByPos(2514, Tank);
    this->setStateByPos(2515, Tank);
    this->setStateByPos(2564, Tank);
    this->setStateByPos(2565, Tank);
    this->setStateByPos(2566, Tank);
    this->setStateByPos(2567, Tank);
    this->setStateByPos(2616, Tank);
    this->setStateByPos(2617, Tank);
    this->setStateByPos(2618, Tank);
    this->setStateByPos(2619, Tank);
    this->setStateByPos(2668, Tank);
    this->setStateByPos(2669, Tank);
    this->setStateByPos(2670, Tank);
    this->setStateByPos(2671, Tank);
    //my respawn-2 place
    //    this->setStateByPos(2528, Tank);
    //    this->setStateByPos(2529, Tank);
    //    this->setStateByPos(2530, Tank);
    //    this->setStateByPos(2531, Tank);
    //    this->setStateByPos(2580, Tank);
    //    this->setStateByPos(2581, Tank);
    //    this->setStateByPos(2582, Tank);
    //    this->setStateByPos(2583, Tank);
    //    this->setStateByPos(2632, Tank);
    //    this->setStateByPos(2633, Tank);
    //    this->setStateByPos(2634, Tank);
    //    this->setStateByPos(2635, Tank);
    //    this->setStateByPos(2684, Tank);
    //    this->setStateByPos(2685, Tank);
    //    this->setStateByPos(2686, Tank);
    //    this->setStateByPos(2687, Tank);
}

int bcGame::randDirection(const int limit)
{
    return qrand() % (limit + 1) + 1;
}

void bcGame::aiMakeShot(const AiTankStruct &aiTankData)
{
    // Tip: UP = step_1 = -1, step_2 = 1, fullstep = -52;
    // LEFT = step_1 = -52, step_2 = 52, fullstep = -1;
    // DOWN = step_1 = -1, step_2 = 1, fullstep = 52;
    // RIGHT = step_1 = -52, step_2 = 52, fullstep = 1;
    if(aiTankData.shell == 0)
    {
        //if tank on border or on the corner;
        int tankPosSign = whereIsTank(aiTankData);
        if(tankPosSign > 0)
        {
            //-----------------top left corner;
            if(tankPosSign == 0)
            {
                if(aiTankData.direction == Right)
                {
                    cornerMoveRight(aiTankData);
                }
                else if(aiTankData.direction == Down)
                {
                    cornerMoveDown(aiTankData);
                }
            }

            //-----------------top right corner;
            else if(tankPosSign == 1)
            {
                if(aiTankData.direction == Left)
                {
                    cornerMoveLeft(aiTankData);
                }
                else if(aiTankData.direction == Down)
                {
                    cornerMoveDown(aiTankData);
                }
            }

            //-----------------bottom left corner;
            else if(tankPosSign == 2)
            {
                if(aiTankData.direction == Up)
                {
                    cornerMoveUp(aiTankData);
                }
                else if(aiTankData.direction == Right)
                {
                    cornerMoveRight(aiTankData);
                }
            }

            //-----------------bottom right corner;
            else if(tankPosSign == 3)
            {
                if(aiTankData.direction == Up)
                {
                    cornerMoveUp(aiTankData);
                }
                else if(aiTankData.direction == Left)
                {
                    cornerMoveLeft(aiTankData);
                }
            }

            //-------------------------top border;
            else if(tankPosSign == 4)
            {
                if(aiTankData.direction == Right)
                {
                    cornerMoveRight(aiTankData);
                }
                else if(aiTankData.direction == Down)
                {
                    cornerMoveDown(aiTankData);
                }
                else if(aiTankData.direction == Left)
                {
                    cornerMoveLeft(aiTankData);
                }
            }

            //-----------------------right border;
            else if(tankPosSign == 5)
            {
                if(aiTankData.direction == Up)
                {
                    cornerMoveUp(aiTankData);
                }
                else if(aiTankData.direction == Down)
                {
                    cornerMoveDown(aiTankData);
                }
                else if(aiTankData.direction == Left)
                {
                    cornerMoveLeft(aiTankData);
                }
            }

            //----------------------bottom border;
            else if(tankPosSign == 6)
            {
                if(aiTankData.direction == Up)
                {
                    cornerMoveUp(aiTankData);
                }
                else if(aiTankData.direction == Right)
                {
                    cornerMoveRight(aiTankData);
                }
                else if(aiTankData.direction == Left)
                {
                    cornerMoveLeft(aiTankData);
                }
            }

            //------------------------left border;
            else if(tankPosSign == 7)
            {
                if(aiTankData.direction == Up)
                {
                    cornerMoveUp(aiTankData);
                }
                else if(aiTankData.direction == Right)
                {
                    cornerMoveRight(aiTankData);
                }
                else if(aiTankData.direction == Down)
                {
                    cornerMoveDown(aiTankData);
                }
            }
        }

        //if tank inside on cell area
        else
        {
            if(aiTankData.direction == Up)
                cornerMoveUp(aiTankData);
            else if(aiTankData.direction == Right)
                cornerMoveRight(aiTankData);
            else if(aiTankData.direction == Down)
                cornerMoveDown(aiTankData);
            else if(aiTankData.direction == Left)
                cornerMoveLeft(aiTankData);
        }
    }
}

void bcGame::cornerMoveRight(const AiTankStruct &aiTankData)
{
    TankShell tShell {};
    int step = 1;
    QVector<int> emptyCells, shellsCells;
    // If next position is empty;
    if(getStateByPos(aiTankData.coords[7] + 1) == Empty && getStateByPos(aiTankData.coords[11] + 1) == Empty)
    {
        shellsCells.append(aiTankData.coords[7] + step);
        shellsCells.append(aiTankData.coords[11] + step);
        setStateByPos(aiTankData.coords[7] + step, Shell);
        setStateByPos(aiTankData.coords[11] + step, Shell);
        emit sigDrawShells(shellsCells);
        //add data to shell's array;
        tShell.position_1 = aiTankData.coords[7] + step;
        tShell.position_2 = aiTankData.coords[11] + step;
        tShell.direction = aiTankData.direction;
        tShell.owner = aiTankData.id;
        m_ShellsArr.append(tShell);
        addShellToAiTank(aiTankData);
        // If shell owner player 1 add shell to player 1 data;
        if(aiTankData.id == -1)
            m_pPlayer1Tank->addShell();
    }
    else
    {
        // Brick & brick
        if(getStateByPos(aiTankData.coords[7] + step) == Brick && getStateByPos(aiTankData.coords[11] + step) == Brick)
        {
            emptyCells.append(aiTankData.coords[7] + step);
            emptyCells.append(aiTankData.coords[11] + step);
            setStateByPos(aiTankData.coords[7] + step, Empty);
            setStateByPos(aiTankData.coords[11] + step, Empty);
            // Check neigborhourd positions;
            if(getStateByPos(aiTankData.coords[3] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[3] + step);
                setStateByPos(aiTankData.coords[3] + step, Empty);
            }
            if(getStateByPos(aiTankData.coords[15] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[15] + step);
                setStateByPos(aiTankData.coords[15] + step, Empty);
            }
            emit sigDrawEmptyCell(emptyCells);
        }
        // Empty,iron & brick
        else if((getStateByPos(aiTankData.coords[7] + step) == Empty || getStateByPos(aiTankData.coords[7] + step) == Iron) && getStateByPos(aiTankData.coords[11] + 1) == Brick)
        {
            emptyCells.append(aiTankData.coords[11] + step);
            setStateByPos(aiTankData.coords[11] + step, Empty);
            if(getStateByPos(aiTankData.coords[15] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[15] + step);
                setStateByPos(aiTankData.coords[15] + step, Empty);
            }
            emit sigDrawEmptyCell(emptyCells);
        }
        // Brick & empty,iron
        else if(getStateByPos(aiTankData.coords[7] + step) == Brick && (getStateByPos(aiTankData.coords[11] + step) == Empty || getStateByPos(aiTankData.coords[11] + 1) == Iron))
        {
            emptyCells.append(aiTankData.coords[7] + step);
            setStateByPos(aiTankData.coords[7] + step, Empty);
            if(getStateByPos(aiTankData.coords[3] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[3] + step);
                setStateByPos(aiTankData.coords[3] + step, Empty);
            }
            emit sigDrawEmptyCell(emptyCells);
        }
        // Tank
        else if(getStateByPos(aiTankData.coords[7] + step) == AiTank || getStateByPos(aiTankData.coords[11] + step) == AiTank)
        {
            // If player 1 shell;
            if(aiTankData.id == -1)
            {
                for(int i(0); i < m_AiTanksArray.size(); ++i)
                {
                    for(int j(0); j < aiTankData.coords.size(); ++j)
                    {
                        // Delete ai tank and draw empty cells;
                        if(m_AiTanksArray.at(i).coords.contains(aiTankData.coords.at(j) + step))
                        {
                            for(int k(0); k < m_AiTanksArray.at(i).coords.size(); ++k)
                            {
                                setStateByPos(m_AiTanksArray.at(i).coords.at(k), Empty);
                                emptyCells.append(m_AiTanksArray.at(i).coords.at(k));
                            }
                            m_AiTanksArray.removeAt(i);
                            emit sigDrawEmptyCell(emptyCells);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void bcGame::cornerMoveDown(const AiTankStruct &aiTankData)
{
    TankShell tShell {};
    int step = 52;
    QVector<int> emptyCells, shellsCells;
    //if next position is empty;
    if(getStateByPos(aiTankData.coords[13] + step) == Empty && getStateByPos(aiTankData.coords[14] + step) == Empty)
    {
        shellsCells.append(aiTankData.coords[13] + step);
        shellsCells.append(aiTankData.coords[14] + step);
        setStateByPos(aiTankData.coords[13] + step, Shell);
        setStateByPos(aiTankData.coords[14] + step, Shell);
        emit sigDrawShells(emptyCells);
        //add data to shell's array;
        tShell.position_1 = aiTankData.coords[13] + step;
        tShell.position_2 = aiTankData.coords[14] + step;
        tShell.direction = aiTankData.direction;
        tShell.owner = aiTankData.id;
        m_ShellsArr.append(tShell);
        addShellToAiTank(aiTankData);
        // If shell owner player 1 add shell to player 1 data;
        if(aiTankData.id == -1)
            m_pPlayer1Tank->addShell();
    }
    else
    {
        //brick & brick
        if(getStateByPos(aiTankData.coords[13] + step) == Brick && getStateByPos(aiTankData.coords[14] + step) == Brick)
        {
            emptyCells.append(aiTankData.coords[13] + step);
            emptyCells.append(aiTankData.coords[14] + step);
            setStateByPos(aiTankData.coords[13] + step, Empty);
            setStateByPos(aiTankData.coords[14] + step, Empty);
            //check neigborhourd positions;
            if(getStateByPos(aiTankData.coords[12] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[12] + step);
                setStateByPos(aiTankData.coords[12] + step, Empty);
            }
            if(getStateByPos(aiTankData.coords[15] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[15] + step);
                setStateByPos(aiTankData.coords[15] + step, Empty);
            }
            emit sigDrawEmptyCell(emptyCells);
        }
        //empty,iron & brick
        else if((getStateByPos(aiTankData.coords[13] + step) == Empty || getStateByPos(aiTankData.coords[13] + step) == Iron) && getStateByPos(aiTankData.coords[14] + step) == Brick)
        {
            emptyCells.append(aiTankData.coords[14] + step);
            setStateByPos(aiTankData.coords[14] + step, Empty);
            if(getStateByPos(aiTankData.coords[15] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[15] + step);
                setStateByPos(aiTankData.coords[15] + step, Empty);
            }
            emit sigDrawEmptyCell(emptyCells);
        }
        //brick & empty,iron
        else if(getStateByPos(aiTankData.coords[13] + step) == Brick && (getStateByPos(aiTankData.coords[14] + 52) == Empty || getStateByPos(aiTankData.coords[14] + 52) == Iron))
        {
            emptyCells.append(aiTankData.coords[13] + step);
            setStateByPos(aiTankData.coords[13] + step, Empty);
            if(getStateByPos(aiTankData.coords[12] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[12] + step);
                setStateByPos(aiTankData.coords[12] + step, Empty);
            }
            emit sigDrawEmptyCell(emptyCells);
        }
        // Tank
        else if(getStateByPos(aiTankData.coords[13] + step) == AiTank || getStateByPos(aiTankData.coords[14] + step) == AiTank)
        {
            // If player 1 shell;
            if(aiTankData.id == -1)
            {
                for(int i(0); i < m_AiTanksArray.size(); ++i)
                {
                    for(int j(0); j < aiTankData.coords.size(); ++j)
                    {
                        // Delete ai tank and draw empty cells;
                        if(m_AiTanksArray.at(i).coords.contains(aiTankData.coords.at(j) + step))
                        {
                            for(int k(0); k < m_AiTanksArray.at(i).coords.size(); ++k)
                            {
                                setStateByPos(m_AiTanksArray.at(i).coords.at(k), Empty);
                                emptyCells.append(m_AiTanksArray.at(i).coords.at(k));
                            }
                            m_AiTanksArray.removeAt(i);
                            emit sigDrawEmptyCell(emptyCells);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void bcGame::cornerMoveLeft(const AiTankStruct &aiTankData)
{
    TankShell tShell {};
    int step = -1;
    QVector<int> emptyCells, shellsCells;
    //if next position is empty;
    if(getStateByPos(aiTankData.coords[4] + step) == Empty && getStateByPos(aiTankData.coords[8] + step) == Empty)
    {
        shellsCells.append(aiTankData.coords[4] + step);
        shellsCells.append(aiTankData.coords[8] + step);
        setStateByPos(aiTankData.coords[4] + step, Shell);
        setStateByPos(aiTankData.coords[8] + step, Shell);
        emit sigDrawEmptyCell(shellsCells);
        //add data to shell's array;
        tShell.position_1 = aiTankData.coords[4] + step;
        tShell.position_2 = aiTankData.coords[8] + step;
        tShell.direction = aiTankData.direction;
        tShell.owner = aiTankData.id;
        m_ShellsArr.append(tShell);
        addShellToAiTank(aiTankData);
        // If shell owner player 1 add shell to player 1 data;
        if(aiTankData.id == -1)
            m_pPlayer1Tank->addShell();
    }
    else
    {
        //brick & brick
        if(getStateByPos(aiTankData.coords[4] + step) == Brick && getStateByPos(aiTankData.coords[8] + step) == Brick)
        {
            emptyCells.append(aiTankData.coords[4] + step);
            emptyCells.append(aiTankData.coords[8] + step);
            setStateByPos(aiTankData.coords[4] + step, Empty);
            setStateByPos(aiTankData.coords[8] + step, Empty);
            //check neighborhood
            if(getStateByPos(aiTankData.coords[0] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[0] + step);
                setStateByPos(aiTankData.coords[0] + step, Empty);
            }
            if(getStateByPos(aiTankData.coords[12] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[12] + step);
                setStateByPos(aiTankData.coords[12] + step, Empty);
            }
            emit sigDrawEmptyCell(emptyCells);
        }
        //empty & iron, brick
        else if((getStateByPos(aiTankData.coords[4] + step) == Empty || getStateByPos(aiTankData.coords[4] + step) == Iron) && getStateByPos(aiTankData.coords[8] + step) == Brick)
        {
            emptyCells.append(aiTankData.coords[8] + step);
            setStateByPos(aiTankData.coords[8] + step, Empty);
            if(getStateByPos(aiTankData.coords[12] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[12] + step);
                setStateByPos(aiTankData.coords[12] + step, Empty);
            }
            emit sigDrawEmptyCell(emptyCells);
        }
        //brick, empty & iron
        else if(getStateByPos(aiTankData.coords[4] + step) == Brick && (getStateByPos(aiTankData.coords[8] + step) == Empty || getStateByPos(aiTankData.coords[8] + step) == Iron))
        {
            emptyCells.append(aiTankData.coords[4] + step);
            setStateByPos(aiTankData.coords[4] + step, Empty);
            if(getStateByPos(aiTankData.coords[0] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[0] + step);
                setStateByPos(aiTankData.coords[0] + step, Empty);
            }
            emit sigDrawEmptyCell(emptyCells);
        }
        // Tank
        else if(getStateByPos(aiTankData.coords[4] + step) == AiTank || getStateByPos(aiTankData.coords[8] + step) == AiTank)
        {
            // If player 1 shell;
            if(aiTankData.id == -1)
            {
                for(int i(0); i < m_AiTanksArray.size(); ++i)
                {
                    for(int j(0); j < aiTankData.coords.size(); ++j)
                    {
                        // Delete ai tank and draw empty cells;
                        if(m_AiTanksArray.at(i).coords.contains(aiTankData.coords.at(j) + step))
                        {
                            for(int k(0); k < m_AiTanksArray.at(i).coords.size(); ++k)
                            {
                                setStateByPos(m_AiTanksArray.at(i).coords.at(k), Empty);
                                emptyCells.append(m_AiTanksArray.at(i).coords.at(k));
                            }
                            m_AiTanksArray.removeAt(i);
                            emit sigDrawEmptyCell(emptyCells);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void bcGame::cornerMoveUp(const AiTankStruct &aiTankData)
{
    TankShell tShell {};
    int step = -52;
    QVector<int> emptyCells, shellsCells;
    //if next position is empty;
    if(getStateByPos(aiTankData.coords[1] + step) == Empty && getStateByPos(aiTankData.coords[2] + step) == Empty)
    {
        shellsCells.append(aiTankData.coords[1] + step);
        shellsCells.append(aiTankData.coords[2] + step);
        setStateByPos(aiTankData.coords[1] + step, Shell);
        setStateByPos(aiTankData.coords[2] + step, Shell);
        emit sigDrawShells(emptyCells);
        //add data to shell's array;
        tShell.position_1 = aiTankData.coords[1] + step;
        tShell.position_2 = aiTankData.coords[2] + step;
        tShell.direction = aiTankData.direction;
        tShell.owner = aiTankData.id;
        m_ShellsArr.append(tShell);
        addShellToAiTank(aiTankData);
        // If shell owner player 1 add shell to player 1 data;
        if(aiTankData.id == -1)
            m_pPlayer1Tank->addShell();
    }
    else
    {
        //brick & brick
        if(getStateByPos(aiTankData.coords[1] + step) == Brick && getStateByPos(aiTankData.coords[2] + step) == Brick)
        {
            emptyCells.append(aiTankData.coords[1] + step);
            emptyCells.append(aiTankData.coords[2] + step);
            setStateByPos(aiTankData.coords[1] + step, Empty);
            setStateByPos(aiTankData.coords[2] + step, Empty);
            //check neigborhourd positions;
            if(getStateByPos(aiTankData.coords[0] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[0] + step);
                setStateByPos(aiTankData.coords[0] + step, Empty);
            }
            if(getStateByPos(aiTankData.coords[3] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[3] + step);
                setStateByPos(aiTankData.coords[3] + step, Empty);
            }
            emit sigDrawEmptyCell(emptyCells);
        }
        //empty,iron & brick
        else if((getStateByPos(aiTankData.coords[1] + step) == Empty || getStateByPos(aiTankData.coords[1] + step) == Iron) && getStateByPos(aiTankData.coords[2] + step) == Brick)
        {
            emptyCells.append(aiTankData.coords[2] + step);
            setStateByPos(aiTankData.coords[2] + step, Empty);
            if(getStateByPos(aiTankData.coords[3] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[3] + step);
                setStateByPos(aiTankData.coords[3] + step, Empty);
            }
            emit sigDrawEmptyCell(emptyCells);
        }
        //brick & empty,iron
        else if(getStateByPos(aiTankData.coords[1] + step) == Brick && (getStateByPos(aiTankData.coords[0] + step) == Empty || getStateByPos(aiTankData.coords[0] + step) == Iron))
        {
            emptyCells.append(aiTankData.coords[1] + step);
            setStateByPos(aiTankData.coords[1] + step, Empty);
            if(getStateByPos(aiTankData.coords[0] + step) == Brick)
            {
                emptyCells.append(aiTankData.coords[0] + step);
                setStateByPos(aiTankData.coords[0] + step, Empty);
            }
            emit sigDrawEmptyCell(emptyCells);
        }
        // Tank
        else if(getStateByPos(aiTankData.coords[1] + step) == AiTank || getStateByPos(aiTankData.coords[2] + step) == AiTank)
        {
            // If player 1 shell;
            if(aiTankData.id == -1)
            {
                for(int i(0); i < m_AiTanksArray.size(); ++i)
                {
                    for(int j(0); j < aiTankData.coords.size(); ++j)
                    {
                        // Delete ai tank and draw empty cells;
                        if(m_AiTanksArray.at(i).coords.contains(aiTankData.coords.at(j) + step))
                        {
                            for(int k(0); k < m_AiTanksArray.at(i).coords.size(); ++k)
                            {
                                setStateByPos(m_AiTanksArray.at(i).coords.at(k), Empty);
                                emptyCells.append(m_AiTanksArray.at(i).coords.at(k));
                            }
                            m_AiTanksArray.removeAt(i);
                            emit sigDrawEmptyCell(emptyCells);
                            break;
                        }
                    }
                }
            }
        }
    }
}

// IMPORTANT FUNCTION; !NOT COMPLEATED!
void bcGame::moveAiShells()
{
    //qDebug() << "moveAiShells func()";
    if(!m_ShellsArr.isEmpty())
    {
        //int shellToDeleteId = -2;
        QVector<int> emptyCells, shellCells;
        for(int i(0); i < m_ShellsArr.size(); ++i)
        {
            emptyCells.clear();
            shellCells.clear();
            //check for previous shell
            //deletePrevShell(m_ShellsArr.at(i));

            int shellPosSign = whereIsShell(m_ShellsArr.at(i));

            // All beginning from previous EMPTY!

            // if shell on border or on the corner;
            if(shellPosSign > 0)
            {
                if(shellPosSign > 0 && shellPosSign <= 7)
                {
                    shellCells.append(m_ShellsArr.at(i).position_1);
                    shellCells.append(m_ShellsArr.at(i).position_2);
                    emit sigDrawShells(shellCells);
                    emptyCells.append(m_ShellsArr.at(i).position_1);
                    emptyCells.append(m_ShellsArr.at(i).position_2);
                    setStateByPos(m_ShellsArr[i].position_1, Empty);
                    setStateByPos(m_ShellsArr[i].position_2, Empty);
                }
                removeShellFromAiTank(m_ShellsArr.at(i));
                m_ShellsArr.removeAt(i);
                emit sigDrawEmptyCell(emptyCells);
            }

            // if shell inside on cell area
            else
            {
                int shellStep = 0, neighbor_1 = 0, neighbor_2 = 0;
                if(m_ShellsArr.at(i).direction == Up)
                {
                    shellStep = -52;
                    neighbor_1 = -1;
                    neighbor_2 = 1;
                }
                else if(m_ShellsArr.at(i).direction == Right)
                {
                    shellStep = 1;
                    neighbor_1 = -52;
                    neighbor_2 = 52;
                }
                else if(m_ShellsArr.at(i).direction == Down)
                {
                    shellStep = 52;
                    neighbor_1 = -1;
                    neighbor_2 = +1;
                }
                else if(m_ShellsArr.at(i).direction == Left)
                {
                    shellStep = -1;
                    neighbor_1 = -52;
                    neighbor_2 = 52;
                }

                //                // IF TANK BLOCK;
                //                if(getStateByPos(m_ShellsArr.at(i).position_1 + shellStep) == AiTank
                //                        || getStateByPos(m_ShellsArr.at(i).position_2 + shellStep) == AiTank)
                //                {
                //                    // If player 1 shell owner;
                //                    if(m_ShellsArr.at(i).owner == -1)
                //                    {
                //                        // Pass the aiTank's array;
                //                        for(int k(0); k < m_AiTanksArray.size(); ++k)
                //                        {
                //                            if(m_AiTanksArray.at(k).coords.contains(m_ShellsArr.at(i).position_1 + shellStep) ||
                //                                    m_AiTanksArray.at(k).coords.contains(m_ShellsArr.at(i).position_2 + shellStep))
                //                            {
                //                                // Get aiTank coords to delete;
                //                                for(int n(0); n < m_AiTanksArray.at(k).coords.size(); ++n)
                //                                {
                //                                    setStateByPos(m_AiTanksArray.at(k).coords.at(n), Empty);
                //                                    emptyCells.append(m_AiTanksArray.at(k).coords.at(n));
                //                                }
                //                                setStateByPos(m_ShellsArr.at(i).position_1, Empty);
                //                                setStateByPos(m_ShellsArr.at(i).position_2, Empty);
                //                                emptyCells.append(m_ShellsArr.at(i).position_1);
                //                                emptyCells.append(m_ShellsArr.at(i).position_2);
                //                                m_AiTanksArray.removeAt(k);
                //                                m_pPlayer1Tank->removeShell();
                //                                m_ShellsArr.removeAt(i);
                //                                emit sigDrawEmptyCell(emptyCells);
                //                                qDebug() << "Match End";
                //                            }
                //                        }
                //                    }
                //                }

                //                // IF BASE BLOCK;
                //                if(getStateByPos(m_ShellsArr.at(i).position_1 + shellStep) == Eagle
                //                        || getStateByPos(m_ShellsArr.at(i).position_2 + shellStep) == Eagle)
                //                    emit gameOver();

                //                // IF SHELL & SHELL CHECK BLOCK;
                //                if(getStateByPos(m_ShellsArr.at(i).position_1 + shellStep) == Shell
                //                        || getStateByPos(m_ShellsArr.at(i).position_2 + shellStep) == Shell)
                //                {
                //                    // remove current shell;
                //                    setStateByPos(m_ShellsArr.at(i).position_1, Empty);
                //                    setStateByPos(m_ShellsArr.at(i).position_2, Empty);
                //                    emptyCells.append(m_ShellsArr.at(i).position_1);
                //                    emptyCells.append(m_ShellsArr.at(i).position_1);
                //                    removeShellFromAiTank(m_ShellsArr.at(i));
                //                    m_ShellsArr.removeAt(i);
                //                    //remove counter shell;
                //                    for(int j(0); j < m_ShellsArr.size(); ++j)
                //                    {
                //                        if( m_ShellsArr.at(j).position_1 == m_ShellsArr.at(j).position_1 + shellStep
                //                                || m_ShellsArr.at(j).position_1 == m_ShellsArr.at(j).position_2 + shellStep
                //                                || m_ShellsArr.at(j).position_2 == m_ShellsArr.at(j).position_1 + shellStep
                //                                || m_ShellsArr.at(j).position_2 == m_ShellsArr.at(j).position_2 + shellStep )
                //                        {

                //                            setStateByPos(m_ShellsArr.at(j).position_1, Empty);
                //                            setStateByPos(m_ShellsArr.at(j).position_2, Empty);
                //                            emptyCells.append(m_ShellsArr.at(j).position_1);
                //                            emptyCells.append(m_ShellsArr.at(j).position_2);
                //                            removeShellFromAiTank(m_ShellsArr.at(j));
                //                            m_ShellsArr.removeAt(j);
                //                        }
                //                        break;
                //                    }
                //                    emit sigDrawEmptyCell(emptyCells);
                //                }

                // Go to the next step;
                m_ShellsArr[i].position_1 += shellStep;
                m_ShellsArr[i].position_2 += shellStep;

                if(getStateByPos(m_ShellsArr.at(i).position_1) == Empty
                        && getStateByPos(m_ShellsArr.at(i).position_2) == Empty)
                {
                    //qDebug() << "Empty & empty check block";
                    deletePrevShell(m_ShellsArr.at(i), shellStep);
                    shellCells.append(m_ShellsArr.at(i).position_1);
                    shellCells.append(m_ShellsArr.at(i).position_2);
                    setStateByPos(m_ShellsArr[i].position_1, Shell);
                    setStateByPos(m_ShellsArr[i].position_2, Shell);
                    emit sigDrawShells(shellCells);
                }

                if(getStateByPos(m_ShellsArr.at(i).position_1) == Iron
                        && getStateByPos(m_ShellsArr.at(i).position_2) == Iron)
                {
                    //qDebug() << "Iron & iron block check";
                    deletePrevShell(m_ShellsArr.at(i), shellStep);
                    removeShellFromAiTank(m_ShellsArr.at(i));
                    m_ShellsArr.removeAt(i);
                }

                if(getStateByPos(m_ShellsArr.at(i).position_1) == Brick
                        && getStateByPos(m_ShellsArr.at(i).position_2) == Brick)
                {
                    //qDebug() << "Brick & brick block check";
                    deletePrevShell(m_ShellsArr.at(i), shellStep);
                    setStateByPos(m_ShellsArr[i].position_1, Empty);
                    setStateByPos(m_ShellsArr[i].position_2, Empty);

                    emptyCells.append(m_ShellsArr[i].position_1);
                    emptyCells.append(m_ShellsArr[i].position_2);

                    //check for neighbourhood;
                    if(getStateByPos(m_ShellsArr.at(i).position_1 + neighbor_1) == Brick
                            && getStateByPos(m_ShellsArr.at(i).position_2 + neighbor_2) == Brick)
                    {
                        emptyCells.append(m_ShellsArr.at(i).position_1+ neighbor_1);
                        emptyCells.append(m_ShellsArr.at(i).position_2 +neighbor_2);
                        setStateByPos(m_ShellsArr.at(i).position_1 +neighbor_1, Empty);
                        setStateByPos(m_ShellsArr.at(i).position_2 + neighbor_2, Empty);
                    }
                    emit sigDrawEmptyCell(emptyCells);
                    //remove shell from data array and ai tank;
                    removeShellFromAiTank(m_ShellsArr.at(i));
                    m_ShellsArr.removeAt(i);
                }

                // IF SHELL & SHELL CHECK BLOCK; TIP: work only with ai and palyer shell;
                if(getStateByPos(m_ShellsArr.at(i).position_1 + shellStep) == Shell
                        || getStateByPos(m_ShellsArr.at(i).position_2 + shellStep) == Shell)
                {
                    qDebug() << "ShellShell1 : " << m_ShellsArr.size();
//                    for(int j(0); j < m_ShellsArr.size(); ++j)
//                    {
//                        if( m_ShellsArr.at(j).position_1 == m_ShellsArr.at(j).position_1 + shellStep
//                                || m_ShellsArr.at(j).position_1 == m_ShellsArr.at(j).position_2 + shellStep
//                                || m_ShellsArr.at(j).position_2 == m_ShellsArr.at(j).position_1 + shellStep
//                                || m_ShellsArr.at(j).position_2 == m_ShellsArr.at(j).position_2 + shellStep )
//                        {
//                            // If shell owner player 1;
//                            //if(m_ShellsArr.at(j).id == -1)
//                            //{
//                            setStateByPos(m_ShellsArr.at(j).position_1, Empty);
//                            setStateByPos(m_ShellsArr.at(j).position_2, Empty);
//                            emptyCells.append(m_ShellsArr.at(j).position_1);
//                            emptyCells.append(m_ShellsArr.at(j).position_2);
//                            deletePrevShell(m_ShellsArr.at(j), shellStep);
//                            removeShellFromAiTank(m_ShellsArr.at(j));
//                            m_ShellsArr.removeAt(j);
//                            shellToDeleteId = m_ShellsArr.at(j).owner;
//                                                        //}
//                        }
//                        break;
//                    }
//                    qDebug() << "ShellShell : " << m_ShellsArr.size();
//                    setStateByPos(m_ShellsArr.at(i).position_1, Empty);
//                    setStateByPos(m_ShellsArr.at(i).position_2, Empty);
//                    emptyCells.append(m_ShellsArr.at(i).position_1);
//                    emptyCells.append(m_ShellsArr.at(i).position_2);
//                    deletePrevShell(m_ShellsArr.at(i), shellStep);
//                    removeShellFromAiTank(m_ShellsArr.at(i));
//                    m_ShellsArr.removeAt(i);
//                    deleteShellFromShellsArray(shellToDeleteId);
//                    emit sigDrawEmptyCell(emptyCells);

                    //                    //remove counter shell;
                    //                    for(int j(0); j < m_ShellsArr.size(); ++j)
                    //                    {
                    //                        if( m_ShellsArr.at(j).position_1 == m_ShellsArr.at(j).position_1 + shellStep
                    //                                || m_ShellsArr.at(j).position_1 == m_ShellsArr.at(j).position_2 + shellStep
                    //                                || m_ShellsArr.at(j).position_2 == m_ShellsArr.at(j).position_1 + shellStep
                    //                                || m_ShellsArr.at(j).position_2 == m_ShellsArr.at(j).position_2 + shellStep )
                    //                        {

                    //                            setStateByPos(m_ShellsArr.at(j).position_1, Empty);
                    //                            setStateByPos(m_ShellsArr.at(j).position_2, Empty);
                    //                            emptyCells.append(m_ShellsArr.at(j).position_1);
                    //                            emptyCells.append(m_ShellsArr.at(j).position_2);
                    //                            removeShellFromAiTank(m_ShellsArr.at(j));
                    //                            m_ShellsArr.removeAt(j);
                    //                        }
                    //                        break;
                    //                    }
                    //                    // remove current shell;
                    //                    deletePrevShell(m_ShellsArr.at(i), shellStep);
                    //                    removeShellFromAiTank(m_ShellsArr.at(i));
                    //                    m_ShellsArr.removeAt(i);
                    //                    emit sigDrawEmptyCell(emptyCells);
                }


                // If next position is empty;
                //                if(getStateByPos(m_ShellsArr.at(i).position_1 + shellStep) == Empty
                //                        && getStateByPos(m_ShellsArr.at(i).position_2 + shellStep) == Empty)
                //                {
                //                    //qDebug() << "Empty & empty check block";
                //                    deletePrevShell(m_ShellsArr.at(i));
                //                    shellCells.append(m_ShellsArr.at(i).position_1 + shellStep);
                //                    shellCells.append(m_ShellsArr.at(i).position_2 + shellStep);
                //                    setStateByPos(m_ShellsArr[i].position_1 + shellStep, Shell);
                //                    setStateByPos(m_ShellsArr[i].position_2 + shellStep, Shell);

                //                    emit sigDrawShells(shellCells);
                //                    // Go to the next step;
                //                    m_ShellsArr[i].position_1 += shellStep;
                //                    m_ShellsArr[i].position_2 += shellStep;
                //                }

                // IRON & IRON CHECK BLOCK;
                //                if(getStateByPos(m_ShellsArr.at(i).position_1 + shellStep) == Iron
                //                        && getStateByPos(m_ShellsArr.at(i).position_2 + shellStep) == Iron)
                //                {
                //                    qDebug() << "Iron & iron block check";

                //                    shellCells.append(m_ShellsArr.at(i).position_1);
                //                    shellCells.append(m_ShellsArr.at(i).position_2);
                //                    emit sigDrawShells(shellCells);

                //                    emptyCells.append(m_ShellsArr[i].position_1);
                //                    emptyCells.append(m_ShellsArr[i].position_2);
                //                    setStateByPos(m_ShellsArr[i].position_1, Empty);
                //                    setStateByPos(m_ShellsArr[i].position_2, Empty);

                //                    removeShellFromAiTank(m_ShellsArr.at(i));
                //                    m_ShellsArr.removeAt(i);
                //                    emit sigDrawEmptyCell(emptyCells);
                //                }

                //                // BRICK & BRICK CHECK BLOCK;
                //                if(getStateByPos(m_ShellsArr.at(i).position_1 + shellStep) == Brick
                //                        && getStateByPos(m_ShellsArr.at(i).position_2 + shellStep) == Brick)
                //                {
                //                    //qDebug() << "Brick & brick block check";
                //                    setStateByPos(m_ShellsArr[i].position_1, Empty);
                //                    setStateByPos(m_ShellsArr[i].position_2, Empty);

                //                    setStateByPos(m_ShellsArr[i].position_1 + shellStep, Empty);
                //                    setStateByPos(m_ShellsArr[i].position_2 + shellStep, Empty);
                //                    emptyCells.append(m_ShellsArr[i].position_1 + shellStep);
                //                    emptyCells.append(m_ShellsArr[i].position_2 + shellStep);

                //                    //check for neighbourhood;
                //                    if(getStateByPos((m_ShellsArr.at(i).position_1 + shellStep) + neighbor_1) == Brick
                //                            && getStateByPos((m_ShellsArr.at(i).position_2 + shellStep) + neighbor_2) == Brick)
                //                    {
                //                        emptyCells.append((m_ShellsArr.at(i).position_1 + shellStep) + neighbor_1);
                //                        emptyCells.append((m_ShellsArr.at(i).position_2 + shellStep) + neighbor_2);
                //                        setStateByPos((m_ShellsArr.at(i).position_1 + shellStep) + neighbor_1, Empty);
                //                        setStateByPos((m_ShellsArr.at(i).position_2 + shellStep) + neighbor_2, Empty);
                //                    }
                //                    emit sigDrawEmptyCell(emptyCells);
                //                    //remove shell from data array and ai tank;
                //                    removeShellFromAiTank(m_ShellsArr.at(i));
                //                    m_ShellsArr.removeAt(i);
                //                }

                // BRICK & EMPTY CHECK BLOCK;
                if(getStateByPos(m_ShellsArr.at(i).position_1 + shellStep) == Brick
                        && getStateByPos(m_ShellsArr.at(i).position_2 + shellStep) == Empty)
                {
                    //qDebug() << "BRICK & EMPTY CHECK BLOCK";
                    // set current coords to empty;
                    setStateByPos(m_ShellsArr[i].position_1, Empty);
                    setStateByPos(m_ShellsArr[i].position_2, Empty);
                    // set next coords to empty;
                    setStateByPos(m_ShellsArr[i].position_1 + shellStep, Empty);

                    emptyCells.append(m_ShellsArr[i].position_1 + shellStep);
                    emptyCells.append(m_ShellsArr[i].position_1);
                    emptyCells.append(m_ShellsArr[i].position_2);

                    //check for neghbourhood;
                    if(getStateByPos((m_ShellsArr.at(i).position_1 + shellStep) + neighbor_1) == Brick)
                    {
                        emptyCells.append((m_ShellsArr.at(i).position_1 + shellStep) + neighbor_1);
                        setStateByPos((m_ShellsArr.at(i).position_1 + shellStep) + neighbor_1, Empty);
                    }
                    emit sigDrawEmptyCell(emptyCells);
                    //remove shell from data array and ai tank;
                    removeShellFromAiTank(m_ShellsArr.at(i));
                    m_ShellsArr.removeAt(i);
                }

                // EMPTY & BRICK CHECK BLOCK; 2
                if(getStateByPos(m_ShellsArr.at(i).position_1 + shellStep) == Empty
                        && getStateByPos(m_ShellsArr.at(i).position_2 + shellStep) == Brick)
                {
                    //qDebug() << "EMPTY & BRICK CHECK BLOCK";
                    // set current coords to empty;
                    setStateByPos(m_ShellsArr[i].position_1, Empty);
                    setStateByPos(m_ShellsArr[i].position_2, Empty);
                    // set next coords to empty;
                    setStateByPos(m_ShellsArr[i].position_2 + shellStep, Empty);

                    emptyCells.append(m_ShellsArr[i].position_2 + shellStep);
                    emptyCells.append(m_ShellsArr[i].position_1);
                    emptyCells.append(m_ShellsArr[i].position_2);

                    //check for neghbourhood;
                    if(getStateByPos((m_ShellsArr.at(i).position_2 + shellStep) + neighbor_2) == Brick)
                    {
                        emptyCells.append((m_ShellsArr.at(i).position_2 + shellStep) + neighbor_2);
                        setStateByPos((m_ShellsArr.at(i).position_2 + shellStep) + neighbor_2, Empty);
                    }
                    emit sigDrawEmptyCell(emptyCells);
                    //remove shell from data array and ai tank;
                    removeShellFromAiTank(m_ShellsArr.at(i));
                    m_ShellsArr.removeAt(i);
                }
            }
        }
    }
}

void bcGame::deletePrevShell(const TankShell &shellData, int step)
{
    QVector<int> emptyCells;
    emptyCells.append(shellData.position_1 - step);
    emptyCells.append(shellData.position_2 - step);
    setStateByPos(shellData.position_1 - step, Empty);
    setStateByPos(shellData.position_2 - step, Empty);
    emit sigDrawEmptyCell(emptyCells);
    //qDebug() << "DELETE";
}

int bcGame::whereIsShell(const TankShell &shellData)
{
    //Return values:
    //top left corner = 0 | top right corner = 1,
    //bottom left corner = 2 | bottom right corner = 3,
    //------------------------------------------------
    //top border = 4 | right border = 5,
    //bottom border = 6 | left border = 7,
    //no border = -1;

    QVector<int> leftBorder, rightBorder, bottomBorder, topBorder;
    QVector<int> cornerCoords = { 0, 51, 2652, 2703 };
    int loop = 0;
    for(int i(0); i < areaSize; ++i)
    {
        for(int j(0); j < areaSize; ++j)
        {
            if(j == 0)
            {
                //remove corner coords;
                if(!cornerCoords.contains(loop))
                    leftBorder.push_back(loop);
            }
            if(j == areaSize - 1)
            {
                if(!cornerCoords.contains(loop))
                    rightBorder.push_back(loop);
            }
            if(i == areaSize - 1)
            {
                if(!cornerCoords.contains(loop))
                    bottomBorder.append(loop);
            }
            if(i == 0)
            {
                if(!cornerCoords.contains(loop))
                    topBorder.append(loop);
            }
            ++loop;
        }
    }
    //check corner coords;
    if(shellData.position_1 == cornerCoords.at(0) || shellData.position_2 == cornerCoords.at(0))
        return 0;
    if(shellData.position_1 == cornerCoords.at(1) || shellData.position_2 == cornerCoords.at(1))
        return 1;
    if(shellData.position_1 == cornerCoords.at(2) || shellData.position_2 == cornerCoords.at(2))
        return 2;
    if(shellData.position_1 == cornerCoords.at(3) || shellData.position_2 == cornerCoords.at(3))
        return 3;

    //check border coords;
    if(topBorder.contains(shellData.position_1) && topBorder.contains(shellData.position_2))
        return 4;
    if(rightBorder.contains(shellData.position_1) && rightBorder.contains(shellData.position_2))
        return 5;
    if(bottomBorder.contains(shellData.position_1) && bottomBorder.contains(shellData.position_2))
        return 6;
    if(leftBorder.contains(shellData.position_1) && leftBorder.contains(shellData.position_2))
        return 7;

    return -1;
}

void bcGame::deleteShellFromShellsArray(const int ownerId)
{
    for(int i(0); i < m_ShellsArr.size(); ++i)
    {
        if(m_ShellsArr.at(i).owner == ownerId)
        {
            m_ShellsArr.removeAt(i);
        }
    }
}

void bcGame::addShellToAiTank(const AiTankStruct &aiTankData)
{
    for(int i(0); i < m_AiTanksArray.size(); ++i)
    {
        if(aiTankData.id == m_AiTanksArray.at(i).id)
            m_AiTanksArray[i].shell = 1;
    }
}

void bcGame::removeShellFromAiTank(const TankShell &shellData)
{
    if(shellData.owner == -1)
    {
        qDebug() << "Player 1 owner";
        m_pPlayer1Tank->removeShell();
    }
    else
    {
        for(int i(0); i < m_AiTanksArray.size(); ++i)
        {
            if(m_AiTanksArray.at(i).id == shellData.owner)
            {
                m_AiTanksArray[i].shell = 0;
            }
        }
    }
}


//check ai shells size on aitank
void bcGame::checkAiShellSize(int shellId)
{
    qDebug() << "Item no: "<< shellId;
    for(int i(0); i < m_AiTanksArray.size(); ++i)
    {
        if(m_AiTanksArray.at(i).id == shellId)
        {
            m_AiTanksArray[i].shell = 0;
        }
    }
}


void bcGame::remShellCollision(const int current_Coord, const int neighbor_Coord)
{
    for(int i(0); i < m_ShellsArr.size(); ++i)
    {
        if(m_ShellsArr.at(i).position_1 == neighbor_Coord || m_ShellsArr.at(i).position_2 == neighbor_Coord)
        {
            for(int j(0); j < m_AiTanksArray.size(); ++j)
            {
                if(m_AiTanksArray.at(j).coords.contains(neighbor_Coord))
                    m_AiTanksArray[j].shell = 0;
            }
            m_ShellsArr.removeAt(i);
        }
        if(m_ShellsArr.at(i).position_1 == current_Coord || m_ShellsArr.at(i).position_2 == current_Coord)
            m_ShellsArr.removeAt(i);
    }
}

void bcGame::clearPrevCoords(const int currentCoord_1, const int currentCoord_2, const int prevCoord_1, const int prevCoord_2, const TankShell &ts)
{
    if(getShellOwnerByCoord(currentCoord_1) == getShellOwnerByCoord(prevCoord_1) &&
            getShellOwnerByCoord(currentCoord_2) == getShellOwnerByCoord(prevCoord_2))
    {
        qDebug() << "MATCH " << ts.direction;
    }
}

int bcGame::getShellOwnerByCoord(const int coord)
{
    for(int i(0); i < m_ShellsArr.size(); ++i)
    {
        if(m_ShellsArr.at(i).position_1 == coord)
            return m_ShellsArr.at(i).owner;
        else if(m_ShellsArr.at(i).position_2 == coord)
            return m_ShellsArr.at(i).owner;
    }
    return coord;
}

void bcGame::checkPrevCoordsForShell(const int curCoord_1, const int curCoord_2, const int prevCoord_1, const int prevCoord_2)
{
    QVector<int> emptyArr;
    if(getStateByPos(prevCoord_1) == Tank && getStateByPos(prevCoord_2) == Tank)
    {
        setStateByPos(prevCoord_1, Shell);
        setStateByPos(prevCoord_2, Shell);
    }
    else if(getStateByPos(prevCoord_1) == Shell && getStateByPos(prevCoord_2) == Shell)
    {
        setStateByPos(prevCoord_1, Empty);
        setStateByPos(prevCoord_2, Empty);
        setStateByPos(prevCoord_1, Shell);
        setStateByPos(prevCoord_2, Shell);
        emptyArr.append(prevCoord_1);
        emptyArr.append(prevCoord_2);
    }
    QVector<int> shellArr = { curCoord_1, curCoord_2 };
    emit sigDrawShells(shellArr);
    if(!emptyArr.isEmpty())
        emit sigDrawEmptyCell(emptyArr);
}


bcGame::~bcGame()
{
    delete m_pPlayer1Tank;
}

void bcGame::slotTimerTimeOut()
{
    //запускаем таймер каждые пол секунды
    //this->moveShell();
}

void bcGame::eraseBonus()
{
    m_sBonusData.active = false;
    m_bonusTimer->stop();
    emit bonusEraseSignal();
}


//tank class implementation
PlayerTankObj::PlayerTankObj()
{
    m_tSpeed = 1;
    m_tCollar = 0;
    m_tLifeNumber = 2;
    m_tLevel = 1;
    m_tDirection = Up;
    m_tCoords = { 2512, 2513, 2514, 2515,
                  2564, 2565, 2566, 2567,
                  2616, 2617, 2618, 2619,
                  2668, 2669, 2670, 2671 };
}

void PlayerTankObj::setCoords(const int *arr)
{
    QVector<int> tmpArr;
    int cnt = 0;
    for(int i(0); i < areaSize; ++i)
    {
        for(int j(0); j < areaSize; ++j)
        {
            if(*(arr + i * areaSize + j) == bcGame::Tank)
            {
                tmpArr.append(cnt);
            }
            ++cnt;
        }
    }
    if(!tmpArr.isEmpty())
    {
        m_tCoords.clear();
        m_tCoords.append(tmpArr);
    }
    //qDebug() << m_tCoords << "\n" ;
}

void PlayerTankObj::setAiCoordsStep(AiTankStruct &inputObject, const int inputStep)
{
    for(int i(0); i < inputObject.coords.size(); ++i)
    {
        inputObject.coords[i] = inputObject.coords[i] + inputStep;
    }
}

void PlayerTankObj::setAiCoords(const QVector<int> &inputCoordsArr)
{
    m_tCoords = inputCoordsArr;
}

void PlayerTankObj::setSpeed(const int speed)
{
    m_tSpeed = speed;
}

void PlayerTankObj::setDirection(const int direction)
{
    m_tDirection = direction;
}

void PlayerTankObj::levelDown()
{
    if(getTankLevel() > 0)
        --m_tLevel;
}

AiTankStruct PlayerTankObj::convertToStruct(const int playerNumber)
{
    AiTankStruct tankStruct;
    tankStruct.coords = this->getTankCoords();
    tankStruct.level = this->getTankLevel();
    tankStruct.direction = this->getDirection();
    tankStruct.shell = this->getShellsSize();
    tankStruct.id = playerNumber;
    return tankStruct;
}

void PlayerTankObj::levelUp()
{
    if(getTankLevel() > 0 && getTankLevel() < 4)
        ++m_tLevel;
}

PlayerTankObj::~PlayerTankObj()
{

}
