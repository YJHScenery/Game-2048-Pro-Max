//
// Created by jehor on 2026/3/31.
//

#include "game_2048.h"

#include <QRandomGenerator>
#include <QtGlobal>

Game2048::Game2048(QObject *parent) : QObject(parent)
{
    m_board2D.resize(m_rows2D * m_cols2D);
    reset2DBoard();
}

bool Game2048::ensure2DBoard(const QString &gameMode, const QVariantList &sizeInfo)
{
    m_currentGameMode = gameMode.isEmpty() ? QStringLiteral("Static") : gameMode;

    int newRows = m_rows2D;
    int newCols = m_cols2D;

    if (sizeInfo.size() >= 2)
    {
        newRows = qMax(1, sizeInfo.at(0).toInt());
        newCols = qMax(1, sizeInfo.at(1).toInt());
    }

    if (newRows != m_rows2D || newCols != m_cols2D || m_board2D.size() != newRows * newCols)
    {
        m_rows2D = newRows;
        m_cols2D = newCols;
        m_board2D = QVector<size_t>(m_rows2D * m_cols2D, 0);
        return true;
    }
    return false;
}

void Game2048::reset2DBoard()
{
    std::fill(m_board2D.begin(), m_board2D.end(), 0);
    addRandomTile2D();
    addRandomTile2D();
}

static QVector<size_t> compressAndMergeLine(const QVector<size_t> &line)
{
    QVector<size_t> nonZero;
    nonZero.reserve(line.size());
    for (const auto v : line)
    {
        if (v != 0)
            nonZero.push_back(v);
    }

    QVector<size_t> merged;
    merged.reserve(line.size());
    for (int i = 0; i < nonZero.size(); ++i)
    {
        if (i + 1 < nonZero.size() && nonZero[i] == nonZero[i + 1])
        {
            merged.push_back(nonZero[i] * 2);
            ++i;
        }
        else
        {
            merged.push_back(nonZero[i]);
        }
    }

    while (merged.size() < line.size())
        merged.push_back(0);
    return merged;
}

bool Game2048::move2DLeft()
{
    bool changed = false;
    for (int r = 0; r < m_rows2D; ++r)
    {
        QVector<size_t> line;
        line.reserve(m_cols2D);
        for (int c = 0; c < m_cols2D; ++c)
        {
            line.push_back(m_board2D[r * m_cols2D + c]);
        }
        const auto newLine = compressAndMergeLine(line);
        if (newLine != line)
            changed = true;
        for (int c = 0; c < m_cols2D; ++c)
        {
            m_board2D[r * m_cols2D + c] = newLine[c];
        }
    }
    return changed;
}

bool Game2048::move2DRight()
{
    bool changed = false;
    for (int r = 0; r < m_rows2D; ++r)
    {
        QVector<size_t> line;
        line.reserve(m_cols2D);
        for (int c = m_cols2D - 1; c >= 0; --c)
        {
            line.push_back(m_board2D[r * m_cols2D + c]);
        }
        const auto newLine = compressAndMergeLine(line);
        if (newLine != line)
            changed = true;
        for (int c = m_cols2D - 1, i = 0; c >= 0; --c, ++i)
        {
            m_board2D[r * m_cols2D + c] = newLine[i];
        }
    }
    return changed;
}

bool Game2048::move2DUp()
{
    bool changed = false;
    for (int c = 0; c < m_cols2D; ++c)
    {
        QVector<size_t> line;
        line.reserve(m_rows2D);
        for (int r = 0; r < m_rows2D; ++r)
        {
            line.push_back(m_board2D[r * m_cols2D + c]);
        }
        const auto newLine = compressAndMergeLine(line);
        if (newLine != line)
            changed = true;
        for (int r = 0; r < m_rows2D; ++r)
        {
            m_board2D[r * m_cols2D + c] = newLine[r];
        }
    }
    return changed;
}

bool Game2048::move2DDown()
{
    bool changed = false;
    for (int c = 0; c < m_cols2D; ++c)
    {
        QVector<size_t> line;
        line.reserve(m_rows2D);
        for (int r = m_rows2D - 1; r >= 0; --r)
        {
            line.push_back(m_board2D[r * m_cols2D + c]);
        }
        const auto newLine = compressAndMergeLine(line);
        if (newLine != line)
            changed = true;
        for (int r = m_rows2D - 1, i = 0; r >= 0; --r, ++i)
        {
            m_board2D[r * m_cols2D + c] = newLine[i];
        }
    }
    return changed;
}

void Game2048::addRandomTile2D()
{
    QVector<int> empties;
    empties.reserve(m_board2D.size());
    for (int i = 0; i < m_board2D.size(); ++i)
    {
        if (m_board2D[i] == 0)
            empties.push_back(i);
    }
    if (empties.isEmpty())
        return;

    const int pickIdx = QRandomGenerator::global()->bounded(empties.size());
    const int cellIndex = empties[pickIdx];
    const int roll = QRandomGenerator::global()->bounded(10); // 0..9
    m_board2D[cellIndex] = (roll == 0) ? 4 : 2;               // 10% 4, 90% 2
}

void Game2048::emit2D()
{
    QVariantList sizeInfo;
    sizeInfo.reserve(2);
    sizeInfo << m_rows2D << m_cols2D;

    QVariantList flat;
    flat.reserve(m_board2D.size());
    for (const auto v : m_board2D)
    {
        flat << static_cast<qulonglong>(v);
    }

    emit sendGameData(m_currentGameMode, sizeInfo, flat);
}

void Game2048::on_ResetGame_emitted(const QString &gameMode, const QVariantList &sizeInfo)
{
    ensure2DBoard(gameMode, sizeInfo);
    reset2DBoard();
    emit2D();
}

void Game2048::on_Up_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    ensure2DBoard(gameMode, sizeInfo);
    if (move2DUp())
        addRandomTile2D();
    emit2D();
}

void Game2048::on_Down_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    ensure2DBoard(gameMode, sizeInfo);
    if (move2DDown())
        addRandomTile2D();
    emit2D();
}

void Game2048::on_Left_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    ensure2DBoard(gameMode, sizeInfo);
    if (move2DLeft())
        addRandomTile2D();
    emit2D();
}

void Game2048::on_Right_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    ensure2DBoard(gameMode, sizeInfo);
    if (move2DRight())
        addRandomTile2D();
    emit2D();
}

void Game2048::on_Forward_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    Q_UNUSED(sizeInfo)
    m_currentGameMode = gameMode.isEmpty() ? QStringLiteral("Static") : gameMode;
    emit2D();
}

void Game2048::on_Back_operated(const QString &gameMode, const QVariantList &sizeInfo)
{
    Q_UNUSED(sizeInfo)
    m_currentGameMode = gameMode.isEmpty() ? QStringLiteral("Static") : gameMode;
    emit2D();
}
