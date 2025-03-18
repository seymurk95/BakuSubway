#include "BakuSubway.h"
#include <random>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>

// Счётчики и мьютексы
static long long total_passengers = 0;
static int current_passengers = 0;
static std::mutex passenger_mutex;
static double total_fuel_cost = 0.0;
static std::mutex fuel_cost_mutex;

struct TrainState {
    int passengers = 0;
    const int max_passengers = 1600;
    bool alternate_route = false;
    bool is_green_stop_at_bakmil = false;
    double total_distance = 0.0;
};

std::map<std::pair<std::string, std::string>, double, std::less<>> distances;

void clearConsole() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void displayWelcomeAnimation() {
    const char* trainArt[] = {
        "  +----------------+  ",
        "  |  Baku Metro    |  ",
        "  |  [] [] [] []   |->",
        "  +----------------+  ",
        "    o    o    o    o  "
    };

    clearConsole();
    std::cout << "\n\n\n";
    std::cout << "    Welcome to Baku Subway Simulation\n\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));



    clearConsole();
    std::cout << "\n\n";
    for (const char* line : trainArt) {
        std::cout << "    " << line << "\n";
    }
    std::cout << "\n    Starting simulation...\n\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    clearConsole();
}

BakuSubway::BakuSubway() {
    // Красная линия
    lines["Red"] = {
        {"Icheri Sheher", "Sahil", "28 May", "Ganjlik", "Nariman Narimanov",
         "Bakmil", "Ulduz", "Koroglu", "Kara Karaev", "Neftchilar",
         "Khalglar Dostlugu", "Ahmedli", "Azi Aslanov"},
        "Bakmil",
        std::vector<std::mutex>(12),
        std::vector<std::mutex>(12)
    };

    // Зеленая линия
    lines["Green"] = {
        {"Darnagul", "Azadlig Prospekti", "Nasimi", "Memar Ajami", "20 January",
         "Inshaatchilar", "Elmlar Akademiyasy", "Nizami", "28 May",
         "Ganjlik", "Nariman Narimanov", "Bakmil", "Ulduz", "Koroglu",
         "Kara Karaev", "Neftchilar", "Khalglar Dostlugu", "Ahmedli", "Azi Aslanov"},
        "Bakmil",
        std::vector<std::mutex>(18),
        std::vector<std::mutex>(18)
    };

    // Фиолетовая линия
    lines["Purple"] = {
        {"Khojasan", "Avtovagzal", "8 Noyabr"},
        "Khojasan",
        std::vector<std::mutex>(2),
        std::vector<std::mutex>(2)
    };

    // Светло-зеленая линия
    lines["Light Green"] = {
        {"Jafar Jabbarly", "Hatai"},
        "N/A",
        std::vector<std::mutex>(1),
        std::vector<std::mutex>(1),
        true
    };

    // Расстояния
    distances = {
        // Красная линия (до 28 May)
        {{"Icheri Sheher", "Sahil"}, 0.9}, {{"Sahil", "Icheri Sheher"}, 0.9},
        {{"Sahil", "28 May"}, 0.5}, {{"28 May", "Sahil"}, 0.5},
        // Общий туннель (красная и зелёная линии после 28 May)
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
        // Зеленая линия (до 28 May и далее до Дарнагюля)
        {{"Nizami", "Elmlar Akademiyasy"}, 1.2}, {{"Elmlar Akademiyasy", "Nizami"}, 1.2},
        {{"Elmlar Akademiyasy", "Inshaatchilar"}, 0.9}, {{"Inshaatchilar", "Elmlar Akademiyasy"}, 0.9},
        {{"Inshaatchilar", "20 January"}, 1.3}, {{"20 January", "Inshaatchilar"}, 1.3},
        {{"20 January", "Memar Ajami"}, 1.8}, {{"Memar Ajami", "20 January"}, 1.8},
        {{"Memar Ajami", "Nasimi"}, 2.66}, {{"Nasimi", "Memar Ajami"}, 2.66},
        {{"Nasimi", "Azadlig Prospekti"}, 2.1}, {{"Azadlig Prospekti", "Nasimi"}, 2.1},
        {{"Azadlig Prospekti", "Darnagul"}, 1.1}, {{"Darnagul", "Azadlig Prospekti"}, 1.1},
        {{"28 May", "Nizami"}, 1.7}, {{"Nizami", "28 May"}, 1.7},
        // Фиолетовая линия
        {{"Khojasan", "Avtovagzal"}, 2.0}, {{"Avtovagzal", "Khojasan"}, 2.0},
        {{"Avtovagzal", "8 Noyabr"}, 1.5}, {{"8 Noyabr", "Avtovagzal"}, 1.5},
        // Светло-зеленая линия
        {{"Jafar Jabbarly", "Hatai"}, 1.0}, {{"Hatai", "Jafar Jabbarly"}, 1.0}
    };
}

bool is_peak_hour() {
    auto now = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(now);
    struct tm* ptm = localtime(&tt);
    int hour = ptm->tm_hour;
    return (hour >= 7 && hour < 9) || (hour >= 17 && hour < 19);
}

int calculate_travel_time(double distance) {
    const double speed_km_h = 40.0;
    int real_time_seconds = static_cast<int>((distance / speed_km_h) * 3600);
    const double time_scale = 120.0;
    int sim_time_ms = static_cast<int>((real_time_seconds / time_scale) * 1000);
    return std::max(250, sim_time_ms);
}

void BakuSubway::safe_print(const std::string& message) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << message << std::endl;
}

void BakuSubway::train(int train_id, const std::string& line_name, int direction) {
    auto& line = lines[line_name];
    const auto& stations = line.stations;
    int depot_index = -1;
    std::string depot = (line_name == "Red" || line_name == "Green") ? "Bakmil" :
                        (line_name == "Purple") ? "Khojasan" : stations[0];
    for (int i = 0; i < stations.size(); ++i) {
        if (stations[i] == depot) depot_index = i;
    }
    int current_station = depot_index != -1 ? depot_index : (direction ? stations.size() - 1 : 0);
    int step = direction ? -1 : 1;
    bool is_shuttle = line.is_shuttle;
    TrainState state;

    std::map<std::string, int> station_popularity = {
        {"Icheri Sheher", 300}, {"Sahil", 250}, {"28 May", 400}, {"Ganjlik", 200},
        {"Nariman Narimanov", 220}, {"Bakmil", 100}, {"Ulduz", 150}, {"Koroglu", 250},
        {"Kara Karaev", 180}, {"Neftchilar", 150}, {"Khalglar Dostlugu", 200}, {"Ahmedli", 220},
        {"Azi Aslanov", 180}, {"Jafar Jabbarly", 200}, {"Hatai", 100},
        {"Khojasan", 80}, {"Avtovagzal", 180}, {"8 Noyabr", 220},
        {"Nizami", 250}, {"Elmlar Akademiyasy", 200}, {"Inshaatchilar", 180},
        {"20 January", 220}, {"Memar Ajami", 230}, {"Nasimi", 210},
        {"Azadlig Prospekti", 190}, {"Darnagul", 170}
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> passenger_dis(0, 100);
    std::uniform_real_distribution<> breakdown_prob(0.0, 1.0);

    static std::map<std::string, std::mutex> shared_station_mutexes;
    static std::mutex init_mutex;
    {
        std::lock_guard<std::mutex> lock(init_mutex);
        for (const auto& station : stations) shared_station_mutexes.try_emplace(station);
        shared_station_mutexes.try_emplace("Jafar Jabbarly");
    }

    auto metro_start = std::chrono::steady_clock::now();
    const auto metro_duration = std::chrono::minutes(10);
    const auto shift_duration = std::chrono::minutes(5);

    const double fuel_cost_per_km = 0.1;
    double breakdown_cost = 0.0;

    int driver_shift = 1;
    safe_print("Train " + std::to_string(train_id) + " (" + line_name +
               ") departing from depot " + depot + " with 0 passengers");

    while (std::chrono::steady_clock::now() - metro_start < metro_duration) {
        auto shift_start = std::chrono::steady_clock::now();
        safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                   ") started shift " + std::to_string(driver_shift));

        while (std::chrono::steady_clock::now() - shift_start < shift_duration &&
               std::chrono::steady_clock::now() - metro_start < metro_duration) {
            std::unique_lock<std::mutex> station_lock(shared_station_mutexes[stations[current_station]]);

            std::string next_station;
            if (current_station + step >= 0 && current_station + step < static_cast<int>(stations.size())) {
                next_station = stations[current_station + step];
            } else {
                next_station = "End of line";
            }

            safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                       ") arrived at " + stations[current_station] +
                       ", next station: " + next_station);

            int passengers_out = std::min(state.passengers, passenger_dis(gen));
            int passengers_in = passenger_dis(gen) % station_popularity[stations[current_station]];
            passengers_in = is_peak_hour() ? passengers_in * 2 : passengers_in;

            {
                std::lock_guard<std::mutex> lock(passenger_mutex);
                int available_capacity = 14400 - current_passengers;
                passengers_in = std::min(passengers_in, available_capacity);
                state.passengers = std::min(state.max_passengers, state.passengers - passengers_out + passengers_in);
                current_passengers = current_passengers - passengers_out + passengers_in;
                if (current_passengers < 0) current_passengers = 0;
                if (passengers_in > 0) total_passengers += passengers_in;
            }

            safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                       "): " + std::to_string(passengers_out) + " out, " +
                       std::to_string(passengers_in) + " in, total: " +
                       std::to_string(state.passengers) + ", system: " + std::to_string(current_passengers));

            int stop_time_ms = (20 + (rand() % 21)) * 1000 / 120;
            std::this_thread::sleep_for(std::chrono::milliseconds(stop_time_ms));

            safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                       ") departed from " + stations[current_station] +
                       ", heading to: " + next_station);

            station_lock.unlock();

            if (current_station + step >= 0 && current_station + step < static_cast<int>(stations.size())) {
                std::string from = stations[current_station];
                std::string to = stations[current_station + step];
                auto key = std::make_pair(from, to);
                auto reverse_key = std::make_pair(to, from);
                double distance = distances.count(key) ? distances[key] : distances[reverse_key];
                state.total_distance += distance;
                int travel_time_ms = calculate_travel_time(distance);
                safe_print("Train " + std::to_string(train_id) + " traveling to " + to +
                           " (" + std::to_string(travel_time_ms / 1000.0) + "s)");
                std::this_thread::sleep_for(std::chrono::milliseconds(travel_time_ms));
            }

            if (breakdown_prob(gen) < 0.01) {
                breakdown_cost += 50.0;
                safe_print("Train " + std::to_string(train_id) + " (" + line_name + ") had a breakdown, repair cost: 50 AZN");
            }

            if (is_shuttle) {
                current_station += step;
                if (current_station < 0 || current_station >= static_cast<int>(stations.size())) {
                    step = -step;
                    current_station += 2 * step;
                }
            } else {
                current_station += step;
                if (current_station < 0 || current_station >= static_cast<int>(stations.size())) {
                    step = -step;
                    current_station += 2 * step;
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(fuel_cost_mutex);
            total_fuel_cost += state.total_distance * fuel_cost_per_km;
        }

        if (!is_shuttle) {
            safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                       ") completed shift " + std::to_string(driver_shift) +
                       " and returned to depot " + depot);
        } else {
            std::unique_lock<std::mutex> station_lock(shared_station_mutexes[stations[current_station]]);
            safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                       ") completed shift " + std::to_string(driver_shift) +
                       " and remained at " + stations[current_station]);
        }
        driver_shift++;
    }

    {
        std::lock_guard<std::mutex> lock(fuel_cost_mutex);
        total_fuel_cost += state.total_distance * fuel_cost_per_km + breakdown_cost;
    }

    if (!is_shuttle) {
        safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                   ") metro closed, returned to depot " + depot);
    } else {
        std::unique_lock<std::mutex> station_lock(shared_station_mutexes[stations[current_station]]);
        safe_print("Train " + std::to_string(train_id) + " (" + line_name +
                   ") metro closed, remained at " + stations[current_station]);
    }
}

void BakuSubway::run() {
    displayWelcomeAnimation();

    std::vector<std::thread> trains;
    trains.emplace_back(&BakuSubway::train, this, 1, "Green", 0);
    trains.emplace_back(&BakuSubway::train, this, 3, "Green", 0);
    trains.emplace_back(&BakuSubway::train, this, 5, "Green", 1);
    trains.emplace_back(&BakuSubway::train, this, 2, "Red", 0);
    trains.emplace_back(&BakuSubway::train, this, 4, "Red", 1);
    trains.emplace_back(&BakuSubway::train, this, 6, "Red", 0);
    trains.emplace_back(&BakuSubway::train, this, 7, "Purple", 0);
    trains.emplace_back(&BakuSubway::train, this, 8, "Light Green", 0);
    trains.emplace_back(&BakuSubway::train, this, 9, "Light Green", 1);

    for (auto& t : trains) t.join();

    const double fare = 0.5;
    const double maintenance_cost = 500.0;
    double revenue = total_passengers * fare;
    double total_cost = total_fuel_cost + maintenance_cost;
    double net_profit = revenue - total_cost;

    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << "Total unique passengers for the day: " << total_passengers << std::endl;
    std::cout << "Total revenue: " << std::fixed << std::setprecision(2) << revenue << " AZN" << std::endl;
    std::cout << "Total fuel cost: " << std::fixed << std::setprecision(2) << total_fuel_cost << " AZN" << std::endl;
    std::cout << "Maintenance cost: " << std::fixed << std::setprecision(2) << maintenance_cost << " AZN" << std::endl;
    std::cout << "Total cost: " << std::fixed << std::setprecision(2) << total_cost << " AZN" << std::endl;
    std::cout << "Net profit for Baku Metro: " << std::fixed << std::setprecision(2) << net_profit << " AZN" << std::endl;
}
