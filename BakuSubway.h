#ifndef BAKUSUBWAY_H
#define BAKUSUBWAY_H

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <memory>

class BakuSubway {
public:
    BakuSubway();
    void run();

private:
    std::vector<std::string> stations;
    std::vector<std::unique_ptr<std::mutex>> stationMutexes;
    std::mutex coutMutex; // Мьютекс для защиты std::cout

    void train(int trainId, int direction);
    void safePrint(const std::string& message); // Потокобезопасный вывод
};

#endif // BAKUSUBWAY_H