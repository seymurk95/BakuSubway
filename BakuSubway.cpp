#include "BakuSubway.h"
#include <random>
#include <chrono>
#include <thread>

// Структура для состояния поезда
struct TrainState {
    int passengers = 0;
    const int max_passengers = 500; // Максимальная вместимость
    bool alternate_route = false; // Для чередования маршрутов зелёной линии
    bool is_green_stop_at_bakmil = false; // Для остановки зелёного поезда на Bakmil через раз
};

// Map для расстояний между станциями
std::map<std::pair<std::string, std::string>, double, std::less<>> distances;

BakuSubway::BakuSubway() {
    // Красная линия (13 станций, 12 переходов)
    lines["Red"] = {
        {"Icheri Sheher", "Sahil", "28 May", "Ganjlik", "Nariman Narimanov",
         "Bakmil", "Ulduz", "Koroglu", "Kara Karaev", "Neftchilar",
         "Khalglar Dostlugu", "Ahmedli", "Azi Aslanov"},
        "Bakmil",
        std::vector<std::mutex>(12),
        std::vector<std::mutex>(12)
    };

    // Зеленая линия (совпадает с красной после 28 May до Azi Aslanov)
    lines["Green"] = {
        {"28 May", "Ganjlik", "Nariman Narimanov", "Bakmil", "Ulduz",
         "Koroglu", "Kara Karaev", "Neftchilar", "Khalglar Dostlugu",
         "Ahmedli", "Azi Aslanov"},
        "Bakmil",
        std::vector<std::mutex>(10),
        std::vector<std::mutex>(10)
    };

    // Фиолетовая линия (3 станции, 2 перехода)
    lines["Purple"] = {
        {"Khojasan", "Avtovagzal", "8 Noyabr"},
        "Khojasan",
        std::vector<std::mutex>(2),
        std::vector<std::mutex>(2)
    };

    // Светло-зеленая линия (2 станции, 1 переход), челнок без депо
    lines["Light Green"] = {
        {"Jafar Jabbarly", "Hatai"},
        "N/A",
        std::vector<std::mutex>(1),
        std::vector<std::mutex>(1),
        true
    };

    // Заполнение расстояний
    distances = {
        // Красная линия (до 28 May)
        {{"Icheri Sheher", "Sahil"}, 0.9}, {{"Sahil", "Icheri Sheher"}, 0.9},
        {{"Sahil", "28 May"}, 0.5}, {{"28 May", "Sahil"}, 0.5},
        // Общий туннель (красная и зелёная линии)
        {{"28 May", "Ganjlik"}, 1.6}, {{"Ganjlik", "28 May"}, 1.6},
        {{"Ganjlik", "Nariman Narimanov"}, 2.1}, {{"Nariman Narimanov", "Ganjlik"}, 2.1},
        {{"Nariman Narimanov", "Bakmil"}, 1.3}, {{"Bakmil", "Nariman Narimanov"}, 1.3},
        {{"Bakmil", "Ulduz"}, 1.9}, {{"Ulduz", "Bakmil"}, 1.9},
        {{"Ulduz", "Koroglu"}, 2.3}, {{"Koroglu", "Ulduz"}, 2.3},
        {{"Koroglu", "Kara Karaev"}, 1.8}, {{"Kara Karaev", "Koroglu"}, 1.8},
        {{"Kara Karaev", "Neftchilar"}, 1.2}, {{"Neftchilar", "Kara Karaev"}, 1.2},
        {{"Neftchilar", "Khalglar Dostlugu"}, 1.1}, {{"Khalglar Dostlugu", "Neftchilar"}, 1.1},
        {{"Khalglar Dostlugu", "Ahmedli"}, 1.6}, {{"Ahmedli", "Khalglar Dostlugu"}, 1.6},
        {{"Ahmedli", "Azi Aslanov"}, 1.3}, {{"Azi Aslanov", "Ahmedli"}, 1.3},
        // Зеленая линия (до 28 May)
        {{"28 May", "Jafar Jabbarly"}, 0.2}, {{"Jafar Jabbarly", "28 May"}, 0.2},
        {{"Jafar Jabbarly", "Nizami"}, 1.7}, {{"Nizami", "Jafar Jabbarly"}, 1.7},
        {{"Nizami", "Elmlar Akademiyasy"}, 1.2}, {{"Elmlar Akademiyasy", "Nizami"}, 1.2},
        {{"Elmlar Akademiyasy", "Inshaatchilar"}, 0.9}, {{"Inshaatchilar", "Elmlar Akademiyasy"}, 0.9},
        {{"Inshaatchilar", "20 January"}, 1.3}, {{"20 January", "Inshaatchilar"}, 1.3},
        {{"20 January", "Memar Ajami"}, 1.8}, {{"Memar Ajami", "20 January"}, 1.8},
        {{"Memar Ajami", "Nasimi"}, 2.66}, {{"Nasimi", "Memar Ajami"}, 2.66},
        {{"Nasimi", "Azadlig Prospekti"}, 2.1}, {{"Azadlig Prospekti", "Nasimi"}, 2.1},
        {{"Azadlig Prospekti", "Darnagul"}, 1.1}, {{"Darnagul", "Azadlig Prospekti"}, 1.1},
        // Фиолетовая линия
        {{"Khojasan", "Avtovagzal"}, 2.0}, {{"Avtovagzal", "Khojasan"}, 2.0},
        {{"Avtovagzal", "8 Noyabr"}, 1.5}, {{"8 Noyabr", "Avtovagzal"}, 1.5},
        // Светло-зеленая линия
        {{"Jafar Jabbarly", "Hatai"}, 1.0}, {{"Hatai", "Jafar Jabbarly"}, 1.0}
    };
}

// Функция расчёта времени движения с масштабированием
int calculate_travel_time(double distance) {
    const double speed_km_h = 40.0; // Скорость поезда в км/ч
    int real_time_seconds = static_cast<int>((distance / speed_km_h) * 3600); // Реальное время в секундах
    const double time_scale = 60.0; // Масштаб: 1 минута реального = 1 час в симуляции
    int sim_time_ms = static_cast<int>((real_time_seconds / time_scale) * 1000); // Масштабированное время в миллисекундах
    return std::max(500, sim_time_ms); // Минимум 500 мс (0.5 секунды)
}

void BakuSubway::safe_print(const std::string& message) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << message << std::endl;
}

void BakuSubway::train(int train_id, const std::string& line_name, int direction) {
    auto& line = lines[line_name];
    const auto& stations = line.stations;
    // Начальная станция — из депо
    int depot_index = -1;
    std::string depot = (line_name == "Red" || line_name == "Green") ? "Bakmil" :
                        (line_name == "Purple") ? "Khojasan" : stations[0];
    for (int i = 0; i < stations.size(); ++i) {
        if (stations[i] == depot) {
            depot_index = i;
            break;
        }
    }
    int current_station = depot_index != -1 ? depot_index : (direction ? stations.size() - 1 : 0);
    int step = direction ? -1 : 1;
    bool is_shuttle = line.is_shuttle;
    TrainState state;
    state.is_green_stop_at_bakmil = (train_id % 4 == 1); // Зелёный поезд с остановкой на Bakmil через раз

    // Популярность станций для пассажиров
    std::map<std::string, int> station_popularity = {
        {"Icheri Sheher", 100}, {"Sahil", 150}, {"28 May", 200}, {"Ganjlik", 80},
        {"Nariman Narimanov", 90}, {"Bakmil", 50}, {"Ulduz", 70}, {"Koroglu", 110},
        {"Kara Karaev", 80}, {"Neftchilar", 60}, {"Khalglar Dostlugu", 90},
        {"Ahmedli", 100}, {"Azi Aslanov", 70}, {"Jafar Jabbarly", 60},
        {"Nizami", 120}, {"Elmlar Akademiyasy", 90}, {"Inshaatchilar", 60},
        {"20 January", 70}, {"Memar Ajami", 130}, {"Nasimi", 80},
        {"Azadlig Prospekti", 50}, {"Darnagul", 40}, {"Khojasan", 30},
        {"Avtovagzal", 90}, {"8 Noyabr", 110}, {"Hatai", 40}
    };

    // Генератор случайных чисел для пассажиров
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> passenger_dis(0, 100);

    // Общие mutex'ы для станций
    static std::map<std::string, std::mutex> shared_station_mutexes;
    static std::mutex init_mutex;
    {
        std::lock_guard<std::mutex> lock(init_mutex);
        for (const auto& station : stations) {
            shared_station_mutexes.try_emplace(station);
        }
        shared_station_mutexes.try_emplace("Jafar Jabbarly");
    }

    // Сообщение о старте из депо
    safe_print("Train " + std::to_string(train_id) + " (" + line_name +
               ") started shift from depot " + (is_shuttle ? stations[current_station] : depot));

    // Время начала работы поезда
    auto start_time = std::chrono::steady_clock::now();
    const auto shift_duration = std::chrono::minutes(8); // 8 часов = 8 минут реального времени

    while (std::chrono::steady_clock::now() - start_time < shift_duration) {
        std::unique_lock<std::mutex> station_lock(shared_station_mutexes[stations[current_station]]);

        // Определяем следующую станцию
        std::string next_station;
        if (current_station + step >= 0 && current_station + step < static_cast<int>(stations.size())) {
            next_station = stations[current_station + step];
        } else if (line_name == "Green" && stations[current_station] == "Bakmil" && state.is_green_stop_at_bakmil) {
            next_station = "Ulduz";
            state.is_green_stop_at_bakmil = false;
        } else {
            next_station = "End of line";
        }

        // Прибытие с указанием следующей станции
        safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                   ") arrived at " + stations[current_station] +
                   ", next station: " + next_station);

        // Пассажиры выходят и заходят
        int passengers_out = std::min(state.passengers, passenger_dis(gen));
        int passengers_in = passenger_dis(gen) % station_popularity[stations[current_station]];
        state.passengers = std::min(state.max_passengers, state.passengers - passengers_out + passengers_in);
        safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                   "): " + std::to_string(passengers_out) + " out, " +
                   std::to_string(passengers_in) + " in, total: " + std::to_string(state.passengers));

        // Масштабированная остановка на станции (20–40 секунд в реальности → 0.33–0.67 секунды в симуляции)
        int stop_time_ms = (20 + (rand() % 21)) * 1000 / 60; // 333–666 мс
        std::this_thread::sleep_for(std::chrono::milliseconds(stop_time_ms));

        // Отправление с указанием следующей станции
        safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                   ") departed from " + stations[current_station] +
                   ", heading to: " + next_station);

        station_lock.unlock();

        // Движение к следующей станции
        if (current_station + step >= 0 && current_station + step < static_cast<int>(stations.size())) {
            std::string from = stations[current_station];
            std::string to = stations[current_station + step];
            auto key = std::make_pair(from, to);
            auto reverse_key = std::make_pair(to, from);
            double distance = distances.count(key) ? distances[key] : distances[reverse_key];
            int travel_time_ms = calculate_travel_time(distance);
            safe_print("Train " + std::to_string(train_id) + " traveling to " + to +
                       " (" + std::to_string(travel_time_ms / 1000.0) + "s)");
            std::this_thread::sleep_for(std::chrono::milliseconds(travel_time_ms));
        } else if (line_name == "Green" && stations[current_station] == "Bakmil" && state.is_green_stop_at_bakmil) {
            // Остановка на Bakmil (20 секунд в реальности → 333 мс в симуляции)
            std::this_thread::sleep_for(std::chrono::milliseconds(20 * 1000 / 60));
            current_station += step;
        }

        // Логика движения
        if (is_shuttle) {
            current_station += step;
            if (current_station < 0 || current_station >= static_cast<int>(stations.size())) {
                step = -step;
                current_station += 2 * step;
            }
        } else if (line_name == "Green" && stations[current_station] == "Bakmil" && state.is_green_stop_at_bakmil) {
            // Уже обработано выше
        } else {
            current_station += step;
            if (current_station < 0 || current_station >= static_cast<int>(stations.size())) {
                step = -step;
                current_station += 2 * step;
                if (line_name == "Green") state.is_green_stop_at_bakmil = !state.is_green_stop_at_bakmil;
            }
        }
    }

    // После 8 часов
    if (line_name == "Red" || line_name == "Green") {
        safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                   ") completed 8-hour shift and returned to depot Bakmil");
    } else if (line_name == "Purple") {
        safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                   ") completed 8-hour shift and returned to depot Khojasan");
    } else if (is_shuttle) {
        std::unique_lock<std::mutex> station_lock(shared_station_mutexes[stations[current_station]]);
        safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                   ") completed 8-hour shift and remained at " + stations[current_station]);
    }
}

void BakuSubway::run() {
    std::vector<std::thread> trains;
    trains.emplace_back(&BakuSubway::train, this, 1, "Green", 0);
    trains.emplace_back(&BakuSubway::train, this, 2, "Red", 0);
    trains.emplace_back(&BakuSubway::train, this, 3, "Green", 0);
    trains.emplace_back(&BakuSubway::train, this, 4, "Red", 1);
    trains.emplace_back(&BakuSubway::train, this, 5, "Purple", 0);
    trains.emplace_back(&BakuSubway::train, this, 6, "Purple", 1);
    trains.emplace_back(&BakuSubway::train, this, 7, "Light Green", 0);
    trains.emplace_back(&BakuSubway::train, this, 8, "Light Green", 1);

    for (auto& t : trains) t.join();
}
