#include "BakuSubway.h"

BakuSubway::BakuSubway() {
    stations={"Bakmill","Nariman Narimanov","Ganjlik","28 May","Sahil","Icheri Seher"};
    stationMutexes.resize(stations.size());
    for (auto& mutexPtr : stationMutexes) {
        mutexPtr = std::make_unique<std::mutex>();
    }
}

void BakuSubway::safePrint(const std::string& message) {
    std::lock_guard<std::mutex> lock(coutMutex); // Защищаем доступ к std::cout
    std::cout << message << std::endl;
}

void BakuSubway::train(int trainId, int direction) {
    int currentStation = 0;
    int step = 1;

    if (direction == 1) {
        currentStation = stations.size() - 1;
        step = -1;
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::unique_lock<std::mutex> lock(*stationMutexes[currentStation]);

        safePrint("Train " + std::to_string(trainId) + " Arrived on " + stations[currentStation]);

        std::this_thread::sleep_for(std::chrono::seconds(2));

        safePrint("Train " + std::to_string(trainId) + " Leaved " + stations[currentStation]);

        lock.unlock();


        if (currentStation == 0 && step == -1) {
            step = 1;
        } else if (currentStation == stations.size() - 1 && step == 1) {
            step = -1;
        }

        currentStation += step;

        if (currentStation < 0 || currentStation >= stations.size()) {
            break;
        }
    }
}

void BakuSubway::run() {

    std::thread train1(&BakuSubway::train, this, 1, 0);
    std::thread train2(&BakuSubway::train, this, 2, 1);
    std::thread train3(&BakuSubway::train, this, 3, 0);
    std::thread train4(&BakuSubway::train, this, 4, 1); 


    train1.join();
    train2.join();
    train3.join();
    train4.join();
}