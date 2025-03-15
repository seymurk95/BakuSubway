#include "BakuSubway.h"
#include <map>

// Структура для хранения информации о станции
struct StationMutexes {
    std::map<std::string, std::mutex> transition_mutexes; // mutex для переходов от станции
};

BakuSubway::BakuSubway() {
    // Инициализация общих mutex'ов для станций
    std::map<std::string, StationMutexes> station_mutexes;

    // Red Line (5 stations, 4 transitions)
    lines["Red"] = {
        {"Bakmil", "Nariman Narimanov", "28 May", "Sahil", "Icheri Sheher"},
        "Bakmil",
        std::vector<std::mutex>(4),
        std::vector<std::mutex>(4)
    };

    // Green Line (11 stations, 10 transitions)
    lines["Green"] = {
        {"Bakmil", "Nariman Narimanov", "28 May", "Nizami", "Elmlar Akademiyasy",
         "Inshaatchilar", "20 January", "Memar Ajami", "Nasimi", "Azadlig Prospekti", "Darnagul"},
        "Bakmil",
        std::vector<std::mutex>(10),
        std::vector<std::mutex>(10)
    };

    // Purple Line (4 stations, 3 transitions)
    lines["Purple"] = {
        {"Khojasan", "Avtovokzal", "Memar Ajami", "8 November"},
        "Khojasan",
        std::vector<std::mutex>(3),
        std::vector<std::mutex>(3)
    };

    // Light Green Line (2 stations, 1 transition)
    lines["Light Green"] = {
        {"Jafar Jabbarly", "Hatai"},
        "N/A",
        std::vector<std::mutex>(1),
        std::vector<std::mutex>(1),
        true
    };
}

void BakuSubway::safe_print(const std::string& message) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << message << std::endl;
}

void BakuSubway::train(int train_id, const std::string& line_name, int direction) {
    auto& line = lines[line_name];
    const auto& stations = line.stations;
    int current_station = direction ? stations.size() - 1 : 0;
    int step = direction ? -1 : 1;
    int laps = 0;
    bool is_shuttle = line.is_shuttle;

    // Создаем общий пул mutex'ов для общих станций
    static std::map<std::string, std::mutex> shared_station_mutexes;
    static std::mutex init_mutex;
    {
        std::lock_guard<std::mutex> lock(init_mutex);
        for (const auto& station : stations) {
            shared_station_mutexes.try_emplace(station);
        }
    }

    while (laps < 20) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Блокируем текущую станцию для всех линий
        std::unique_lock<std::mutex> station_lock(shared_station_mutexes[stations[current_station]]);

        // Блокируем переход только если есть следующая станция
        std::unique_lock<std::mutex> transition_lock;
        if (!is_shuttle && current_station + step >= 0 && current_station + step < static_cast<int>(stations.size())) {
            if (direction == 0 && current_station < static_cast<int>(line.forward_mutexes.size())) {
                transition_lock = std::unique_lock(line.forward_mutexes[current_station]);
            } else if (direction == 1 && current_station < static_cast<int>(line.backward_mutexes.size())) {
                transition_lock = std::unique_lock(line.backward_mutexes[current_station]);
            }
        } else if (is_shuttle && current_station >= 0 && current_station < static_cast<int>(line.forward_mutexes.size())) {
            transition_lock = std::unique_lock(line.forward_mutexes[current_station]);
        }

        safe_print("Train " + std::to_string(train_id) +
                  " (" + line_name + ") arrived at " + stations[current_station]);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        safe_print("Train " + std::to_string(train_id) +
                  " (" + line_name + ") departed from " + stations[current_station]);

        // Освобождаем станцию перед перемещением
        station_lock.unlock();

        // Логика движения
        if (is_shuttle) {
            current_station += step;
            if (current_station < 0 || current_station >= static_cast<int>(stations.size())) {
                step = -step;
                current_station += 2 * step;
                laps++;
            }
        } else {
            current_station += step;
            if (current_station < 0 || current_station >= static_cast<int>(stations.size())) {
                step = -step;
                current_station += 2 * step;
                laps++;
            }
        }
    }

    if (is_shuttle) {
        std::unique_lock<std::mutex> station_lock(shared_station_mutexes[stations[current_station]]);
        safe_print("Train " + std::to_string(train_id) +
                  " (" + line_name + ") remained at " + stations[current_station]);
    } else {
        safe_print("Train " + std::to_string(train_id) +
                  " (" + line_name + ") went to depot " + line.depot);
    }
}

void BakuSubway::run() {
    std::vector<std::thread> trains;

    trains.emplace_back(&BakuSubway::train, this, 1, "Red", 0);
    trains.emplace_back(&BakuSubway::train, this, 2, "Red", 1);

    trains.emplace_back(&BakuSubway::train, this, 3, "Green", 0);
    trains.emplace_back(&BakuSubway::train, this, 4, "Green", 1);

    trains.emplace_back(&BakuSubway::train, this, 5, "Purple", 0);
    trains.emplace_back(&BakuSubway::train, this, 6, "Purple", 1);

    trains.emplace_back(&BakuSubway::train, this, 7, "Light Green", 0);
    trains.emplace_back(&BakuSubway::train, this, 8, "Light Green", 1);

    for (auto& t : trains) t.join();
}
