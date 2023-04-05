#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <random>

std::map<int, std::map<std::string, std::vector<int>>> generate_pop(std::map<int, std::vector<int>> box_params, int count, int rotation=5) {
    std::map<int, std::map<std::string, std::vector<int>>> population;

    int x = (count > 5) ? 5 : count;
    for (int i = 0; i < x; i++) {
        std::map<int, std::vector<int>> sorted_box;
        for (const auto& [box_num, box_values] : box_params) {
            sorted_box[box_num] = box_values;
        }
        std::sort(sorted_box.begin(), sorted_box.end(), [](const auto& a, const auto& b) {
            return a.second[i] < b.second[i];
        });

        std::vector<int> order;
        for (const auto& [box_num, _] : sorted_box) {
            order.push_back(box_num);
        }

        std::vector<int> rotate;
        for (int j = 0; j < box_params.size(); j++) {
            rotate.push_back(rand() % rotation);
        }

        population[i] = {{"order", order}, {"rotate", rotate}};
    }

    std::vector<int> keys;
    for (const auto& [box_num, _] : box_params) {
        keys.push_back(box_num);
    }

    for (int i = 5; i < count; i++) {
        std::random_shuffle(keys.begin(), keys.end());

        std::vector<int> rotate;
        for (int j = 0; j < box_params.size(); j++) {
            rotate.push_back(rand() % rotation);
        }

        population[i] = {{"order", keys}, {"rotate", rotate}};
    }

    return population;
}