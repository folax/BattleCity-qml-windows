#ifndef BCGAME_H
#define BCGAME_H

#include <QObject>
#include <QVector>
#include <QtQml>

class QStringList;
class PlayerTankObj;
class QTimer;

enum Direction { Up, Down, Right, Left };

const int areaSize = 52;

struct TankShell
{
    int position_1;
    int position_2;
    int prev_position_1 = -1;
    int prev_position_2 = -1;
    int direction;
    int speed;
    int owner;
    bool onBorder = false;
};

struct AiTankStruct
{
    QVector<int> coords;
    int direction;
    int level;
    int shell;
    int id = -1;
};

class bcGame : public QObject
{
    Q_OBJECT

public:
    enum States { Brick, Iron, Forest, Ice, River, Empty, Respawn, Eagle, Shell, Tank, AiTank };
    enum Bonuses { eBomb, eLife, eShovel, eStar, eTime };
    Q_ENUMS(States)

    static void declareQML() {
        qmlRegisterType<bcGame>("CppStates", 1, 0, "States");
    }

    struct sBonus
    {
        QVector<int> coords;
        Bonuses bonus;
        bool active;
    };

    // Constructor;
    explicit bcGame();

    // Get cell state by cell position;
    int getStateByPos(const int);

    // Functions for QML side;
    Q_INVOKABLE QStringList getData();
    Q_INVOKABLE void importDataFromQML(const QVariantList&);
    Q_INVOKABLE void movePlayerTank(const int direction);
    Q_INVOKABLE void makeShot();

    Q_INVOKABLE void makeBonus();
    Q_INVOKABLE QVector<int> getTankCoords();

    ~bcGame();

private slots:
    void slotTimerTimeOut();

signals:
    //void sigPreviousShellPositions();
    void sigTankMoved(int direction);

    //new variant
    void sigDrawShells(QVector<int> sCoords);
    void sigDrawEmptyCell(QVector<int> eCoords);

    //for ai section signals
    void sigDrawAiTanks(QVector<int> aiToDrawData);
    void sigSizeOfAiArray(int aiArraySize);
    void sigDrawAiTanksQml(QVector<int> dataAiCoordsDirectionLevelArr, int dataAiArrSize, int totalAiSize);

    void gameOver();

    void bonusCreateSignal(int position, int bonus);
    void bonusEraseSignal();
    void tankLevelSignal(int level);

private:
    void initGame();
    void eraseBonus();

    void setStateByPos(const int, const int);
    bool isHaveBarrier(const QVector<int>&, int);
    bool isAiHaveBarrier(const QVector<int> aiCoords, const int aiDirection);
    bool isShellOutOfBorder(const QVector<TankShell> &, const int );
    bool isShellOnBorder(const TankShell&);


    int m_totalCurrentAiSize = 0;
    int m_statesArr[areaSize][areaSize];
    QStringList m_levelsPath;
    QTimer *m_Player1ShellSpeedTimer;
    QTimer *m_bonusTimer;

    // Ai respawn and movement timer;
    QTimer *m_respawnAiTanksTimer;
    QTimer *m_pAiMoveNormalTimer;
    QTimer *m_pAiMoveFastTimer;
    QTimer *m_pShellMove;

    // Shell array;
    QVector<TankShell> m_ShellsArr;
    //QVector<TankShell> m_shellsArr;

    //game level
    uint m_gameLevel;

    // Player 1 tank variable;
    PlayerTankObj *m_pPlayer1Tank;


    sBonus m_sBonusData;

    //AI create and manipulate

    void addAiTankToArr();
    void moveAiTank();
    bool checkForEmptyRespawn(const QVector<int> coords);
    void removeRespawnCells();
    int randDirection(const int limit);

    //AI MAKE SHELL SECTION;
    void aiMakeShot(const AiTankStruct &aiTankData);
    void cornerMoveRight(const AiTankStruct &aiTankData);
    void cornerMoveDown(const AiTankStruct &aiTankData);
    void cornerMoveLeft(const AiTankStruct &aiTankData);
    void cornerMoveUp(const AiTankStruct &aiTankData);
    int whereIsTank(const AiTankStruct &aiTankData);

    //AI MOVE SHELL SECTION;
    void moveAiShells();
    void deletePrevShell(const TankShell &shellData, int step = 0);
    int whereIsShell(const TankShell &shellData);
    void deleteShellFromShellsArray(const int ownerId);


    //new version
    QVector<AiTankStruct> m_AiTanksArray;
    void addShellToAiTank(const AiTankStruct &aiTankData);
    void removeShellFromAiTank(const TankShell &shellData);

    void checkAiShellSize(int shellId);
    void remShellCollision(const int current_Coord, const int neighbor_Coord = -1);
    void clearPrevCoords(const int currentCoord_1, const int currentCoord_2,
                         const int prevCoord_1, const int prevCoord_2, const TankShell& ts);
    int getShellOwnerByCoord(const int coord);

    void checkPrevCoordsForShell(const int curCoord_1, const int curCoord_2, const int prevCoord_1, const int prevCoord_2);
};


class PlayerTankObj : public QObject
{
    Q_OBJECT
public:
    explicit PlayerTankObj();

    QVector<int> getTankCoords() const { return m_tCoords; }
    inline int getDirection() const { return m_tDirection; }

    void setCoords(const int*);
    void setAiCoordsStep(AiTankStruct& inputObject, const int inputStep);
    void setAiCoords(const QVector<int>&inputCoordsArr);
    void setSpeed(const int);
    void setDirection(const int);

    inline void addShell() { ++m_tCollar; }
    inline void removeShell() { --m_tCollar; }

    inline void addLife() { ++m_tLifeNumber; }
    inline void minusLife() { --m_tLifeNumber; }
    inline int getTankLifeNumber() const { return m_tLifeNumber; }

    void levelUp();
    void levelDown();
    inline int getTankLevel() const { return m_tLevel; }
    inline void setTankLevel(const int level) { m_tLevel = level; }

    //SHOT
    inline int getShellsSize() const { return m_tCollar; }

    AiTankStruct convertToStruct(const int playerNumber);

    ~PlayerTankObj();

private:
    int m_tSpeed;
    int m_tLevel;
    int m_tLifeNumber;
    int m_tCollar;
    int m_tDirection;
    QVector<int> m_tCoords;
};

#endif // BCGAME_H
