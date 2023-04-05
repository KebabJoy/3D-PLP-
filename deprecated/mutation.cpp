#include <random>
#include <unordered_map>
#include <vector>
#include <algorithm>

std::unordered_map<int, std::unordered_map<std::string, std::vector<int>>> mutate(
        const std::unordered_map<int, std::unordered_map<std::string, std::vector<int>>> &offsprings,
        double pm1,
        double pm2,
        int rotation = 6) {

    std::unordered_map<int, std::unordered_map<std::string, std::vector<int>>> mutated_offsprings;

    for (const auto &[key, child] : offsprings) {
        std::vector<int> order = child.at("order");
        std::vector<int> rotate = child.at("rotate");

        if (std::uniform_real_distribution<>(0, 1)(std::mt19937(std::random_device{}())) <= pm1) {
            int i = std::uniform_int_distribution<>(1, static_cast<int>(order.size() / 2) + 1)(
                    std::mt19937(std::random_device{}()));
            int j = std::uniform_int_distribution<>(i + 1, static_cast<int>(order.size() - 1))(
                    std::mt19937(std::random_device{}()));
            std::reverse(order.begin() + i, order.begin() + j + 1);
            std::reverse(rotate.begin() + i, rotate.begin() + j + 1);
        }

        for (int i = 0; i < rotate.size(); i++) {
            if (std::uniform_real_distribution<>(0, 1)(std::mt19937(std::random_device{}())) <= pm2) {
                rotate[i] = std::uniform_int_distribution<>(0, rotation - 1)(std::mt19937(std::random_device{}()));
            }
        }

        mutated_offsprings[key] = {{"order", order}, {"rotate", rotate}};
    }

    return mutated_offsprings;
}