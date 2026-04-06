//
// Created by jehor on 2026/4/3.
//

#ifndef GAME_2048_QUICK_GAME_DATA_MANAGER_H
#define GAME_2048_QUICK_GAME_DATA_MANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument> // 用于处理二维矩阵的序列化
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths> // 用于获取安全的文件存储路径
#include <QDir>
#include <QObject>
#include <QList>


struct GameData
{
    QList<std::uint64_t> gameSize{};
    int currentScore{};
    int maxScore{};
    QList<std::uint64_t> flatTensorData{};
};

class GameDataManager : public QObject
{
    Q_OBJECT
public:
    static GameDataManager* instance(); // 单例获取

    // 初始化数据库（调用一次）
    bool initDatabase();

    // 保存或更新游戏进度
    bool saveGame(const GameData &record) const;

    // 读取指定模式的进度
    GameData loadGame(const QList<uint64_t>& mode) const;

    // 更新最高分（如果当前分更高）
    void updateHighScore(const QList<uint64_t>& mode, int score) const;

private:
    explicit GameDataManager(QObject *parent = nullptr);
    QSqlDatabase m_db;

    // 辅助函数：将二维矩阵转为 JSON 字符串
    static QString arrayToJson(const QList<std::uint64_t> &grid);
    // 辅助函数：将 JSON 字符串还原为矩阵
    static QList<std::uint64_t> jsonToArray(const QString &jsonStr);

};
#endif //GAME_2048_QUICK_GAME_DATA_MANAGER_H
