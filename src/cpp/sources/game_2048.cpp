//
// Created by jehor on 2026/3/31.
//

#include "game_2048.h"

Game2048::Game2048(QObject *parent) : QObject(parent)
{
    // 默认初始化一个可用棋盘（避免 QML 首次进入时拿到未初始化数据）
    m_GameBoard4x4.resetAndSeed(2);
    m_GameBoard4x4x4.resetAndSeed(2);
}

int Game2048::parse2DSize(const QVariantList &sizeInfo)
{
    if (!sizeInfo.empty())
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

void Game2048::operate2DAndEmitTrace(const QString &gameMode, const int size, const int dim, const MoveDirection dir)
{
    QVariantList outSize;
    outSize.reserve(2);
    outSize << size << size;

    QVariantList flat;
    QVariantList moves;
    QVariantList merges;
    QVariantMap spawn;

    auto fillCommon = [&](const auto &trace)
    {
        moves.reserve(static_cast<int>(trace.moves.size()));
        for (const auto &m : trace.moves)
        {
            QVariantMap mm;
            mm.insert(QStringLiteral("from"), static_cast<qulonglong>(m.from));
            mm.insert(QStringLiteral("to"), static_cast<qulonglong>(m.to));
            mm.insert(QStringLiteral("value"), static_cast<qulonglong>(m.value));
            mm.insert(QStringLiteral("merged"), m.merged);
            mm.insert(QStringLiteral("primary"), m.primary);
            moves << mm;
        }

        merges.reserve(static_cast<int>(trace.merges.size()));
        for (const auto &me : trace.merges)
        {
            QVariantMap rm;
            rm.insert(QStringLiteral("to"), static_cast<qulonglong>(me.to));
            rm.insert(QStringLiteral("fromA"), static_cast<qulonglong>(me.fromA));
            rm.insert(QStringLiteral("fromB"), static_cast<qulonglong>(me.fromB));
            rm.insert(QStringLiteral("newValue"), static_cast<qulonglong>(me.newValue));
            merges << rm;
        }

        if (trace.spawn.has_value())
        {
            spawn.insert(QStringLiteral("index"), static_cast<qulonglong>(trace.spawn->index));
            spawn.insert(QStringLiteral("value"), static_cast<qulonglong>(trace.spawn->value));
        }
    };

    if (size == 6)
    {
        const auto trace = m_GameBoard6x6.operateAndSpawnTrace(dim, dir);

        const auto data = m_GameBoard6x6.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);

        fillCommon(trace);
    }
    else if (size == 8)
    {
        const auto trace = m_GameBoard8x8.operateAndSpawnTrace(dim, dir);

        const auto data = m_GameBoard8x8.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);

        fillCommon(trace);
    }
    else
    {
        const auto trace = m_GameBoard4x4.operateAndSpawnTrace(dim, dir);

        const auto data = m_GameBoard4x4.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);

        fillCommon(trace);
    }

    emit sendMoveTrace2D(gameMode.isEmpty() ? QStringLiteral("Static") : gameMode, outSize, flat, moves, merges, spawn);
}

int Game2048::parse3DSize(const QVariantList &sizeInfo)
{
    if (!sizeInfo.empty())
    {
        const int s = sizeInfo.at(0).toInt();
        if (s == 4 || s == 6 || s == 8)
            return s;
    }
    return 4;
}

void Game2048::emit3D(const QString &gameMode, const int size)
{
    QVariantList outSize;
    outSize.reserve(3);
    outSize << size << size << size;

    QVariantList flat;

    if (size == 6)
    {
        const auto data = m_GameBoard6x6x6.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);
    }
    else if (size == 8)
    {
        const auto data = m_GameBoard8x8x8.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);
    }
    else
    {
        const auto data = m_GameBoard4x4x4.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);
    }

    emit sendGameData3D(gameMode.isEmpty() ? QStringLiteral("Static") : gameMode, outSize, flat);
}

void Game2048::reset3D(const int size)
{
    if (size == 6)
        m_GameBoard6x6x6.resetAndSeed(2);
    else if (size == 8)
        m_GameBoard8x8x8.resetAndSeed(2);
    else
        m_GameBoard4x4x4.resetAndSeed(2);
}

void Game2048::operate3D(const int size, const int dim, const MoveDirection dir)
{
    if (size == 6)
        (void)m_GameBoard6x6x6.operateAndSpawn(dim, dir);
    else if (size == 8)
        (void)m_GameBoard8x8x8.operateAndSpawn(dim, dir);
    else
        (void)m_GameBoard4x4x4.operateAndSpawn(dim, dir);
}

void Game2048::operate3DAndEmitTrace(const QString &gameMode, const int size, const int dim, const MoveDirection dir)
{
    QVariantList outSize;
    outSize.reserve(3);
    outSize << size << size << size;

    QVariantList flat;
    QVariantList moves;
    QVariantList merges;
    QVariantMap spawn;

    auto fillCommon = [&](const auto &trace)
    {
        moves.reserve(static_cast<int>(trace.moves.size()));
        for (const auto &m : trace.moves)
        {
            QVariantMap mm;
            mm.insert(QStringLiteral("from"), static_cast<qulonglong>(m.from));
            mm.insert(QStringLiteral("to"), static_cast<qulonglong>(m.to));
            mm.insert(QStringLiteral("value"), static_cast<qulonglong>(m.value));
            mm.insert(QStringLiteral("merged"), m.merged);
            mm.insert(QStringLiteral("primary"), m.primary);
            moves << mm;
        }

        merges.reserve(static_cast<int>(trace.merges.size()));
        for (const auto &me : trace.merges)
        {
            QVariantMap rm;
            rm.insert(QStringLiteral("to"), static_cast<qulonglong>(me.to));
            rm.insert(QStringLiteral("fromA"), static_cast<qulonglong>(me.fromA));
            rm.insert(QStringLiteral("fromB"), static_cast<qulonglong>(me.fromB));
            rm.insert(QStringLiteral("newValue"), static_cast<qulonglong>(me.newValue));
            merges << rm;
        }

        if (trace.spawn.has_value())
        {
            spawn.insert(QStringLiteral("index"), static_cast<qulonglong>(trace.spawn->index));
            spawn.insert(QStringLiteral("value"), static_cast<qulonglong>(trace.spawn->value));
        }
    };

    if (size == 6)
    {
        const auto trace = m_GameBoard6x6x6.operateAndSpawnTrace(dim, dir);

        const auto data = m_GameBoard6x6x6.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);

        fillCommon(trace);
    }
    else if (size == 8)
    {
        const auto trace = m_GameBoard8x8x8.operateAndSpawnTrace(dim, dir);

        const auto data = m_GameBoard8x8x8.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);

        fillCommon(trace);
    }
    else
    {
        const auto trace = m_GameBoard4x4x4.operateAndSpawnTrace(dim, dir);

        const auto data = m_GameBoard4x4x4.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);

        fillCommon(trace);
    }

    emit sendMoveTrace3D(gameMode.isEmpty() ? QStringLiteral("Static") : gameMode, outSize, flat, moves, merges, spawn);
}

void Game2048::resetGame_emitted(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    reset2D(size);
    emit2D(gameMode, size);
}

void Game2048::up_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    operate2DAndEmitTrace(gameMode, size, 0, MoveDirection::Negative);
}

void Game2048::down_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    operate2DAndEmitTrace(gameMode, size, 0, MoveDirection::Positive);
}

void Game2048::left_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    operate2DAndEmitTrace(gameMode, size, 1, MoveDirection::Negative);
}

void Game2048::right_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    operate2DAndEmitTrace(gameMode, size, 1, MoveDirection::Positive);
}

// void Game2048::on_Forward_operated(const QString &gameMode, const QVariantList &sizeInfo)
// {
//     const int size = parse2DSize(sizeInfo);
//     emit2D(gameMode, size);
// }
//
// void Game2048::on_Back_operated(const QString &gameMode, const QVariantList &sizeInfo)
// {
//     const int size = parse2DSize(sizeInfo);
//     emit2D(gameMode, size);
// }

void Game2048::resetGame3D_emitted(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse3DSize(sizeInfo);
    reset3D(size);
    emit3D(gameMode, size);
}

void Game2048::left3D_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse3DSize(sizeInfo);
    // x axis (fastest-changing) is dim=2
    operate3DAndEmitTrace(gameMode, size, 2, MoveDirection::Negative);
}

void Game2048::right3D_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse3DSize(sizeInfo);
    operate3DAndEmitTrace(gameMode, size, 2, MoveDirection::Positive);
}

void Game2048::forward3D_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse3DSize(sizeInfo);
    // z axis (slowest-changing) is dim=0
    operate3DAndEmitTrace(gameMode, size, 0, MoveDirection::Negative);
}

void Game2048::back3D_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse3DSize(sizeInfo);
    operate3DAndEmitTrace(gameMode, size, 0, MoveDirection::Positive);
}

void Game2048::down3D_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse3DSize(sizeInfo);
    // y axis is dim=1
    operate3DAndEmitTrace(gameMode, size, 1, MoveDirection::Negative);
}

void Game2048::up3D_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse3DSize(sizeInfo);
    operate3DAndEmitTrace(gameMode, size, 1, MoveDirection::Positive);
}
