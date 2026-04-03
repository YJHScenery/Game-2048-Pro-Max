//
// Created by jehor on 2026/3/31.
//

#include "game_2048.h"

namespace
{
    QString keyFor2DSize(const int size)
    {
        return QStringLiteral("%1x%1").arg(size);
    }

    QString keyFor3DSize(const int size)
    {
        return QStringLiteral("%1x%1x%1").arg(size);
    }
}

Game2048::Game2048(QObject *parent) : QObject(parent), m_dataManager(GameDataManager::instance())
{
    // 默认初始化一个可用棋盘（避免 QML 首次进入时拿到未初始化数据）
    m_GameBoard4x4.resetAndSeed(2);
    m_GameBoard4x4x4.resetAndSeed(2);

    m_dataManager->initDatabase();

    loadAllData();

    loadAllHash();

    connect(this, &Game2048::updateGameScore, this, &Game2048::gameScore_updated, Qt::QueuedConnection);

    connect(this, &Game2048::gameOver, this, []()
            { qDebug() << "gameOver"; });
}

Game2048::~Game2048()
{
    saveAllData();
}

void Game2048::saveAllData()
{
    for (int i = 0; i < 6; ++i)
    {
        m_dataManager->saveGame(getDataWithIndex(i));
    }
}

void Game2048::loadAllData()
{
    GameData data4x4{m_dataManager->loadGame({4, 4})};
    GameData data6x6{m_dataManager->loadGame({6, 6})};
    GameData data8x8{m_dataManager->loadGame({8, 8})};
    GameData data4x4x4{m_dataManager->loadGame({4, 4, 4})};
    GameData data6x6x6{m_dataManager->loadGame({6, 6, 6})};
    GameData data8x8x8{m_dataManager->loadGame({8, 8, 8})};

    m_scoreMap[keyFor2DSize(4)] = {data4x4.maxScore, data4x4.currentScore};
    m_scoreMap[keyFor2DSize(6)] = {data6x6.maxScore, data6x6.currentScore};
    m_scoreMap[keyFor2DSize(8)] = {data8x8.maxScore, data8x8.currentScore};
    m_scoreMap[keyFor3DSize(4)] = {data4x4x4.maxScore, data4x4x4.currentScore};
    m_scoreMap[keyFor3DSize(6)] = {data6x6x6.maxScore, data6x6x6.currentScore};
    m_scoreMap[keyFor3DSize(8)] = {data8x8x8.maxScore, data8x8x8.currentScore};

    static_assert(std::is_same_v<QList<int>::size_type, qsizetype>);

    m_GameBoard4x4.setData<std::uint64_t, QList>(data4x4.flatTensorData);
    m_GameBoard6x6.setData<std::uint64_t, QList>(data6x6.flatTensorData);
    m_GameBoard8x8.setData<std::uint64_t, QList>(data8x8.flatTensorData);
    m_GameBoard4x4x4.setData<std::uint64_t, QList>(data4x4x4.flatTensorData);
    m_GameBoard6x6x6.setData<std::uint64_t, QList>(data6x6x6.flatTensorData);
    m_GameBoard8x8x8.setData<std::uint64_t, QList>(data8x8x8.flatTensorData);
}

void Game2048::loadAllHash()
{
    if (m_hashList.size() != 6)
        m_hashList = QList<uint64_t>(6);

    m_hashList[0] = m_GameBoard4x4.getHash();
    m_hashList[1] = m_GameBoard6x6.getHash();
    m_hashList[2] = m_GameBoard8x8.getHash();
    m_hashList[3] = m_GameBoard4x4x4.getHash();
    m_hashList[4] = m_GameBoard6x6x6.getHash();
    m_hashList[5] = m_GameBoard8x8x8.getHash();
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

void Game2048::reset2D(const int size, const bool reserveData)
{
    Q_UNUSED(reserveData)
    if (size == 6)
    {
        m_GameBoard6x6.resetAndSeed(2);
        m_hashList[1] = m_GameBoard6x6.getHash();
    }
    else if (size == 8)
    {
        m_GameBoard8x8.resetAndSeed(2);
        m_hashList[2] = m_GameBoard8x8.getHash();
    }
    else
    {
        m_GameBoard4x4.resetAndSeed(2);
        m_hashList[0] = m_GameBoard4x4.getHash();
    }
}

// void Game2048::operate2D(const int size, const int dim, const MoveDirection dir)
// {
//     if (size == 6)
//         (void)m_GameBoard6x6.operateAndSpawn(dim, dir);
//     else if (size == 8)
//         (void)m_GameBoard8x8.operateAndSpawn(dim, dir);
//     else
//         (void)m_GameBoard4x4.operateAndSpawn(dim, dir);
// }

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

            emit updateGameScore(QVariantList{size, size}, me.newValue);

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

        if (m_hashList[1] == m_GameBoard6x6.getHash())
        {
            std::vector<EqualPair> equals{m_GameBoard6x6.checkOver()};
            if (equals.empty())
            {
                emit gameOver();
            }
            else
            {
                for (const auto v : equals)
                {
                    qDebug() << v.dim << " " << v.pos;
                }
                qDebug() << "Not over";
            }
        }
        else
        {
            m_hashList[1] = m_GameBoard6x6.getHash();
        }
    }
    else if (size == 8)
    {
        const auto trace = m_GameBoard8x8.operateAndSpawnTrace(dim, dir);

        const auto data = m_GameBoard8x8.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);

        fillCommon(trace);

        if (m_hashList[2] == m_GameBoard8x8.getHash())
        {
            std::vector<EqualPair> equals{m_GameBoard8x8.checkOver()};
            if (equals.empty())
            {
                emit gameOver();
            }
            else
            {
                for (const auto v : equals)
                {
                    qDebug() << v.dim << " " << v.pos;
                }
                qDebug() << "Not over";
            }
        }
        else
        {
            m_hashList[2] = m_GameBoard8x8.getHash();
        }
    }
    else
    {
        const auto trace = m_GameBoard4x4.operateAndSpawnTrace(dim, dir);

        const auto data = m_GameBoard4x4.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);

        fillCommon(trace);

        if (m_hashList[0] == m_GameBoard4x4.getHash())
        {
            std::vector<EqualPair> equals{m_GameBoard4x4.checkOver()};
            if (equals.empty())
            {
                emit gameOver();
            }
            else
            {
                for (const auto v : equals)
                {
                    qDebug() << v.dim << " " << v.pos;
                }
                qDebug() << "Not over";
            }
        }
        else
        {
            m_hashList[0] = m_GameBoard4x4.getHash();
        }
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
    {
        m_GameBoard6x6x6.resetAndSeed(2);
        m_hashList[4] = m_GameBoard6x6x6.getHash();
    }
    else if (size == 8)
    {
        m_GameBoard8x8x8.resetAndSeed(2);
        m_hashList[5] = m_GameBoard8x8x8.getHash();
    }
    else
    {
        m_GameBoard4x4x4.resetAndSeed(2);
        m_hashList[3] = m_GameBoard4x4x4.getHash();
    }
}

// void Game2048::operate3D(const int size, const int dim, const MoveDirection dir)
// {
//     if (size == 6)
//         (void)m_GameBoard6x6x6.operateAndSpawn(dim, dir);
//     else if (size == 8)
//         (void)m_GameBoard8x8x8.operateAndSpawn(dim, dir);
//     else
//         (void)m_GameBoard4x4x4.operateAndSpawn(dim, dir);
// }

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

            emit updateGameScore(QVariantList{size, size, size}, me.newValue);

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

        if (m_hashList[4] == m_GameBoard6x6x6.getHash())
        {
            std::vector<EqualPair> equals{m_GameBoard6x6x6.checkOver()};
            if (equals.empty())
            {
                emit gameOver();
            }
            else
            {
                for (const auto v : equals)
                {
                    qDebug() << v.dim << " " << v.pos;
                }
                qDebug() << "Not over";
            }
        }
        else
        {
            m_hashList[4] = m_GameBoard6x6x6.getHash();
        }
    }
    else if (size == 8)
    {
        const auto trace = m_GameBoard8x8x8.operateAndSpawnTrace(dim, dir);

        const auto data = m_GameBoard8x8x8.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);

        fillCommon(trace);

        if (m_hashList[5] == m_GameBoard8x8x8.getHash())
        {
            std::vector<EqualPair> equals{m_GameBoard8x8x8.checkOver()};
            if (equals.empty())
            {
                emit gameOver();
            }
            else
            {
                for (const auto v : equals)
                {
                    qDebug() << v.dim << " " << v.pos;
                }
                qDebug() << "Not over";
            }
        }
        else
        {
            m_hashList[5] = m_GameBoard8x8x8.getHash();
        }
    }
    else
    {
        const auto trace = m_GameBoard4x4x4.operateAndSpawnTrace(dim, dir);

        const auto data = m_GameBoard4x4x4.flatData();
        flat.reserve(static_cast<int>(data.size()));
        for (const auto v : data)
            flat << static_cast<qulonglong>(v);

        fillCommon(trace);

        if (m_hashList[3] == m_GameBoard4x4x4.getHash())
        {
            std::vector<EqualPair> equals{m_GameBoard4x4x4.checkOver()};
            if (equals.empty())
            {
                emit gameOver();
            }
            else
            {
                for (const auto v : equals)
                {
                    qDebug() << v.dim << " " << v.pos;
                }
                qDebug() << "Not over";
            }
        }
        else
        {
            m_hashList[3] = m_GameBoard4x4x4.getHash();
        }
    }

    emit sendMoveTrace3D(gameMode.isEmpty() ? QStringLiteral("Static") : gameMode, outSize, flat, moves, merges, spawn);
}

GameData Game2048::getDataWithIndex(int index)
{
    const std::function processFunc{
        [](const std::vector<size_t> &data)
        {
            QList<std::uint64_t> qtData{};
            std::ranges::copy(data, std::back_inserter(qtData));
            return std::move(qtData);
        }};
    GameData data{};
    switch (index)
    {
    case 0:
    {
        data = {
            .gameSize = {4, 4},
            .currentScore = m_scoreMap[keyFor2DSize(4)].second,
            .maxScore = m_scoreMap[keyFor2DSize(4)].first,
            .flatTensorData = processFunc(m_GameBoard4x4.flatData())};
        break;
    }
    case 1:
    {
        data = {
            .gameSize = {6, 6},
            .currentScore = m_scoreMap[keyFor2DSize(6)].second,
            .maxScore = m_scoreMap[keyFor2DSize(6)].first,
            .flatTensorData = processFunc(m_GameBoard6x6.flatData())};
        break;
    }
    case 2:
    {
        data = {
            .gameSize = {8, 8},
            .currentScore = m_scoreMap[keyFor2DSize(8)].second,
            .maxScore = m_scoreMap[keyFor2DSize(8)].first,
            .flatTensorData = processFunc(m_GameBoard8x8.flatData())};
        break;
    }
    case 3:
    {
        data = {
            .gameSize = {4, 4, 4},
            .currentScore = m_scoreMap[keyFor3DSize(4)].second,
            .maxScore = m_scoreMap[keyFor3DSize(4)].first,
            .flatTensorData = processFunc(m_GameBoard4x4x4.flatData())};
        break;
    }
    case 4:
    {
        data = {
            .gameSize = {6, 6, 6},
            .currentScore = m_scoreMap[keyFor3DSize(6)].second,
            .maxScore = m_scoreMap[keyFor3DSize(6)].first,
            .flatTensorData = processFunc(m_GameBoard6x6x6.flatData())};
        break;
    }
    case 5:
    {
        data = {
            .gameSize = {8, 8, 8},
            .currentScore = m_scoreMap[keyFor3DSize(8)].second,
            .maxScore = m_scoreMap[keyFor3DSize(8)].first,
            .flatTensorData = processFunc(m_GameBoard8x8x8.flatData())};
        break;
    }
    default:
    {
        break;
    }
    }
    return data;
}

void Game2048::resetGame_emitted(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    reset2D(size);

    const QString scoreKey = keyFor2DSize(size);
    const int maxScore = m_scoreMap[scoreKey].first;
    m_scoreMap[scoreKey] = {maxScore, 0};

    emit2D(gameMode, size);
    emit sendScoreInfoToQML(QVariantList{m_scoreMap[scoreKey].first, m_scoreMap[scoreKey].second});
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

    const QString scoreKey = keyFor3DSize(size);
    const int maxScore = m_scoreMap[scoreKey].first;
    m_scoreMap[scoreKey] = {maxScore, 0};

    emit3D(gameMode, size);
    emit sendScoreInfoToQML(QVariantList{m_scoreMap[scoreKey].first, m_scoreMap[scoreKey].second});
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

void Game2048::gameScore_updated(const QVariantList &sizeInfo, const std::uint64_t addScore)
{
    auto sizes{sizeInfo.toList()};

    QString tempModeString{};
    if (sizes.size() == 2)
    {
        tempModeString = {QString{"%1x%2"}.arg(QString::number(sizes[0].toInt()), QString::number(sizes[1].toInt()))};
    }
    else if (sizes.size() == 3)
    {
        tempModeString = {
            QString{"%1x%2x%3"}.arg(
                QString::number(sizes[0].toInt()),
                QString::number(sizes[1].toInt()),
                QString::number(sizes[2].toInt()))};
    }
    auto [maxScore, currentScore]{m_scoreMap[tempModeString]};
    currentScore += addScore;
    if (currentScore > maxScore)
    {
        maxScore = currentScore;
    }
    m_scoreMap[tempModeString] = {maxScore, currentScore};

    sendScoreInfoToQML(QVariantList{maxScore, currentScore});
}

void Game2048::saveData_emitted(const int innerIndex)
{

    GameData data{getDataWithIndex(innerIndex)};
    std::ignore = m_dataManager->saveGame(data);
}

QVariantList Game2048::getScoreInfo_emitted(const int innerIndex)
{
    QString key{};
    switch (innerIndex)
    {
    case 0:
    {
        key = keyFor2DSize(4);
        break;
    }
    case 1:
    {
        key = keyFor2DSize(6);
        break;
    }
    case 2:
    {
        key = keyFor2DSize(8);
        break;
    }
    case 3:
    {
        key = keyFor3DSize(4);
        break;
    }
    case 4:
    {
        key = keyFor3DSize(6);
        break;
    }
    case 5:
    {
        key = keyFor3DSize(8);
        break;
    }
    default:
    {
        break;
    }
    }
    if (!key.isEmpty())
    {
        return QVariantList{m_scoreMap[key].first, m_scoreMap[key].second};
    }
    return QVariantList{};
}

QVariantList Game2048::getBoardData_emitted(const int innerIndex) const
{
    QVariantList boardData{};
    switch (innerIndex)
    {
    case 0:
    {
        auto data{m_GameBoard4x4.flatData()};
        std::ranges::copy(data, std::back_inserter(boardData));
        break;
    }
    case 1:
    {
        auto data{m_GameBoard6x6.flatData()};
        std::ranges::copy(data, std::back_inserter(boardData));
        break;
    }
    case 2:
    {
        auto data{m_GameBoard8x8.flatData()};
        std::ranges::copy(data, std::back_inserter(boardData));
        break;
    }
    case 3:
    {
        auto data{m_GameBoard4x4x4.flatData()};
        std::ranges::copy(data, std::back_inserter(boardData));
        break;
    }
    case 4:
    {
        auto data{m_GameBoard6x6x6.flatData()};
        std::ranges::copy(data, std::back_inserter(boardData));
        break;
    }
    case 5:
    {
        auto data{m_GameBoard8x8x8.flatData()};
        std::ranges::copy(data, std::back_inserter(boardData));
        break;
    }
    default:
        break;
    }
    return boardData;
}
