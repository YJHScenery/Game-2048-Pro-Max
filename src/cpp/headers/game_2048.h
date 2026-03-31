//
// Created by jehor on 2026/3/31.
//

#ifndef GAME_2048_QUICK_GAME_2048_H
#define GAME_2048_QUICK_GAME_2048_H
#include "logic_2048_pm.h"
#include <QObject>

#include <QList>
#include <QString>
#include <QVariant>
#include <QVector>

struct GameData
{
    QString gameMode;       //! @note Static or Dynamic
    QList<int> sizeInfo;    //! @note {2, 2} means 2x2 Size; {2, 2, 2} means 2*2*2 Size; etc.
    QList<size_t> flatData; //! @note Game Board Matrix Data(Flatted), Row Major.
};

struct GameMode
{
    QString gameMode;
    QList<int> sizeInfo;
};

class Game2048 : public QObject
{
    Q_OBJECT
public:
    explicit Game2048(QObject *parent = nullptr);

    int parse2DSize(const QVariantList &sizeInfo) const;
    void emit2D(const QString &gameMode, int size);
    void reset2D(int size);
    void operate2D(int size, int dim, MoveDirection dir);

public slots:
    void on_ResetGame_emitted(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Up_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Down_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Left_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Right_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Forward_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Back_operated(const QString &gameMode, const QVariantList &sizeInfo);



signals:
    void sendGameData(const QString &gameMode, const QVariantList &sizeInfo, const QVariantList &flatData);

private:
    // bool ensure2DBoard(const QString &gameMode, const QVariantList &sizeInfo);
    // void reset2DBoard();
    // bool move2DUp();
    // bool move2DDown();
    // bool move2DLeft();
    // bool move2DRight();
    // void addRandomTile2D();
    // void emit2D();

    QString m_currentGameMode{"Static"};
    int m_rows2D{4};
    int m_cols2D{4};

    QVector<size_t> m_board2D;

    Logic2048_tm<size_t, size_t, 2, 4, 4> m_GameBoard4x4;
    Logic2048_tm<size_t, size_t, 2, 6, 6> m_GameBoard6x6;
    Logic2048_tm<size_t, size_t, 2, 8, 8> m_GameBoard8x8;
    Logic2048_tm<size_t, size_t, 3, 4, 4, 4> m_GameBoard4x4x4;
    Logic2048_tm<size_t, size_t, 3, 6, 6, 6> m_GameBoard6x6x6;
    Logic2048_tm<size_t, size_t, 3, 8, 8, 8> m_GameBoard8x8x8;
};
#endif // GAME_2048_QUICK_GAME_2048_H
