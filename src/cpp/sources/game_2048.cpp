//
// Created by jehor on 2026/3/31.
//

#include "game_2048.h"

Game2048::Game2048(QObject *parent) : QObject(parent)
{
    // 默认初始化一个可用棋盘（避免 QML 首次进入时拿到未初始化数据）
    m_GameBoard4x4.resetAndSeed(2);
}

int Game2048::parse2DSize(const QVariantList &sizeInfo) const
{
    if (sizeInfo.size() >= 1)
    {
        const int s = sizeInfo.at(0).toInt();
        if (s == 4 || s == 6 || s == 8)
            return s;
    }
    return 4;
}

void Game2048::emit2D(const QString &gameMode, const int size)
{
    QVariantList outSize;
    outSize.reserve(2);
    outSize << size << size;

    QVariantList flat;

    if (size == 6)
    {
        const auto data = m_GameBoard6x6.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);
    }
    else if (size == 8)
    {
        const auto data = m_GameBoard8x8.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);
    }
    else
    {
        const auto data = m_GameBoard4x4.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);
    }

    emit sendGameData(gameMode.isEmpty() ? QStringLiteral("Static") : gameMode, outSize, flat);
}

void Game2048::reset2D(const int size)
{
    if (size == 6)
        m_GameBoard6x6.resetAndSeed(2);
    else if (size == 8)
        m_GameBoard8x8.resetAndSeed(2);
    else
        m_GameBoard4x4.resetAndSeed(2);
}

void Game2048::operate2D(const int size, const int dim, const MoveDirection dir)
{
    if (size == 6)
        (void)m_GameBoard6x6.operateAndSpawn(dim, dir);
    else if (size == 8)
        (void)m_GameBoard8x8.operateAndSpawn(dim, dir);
    else
        (void)m_GameBoard4x4.operateAndSpawn(dim, dir);
}

void Game2048::on_ResetGame_emitted(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    reset2D(size);
    emit2D(gameMode, size);
}

void Game2048::on_Up_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    operate2D(size, 0, MoveDirection::Negative);
    emit2D(gameMode, size);
}

void Game2048::on_Down_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    operate2D(size, 0, MoveDirection::Positive);
    emit2D(gameMode, size);
}

void Game2048::on_Left_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    operate2D(size, 1, MoveDirection::Negative);
    emit2D(gameMode, size);
}

void Game2048::on_Right_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    operate2D(size, 1, MoveDirection::Positive);
    emit2D(gameMode, size);
}

void Game2048::on_Forward_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    emit2D(gameMode, size);
}

void Game2048::on_Back_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    emit2D(gameMode, size);
}
