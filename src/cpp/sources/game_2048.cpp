//
// Created by jehor on 2026/3/31.
//

#include "game_2048.h"

#include <cmath>
#include <cfloat>

namespace
{
    struct AiSearchResult
    {
        int move{-1};
        float score{0.0f};
    };

    QString keyFor2DSize(const int size)
    {
        return QStringLiteral("%1x%1").arg(size);
    }

    QString keyFor3DSize(const int size)
    {
        return QStringLiteral("%1x%1x%1").arg(size);
    }

    inline std::size_t flatIndex(const int row, const int col, const int size)
    {
        return static_cast<std::size_t>(row * size + col);
    }

    float calculateEmpty(const std::vector<std::size_t> &data)
    {
        int empty = 0;
        for (const auto v : data)
        {
            if (v == 0)
                ++empty;
        }
        return static_cast<float>(empty);
    }

    float calculateMaxNum(const std::vector<std::size_t> &data)
    {
        std::size_t mx = 0;
        for (const auto v : data)
            if (v > mx)
                mx = v;
        return static_cast<float>(mx);
    }

    float calculateSmoothness(const std::vector<std::size_t> &data, const int size)
    {
        float smooth = 0.0f;
        for (int i = 0; i < size; ++i)
        {
            for (int j = 0; j < size; ++j)
            {
                const auto v = data[flatIndex(i, j, size)];
                if (v == 0)
                    continue;

                const float val = std::log2f(static_cast<float>(v) + 1.0f);
                if (i + 1 < size)
                {
                    const float v2 = std::log2f(static_cast<float>(data[flatIndex(i + 1, j, size)]) + 1.0f);
                    smooth -= std::fabs(v2 - val);
                }
                if (j + 1 < size)
                {
                    const float v2 = std::log2f(static_cast<float>(data[flatIndex(i, j + 1, size)]) + 1.0f);
                    smooth -= std::fabs(v2 - val);
                }
                if (i - 1 >= 0)
                {
                    const float v2 = std::log2f(static_cast<float>(data[flatIndex(i - 1, j, size)]) + 1.0f);
                    smooth -= std::fabs(v2 - val);
                }
                if (j - 1 >= 0)
                {
                    const float v2 = std::log2f(static_cast<float>(data[flatIndex(i, j - 1, size)]) + 1.0f);
                    smooth -= std::fabs(v2 - val);
                }
            }
        }
        return smooth;
    }

    float calculateMonotonicity(const std::vector<std::size_t> &data, const int size)
    {
        float totals[4] = {0, 0, 0, 0};

        for (int i = 0; i < size; ++i)
        {
            int current = 0;
            int next = current + 1;
            while (next < size)
            {
                while (next < size && data[flatIndex(i, next, size)] == 0)
                    ++next;
                if (next >= size)
                    --next;

                const auto currentRaw = data[flatIndex(i, current, size)];
                const auto nextRaw = data[flatIndex(i, next, size)];
                const float currentVal = (currentRaw != 0) ? std::log2f(static_cast<float>(currentRaw)) : 0.0f;
                const float nextVal = (nextRaw != 0) ? std::log2f(static_cast<float>(nextRaw)) : 0.0f;

                if (currentVal > nextVal)
                    totals[0] += nextVal - currentVal;
                else
                    totals[1] += currentVal - nextVal;

                current = next;
                ++next;
            }
        }

        for (int j = 0; j < size; ++j)
        {
            int current = 0;
            int next = current + 1;
            while (next < size)
            {
                while (next < size && data[flatIndex(next, j, size)] == 0)
                    ++next;
                if (next >= size)
                    --next;

                const auto currentRaw = data[flatIndex(current, j, size)];
                const auto nextRaw = data[flatIndex(next, j, size)];
                const float currentVal = (currentRaw != 0) ? std::log2f(static_cast<float>(currentRaw)) : 0.0f;
                const float nextVal = (nextRaw != 0) ? std::log2f(static_cast<float>(nextRaw)) : 0.0f;

                if (currentVal > nextVal)
                    totals[2] += nextVal - currentVal;
                else
                    totals[3] += currentVal - nextVal;

                current = next;
                ++next;
            }
        }

        const float max1 = (totals[0] > totals[1]) ? totals[0] : totals[1];
        const float max2 = (totals[2] > totals[3]) ? totals[2] : totals[3];
        return max1 + max2;
    }

    void markIsland(const std::vector<std::size_t> &data,
                    const int size,
                    const int x,
                    const int y,
                    const std::size_t value,
                    std::vector<std::uint8_t> &marked)
    {
        if (x < 0 || x >= size || y < 0 || y >= size)
            return;
        const auto idx = flatIndex(x, y, size);
        if (data[idx] == 0 || data[idx] != value || marked[idx] != 0)
            return;

        marked[idx] = 1;
        markIsland(data, size, x + 1, y, value, marked);
        markIsland(data, size, x - 1, y, value, marked);
        markIsland(data, size, x, y + 1, value, marked);
        markIsland(data, size, x, y - 1, value, marked);
    }

    float calculateIslands(const std::vector<std::size_t> &data, const int size)
    {
        std::vector<std::uint8_t> marked(static_cast<std::size_t>(size * size), 1);
        for (int i = 0; i < size; ++i)
        {
            for (int j = 0; j < size; ++j)
            {
                const auto idx = flatIndex(i, j, size);
                if (data[idx] != 0)
                    marked[idx] = 0;
            }
        }

        float islands = 0.0f;
        for (int i = 0; i < size; ++i)
        {
            for (int j = 0; j < size; ++j)
            {
                const auto idx = flatIndex(i, j, size);
                if (data[idx] != 0 && marked[idx] == 0)
                {
                    islands += 1.0f;
                    markIsland(data, size, i, j, data[idx], marked);
                }
            }
        }
        return islands;
    }

    float calculateEvaluation(const std::vector<std::size_t> &data, const int size)
    {
        const float empty = calculateEmpty(data);
        const float emptyWeight = 2.7f + (std::logf(17.0f) - std::logf(empty + 1.0f)) * 0.1f;
        const float maxnumWeight = 1.0f;
        const float smoothWeight = 0.1f;
        const float monoWeight = 1.0f;

        return emptyWeight * std::logf(empty + 1.0f) +
               maxnumWeight * calculateMaxNum(data) +
               smoothWeight * calculateSmoothness(data, size) +
               monoWeight * calculateMonotonicity(data, size);
    }

    bool mapMoveToLogic(const int move, std::size_t &dim, MoveDirection &dir)
    {
        switch (move)
        {
        case 0: // left
            dim = 1;
            dir = MoveDirection::Negative;
            return true;
        case 1: // up
            dim = 0;
            dir = MoveDirection::Negative;
            return true;
        case 2: // right
            dim = 1;
            dir = MoveDirection::Positive;
            return true;
        case 3: // down
            dim = 0;
            dir = MoveDirection::Positive;
            return true;
        default:
            return false;
        }
    }

    template <typename BoardT>
    bool applyMoveNoSpawn(BoardT &board, const int move)
    {
        std::size_t dim = 0;
        MoveDirection dir = MoveDirection::Negative;
        if (!mapMoveToLogic(move, dim, dir))
            return false;

        const auto before = board.getHash();
        board.operate(dim, dir);
        return before != board.getHash();
    }

    template <typename BoardT>
    bool setFlatCell(BoardT &board, const std::size_t index, const std::size_t value)
    {
        auto data = board.flatData();
        if (index >= data.size() || data[index] != 0)
            return false;
        data[index] = value;
        board.template setData<std::size_t, std::vector>(data);
        return true;
    }

    template <typename BoardT>
    std::vector<std::size_t> freeCells(const BoardT &board)
    {
        const auto data = board.flatData();
        std::vector<std::size_t> out;
        out.reserve(data.size());
        for (std::size_t i = 0; i < data.size(); ++i)
        {
            if (data[i] == 0)
                out.push_back(i);
        }
        return out;
    }

    template <typename BoardT>
    AiSearchResult searchBest(BoardT board, const int depth, const float alpha, const float beta, const bool playerTurn, const int size)
    {
        AiSearchResult result;

        if (playerTurn)
        {
            float bestScore = alpha;
            int bestMove = -1;

            for (int direction = 0; direction < 4; ++direction)
            {
                BoardT next = board;
                const bool changed = applyMoveNoSpawn(next, direction);
                if (!changed)
                    continue;

                AiSearchResult sub;
                if (depth == 0)
                {
                    sub.move = direction;
                    sub.score = calculateEvaluation(next.flatData(), size);
                }
                else
                {
                    sub = searchBest(next, depth - 1, bestScore, beta, false, size);
                }

                if (sub.score > bestScore)
                {
                    bestScore = sub.score;
                    bestMove = direction;
                }

                if (bestScore > beta)
                {
                    result.move = bestMove;
                    result.score = beta;
                    return result;
                }
            }

            result.move = bestMove;
            result.score = bestScore;
            return result;
        }

        float bestScore = beta;
        const auto empty = freeCells(board);
        if (empty.empty())
        {
            result.move = -1;
            result.score = calculateEvaluation(board.flatData(), size);
            return result;
        }

        std::vector<float> score2(empty.size(), -FLT_MAX);
        std::vector<float> score4(empty.size(), -FLT_MAX);
        for (std::size_t i = 0; i < empty.size(); ++i)
        {
            BoardT t2 = board;
            if (setFlatCell(t2, empty[i], 2))
                score2[i] = -calculateSmoothness(t2.flatData(), size) + calculateIslands(t2.flatData(), size);

            BoardT t4 = board;
            if (setFlatCell(t4, empty[i], 4))
                score4[i] = -calculateSmoothness(t4.flatData(), size) + calculateIslands(t4.flatData(), size);
        }

        float maxScore = -FLT_MAX;
        for (std::size_t i = 0; i < empty.size(); ++i)
        {
            if (score2[i] > maxScore)
                maxScore = score2[i];
            if (score4[i] > maxScore)
                maxScore = score4[i];
        }

        struct WorstCase
        {
            std::size_t idx;
            std::size_t value;
        };
        std::vector<WorstCase> worst;
        worst.reserve(empty.size() * 2);
        for (std::size_t i = 0; i < empty.size(); ++i)
        {
            if (score2[i] == maxScore)
                worst.push_back({empty[i], 2});
            if (score4[i] == maxScore)
                worst.push_back({empty[i], 4});
        }

        for (const auto &w : worst)
        {
            BoardT next = board;
            if (!setFlatCell(next, w.idx, w.value))
                continue;

            const auto sub = searchBest(next, depth, alpha, bestScore, true, size);
            if (sub.score < bestScore)
                bestScore = sub.score;

            if (bestScore < alpha)
            {
                result.move = -1;
                result.score = alpha;
                return result;
            }
        }

        result.move = -1;
        result.score = bestScore;
        return result;
    }

    inline int chooseDepth(const int size)
    {
        switch (size)
        {
        case 4:
            return 4;
        case 6:
            return 1;
        case 8:
            return 1;
        default:
            return 1;
        }
    }

    template <typename BoardT>
    int getBestMove(BoardT board, const int size)
    {
        int depth = chooseDepth(size);
        auto result = searchBest(board, depth, -1000000.0f, 1000000.0f, true, size);

        while (depth > 0)
        {
            if (result.move == -1)
                result = searchBest(board, --depth, -1000000.0f, 1000000.0f, true, size);
            else
                break;
        }

        if (result.move != -1)
        {
            BoardT test = board;
            if (applyMoveNoSpawn(test, result.move))
                return result.move;
        }

        for (int d = 0; d < 4; ++d)
        {
            BoardT test = board;
            if (applyMoveNoSpawn(test, d))
                return d;
        }
        return -1;
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

void Game2048::aiStep2D_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    const int size = parse2DSize(sizeInfo);
    int move = -1;

    if (size == 6)
        move = getBestMove(m_GameBoard6x6, size);
    else if (size == 8)
        move = getBestMove(m_GameBoard8x8, size);
    else
        move = getBestMove(m_GameBoard4x4, size);

    std::size_t dim = 0;
    MoveDirection dir = MoveDirection::Negative;
    if (!mapMoveToLogic(move, dim, dir))
    {
        emit gameOver();
        return;
    }

    operate2DAndEmitTrace(gameMode, size, static_cast<int>(dim), dir);
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
