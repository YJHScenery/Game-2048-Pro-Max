//
// Created by jehor on 2026/4/3.
//
#include "game_data_manager.h"

GameDataManager::GameDataManager(QObject *parent) : QObject(parent) {
    // 构造函数中不直接打开数据库，建议在 initDatabase 中处理

    qDebug() << "Game Data Manager was Created. Please call function GameDataManager::initDatabase() to initialize Database";
}

QString GameDataManager::arrayToJson(const QList<std::uint64_t>& grid)
{
    QJsonArray array;
    for (const auto &row : grid) {
        array.append(static_cast<qint64>(row));
    }
    return QJsonDocument(array).toJson(QJsonDocument::Compact);
}

QList<std::uint64_t> GameDataManager::jsonToArray(const QString& jsonStr)
{
    QList<std::uint64_t> grid;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());

    if (doc.isArray()) {
        for (const auto &val : doc.array()) {
            grid.append(val.toInt());
        }
    }
    return grid;
}

GameDataManager* GameDataManager::instance()
{
    static GameDataManager instance;
    return &instance;
}

bool GameDataManager::initDatabase() {
    // 1. 确定数据库路径 (使用用户数据目录，避免权限问题)
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    std::ignore = QDir().mkpath(dbPath); // 确保目录存在
    dbPath += "/game2048_data.db";

    // 2. 添加数据库驱动
    m_db = QSqlDatabase::addDatabase("QSQLITE", "GameConnection"); // 指定连接名
    m_db.setDatabaseName(dbPath);

    // 3. 打开连接
    if (!m_db.open()) {
        qDebug() << "Failed to open DataBase:" << m_db.lastError().text();
        return false;
    }

    // 4. 创建表 (如果不存在)
    QSqlQuery query(m_db);
    // 表结构：模式(主键), 棋盘数据(JSON), 当前分, 最高分
    static constexpr char createTableSQL[] = R"(
        CREATE TABLE IF NOT EXISTS saves (
            mode TEXT PRIMARY KEY NOT NULL,
            grid_data TEXT,
            current_score INTEGER DEFAULT 0,
            high_score INTEGER DEFAULT 0
        )
    )";
    
    if (!query.exec(createTableSQL)) {
        qDebug() << "Failed to create table:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Database initialized successfully";
    return true;
}

bool GameDataManager::saveGame(const GameData &record) const
{
    QSqlQuery query(m_db);
    
    // 使用预处理语句防止 SQL 注入，并提高性能
    // 逻辑：插入或替换 (INSERT OR REPLACE)
    static constexpr char sql[] = R"(
        INSERT OR REPLACE INTO saves (mode, grid_data, current_score, high_score)
        VALUES (:mode, :grid, :score, :high)
    )";
    
    if (query.prepare(sql)) {
        query.bindValue(":mode", arrayToJson(record.gameSize));
        query.bindValue(":grid", arrayToJson(record.flatTensorData)); // 序列化矩阵
        query.bindValue(":score", record.currentScore);
        query.bindValue(":high", record.maxScore);
        
        return query.exec();
    }
    return false;
}

GameData GameDataManager::loadGame(const QList<uint64_t>& mode) const
{
    GameData record;
    record.gameSize = mode;
    record.currentScore = 0;
    record.maxScore = 0;
    // 初始化为空棋盘
    record.flatTensorData = QList<std::uint64_t>(4, 0);

    QSqlQuery query(m_db);
    query.prepare("SELECT grid_data, current_score, high_score FROM saves WHERE mode = :mode");
    query.bindValue(":mode", arrayToJson(mode));
    
    if (query.exec() && query.next()) {
        record.flatTensorData = jsonToArray(query.value("grid_data").toString());
        record.currentScore = query.value("current_score").toInt();
        record.maxScore = query.value("high_score").toInt();
    }
    
    return record;
}

void GameDataManager::updateHighScore(const QList<uint64_t>& mode, const int score) const
{
    // 先读取旧的最高分
    GameData current = loadGame(mode);
    if (score > current.maxScore) {
        current.maxScore = score;
        current.currentScore = score; // 假设更新最高分同时也更新当前分
        std::ignore = saveGame(current);
    }
}
