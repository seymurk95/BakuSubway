#ifndef BAKUSUBWAY_H
#define BAKUSUBWAY_H

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <map>

class BakuSubway {
public:
    BakuSubway();
    void run();

private:
    struct Line {
        std::vector<std::string> stations;
        std::string depot;
        std::vector<std::mutex> forward_mutexes;  
        std::vector<std::mutex> backward_mutexes; 
        bool is_shuttle = false; 
    };

    std::map<std::string, Line> lines;
    std::mutex cout_mutex;

    void train(int train_id, const std::string& line_name, int direction);
    void safe_print(const std::string& message);
};

#endif // BAKUSUBWAY_H
