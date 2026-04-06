//
// Created by jehor on 2026/4/6.
//
#pragma once
#include "game_2048.h"
#include <QDebug>
#include <chrono>

class AITest
{
public:

    void operator()() const
    {
        using namespace std::chrono;
        auto logic{Logic2048_tm<ArchCPU, size_t, size_t, 2, 8, 8>{}};

        const auto start = high_resolution_clock::now();

        logic.resetAndSeed();

        // int index {0};
        // while (index ++ <= 10000) {
        //     logic.operateBestMoveAndSpawnTrace2D(1);
        // }

        while (!logic.checkOver().empty()) {
            logic.operateBestMoveAndSpawnTrace2D(1);
        }



        const auto end = high_resolution_clock::now();
        const auto us = duration_cast<microseconds>(end - start).count();
        qDebug() << "执行时间：" << us << " 微秒\n";

        std::stringstream ss;
        logic.outputData(ss);

        qDebug() << ss.str().c_str();
        //
        // const auto vec{logic.checkOver()};
        // for (const auto& [pos, dim] : vec) {
        //     qDebug() << dim  << ": "<< pos;
        // }
    }


};