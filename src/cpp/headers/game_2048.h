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
#include <QVariantMap>

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

public slots:
    void on_ResetGame_emitted(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Up_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Down_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Left_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Right_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Forward_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Back_operated(const QString &gameMode, const QVariantList &sizeInfo);

    // ---- 3D slots (avoid name clash with 2D) ----
    void on_ResetGame3D_emitted(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Left3D_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Right3D_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Forward3D_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Back3D_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Down3D_operated(const QString &gameMode, const QVariantList &sizeInfo);

    void on_Up3D_operated(const QString &gameMode, const QVariantList &sizeInfo);

signals:
    void sendGameData(const QString &gameMode, const QVariantList &sizeInfo, const QVariantList &flatData);

    // 2D move trace for animation (方案1)
    void sendMoveTrace2D(const QString &gameMode,
                         const QVariantList &sizeInfo,
                         const QVariantList &flatData,
                         const QVariantList &moves,
                         const QVariantList &merges,
                         const QVariantMap &spawn);

    // ---- 3D signal (avoid name clash with 2D) ----
    void sendGameData3D(const QString &gameMode, const QVariantList &sizeInfo, const QVariantList &flatData);

    // 3D move trace for animation (方案1)
    void sendMoveTrace3D(const QString &gameMode,
                         const QVariantList &sizeInfo,
                         const QVariantList &flatData,
                         const QVariantList &moves,
                         const QVariantList &merges,
                         const QVariantMap &spawn);

private:
    int parse2DSize(const QVariantList &sizeInfo) const;
    void emit2D(const QString &gameMode, int size);
    void reset2D(int size);
    void operate2D(int size, int dim, MoveDirection dir);
    void operate2DAndEmitTrace(const QString &gameMode, int size, int dim, MoveDirection dir);

    int parse3DSize(const QVariantList &sizeInfo) const;
    void emit3D(const QString &gameMode, int size);
    void reset3D(int size);
    void operate3D(int size, int dim, MoveDirection dir);
    void operate3DAndEmitTrace(const QString &gameMode, int size, int dim, MoveDirection dir);

    Logic2048_tm<size_t, size_t, 2, 4, 4> m_GameBoard4x4;
    Logic2048_tm<size_t, size_t, 2, 6, 6> m_GameBoard6x6;
    Logic2048_tm<size_t, size_t, 2, 8, 8> m_GameBoard8x8;
    Logic2048_tm<size_t, size_t, 3, 4, 4, 4> m_GameBoard4x4x4;
    Logic2048_tm<size_t, size_t, 3, 6, 6, 6> m_GameBoard6x6x6;
    Logic2048_tm<size_t, size_t, 3, 8, 8, 8> m_GameBoard8x8x8;
};
#endif // GAME_2048_QUICK_GAME_2048_H
