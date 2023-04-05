#include <algorithm>
#include <map>
#include <vector>
#include <cmath>
#include <iostream>
using namespace std;

map<string, map<string, vector<int>>> evaluate(map<string, map<string, int>> population, vector<int> truck_dimension, map<string, vector<int>> boxes, int total_value) {
    int container_vol = truck_dimension[0] * truck_dimension[1] * truck_dimension[2];
    map<string, vector<double>> ft;
    for (auto const&[key, individual] : population) {
        vector<vector<int>> dblf = {{0, 0, 0, truck_dimension[0], truck_dimension[1], truck_dimension[2]}};
        int occupied_vol = 0;
        int number_boxes = 0;
        int value = 0;
        vector<vector<int>> result;
        for (int i = 0; i < individual["order"].size(); i++) {
            int box_number = individual["order"][i];
            int r = individual["rotate"][i];
            sort(dblf.begin(), dblf.end(), [](const vector<int>& a, const vector<int>& b) {
                if (a[3] != b[3]) {
                    return a[3] < b[3];
                }
                if (a[5] != b[5]) {
                    return a[5] < b[5];
                }
                return a[4] < b[4];
            });
            bool box_placed = false;
            for (auto& pos : dblf) {
                vector<int> current = pos;
                int space_vol = pos[3] * pos[4] * pos[5];
                int box_vol = boxes[to_string(box_number)][3];
                int l, w, h;
                if (r == 0) {
                    l = boxes[to_string(box_number)][0];
                    w = boxes[to_string(box_number)][1];
                    h = boxes[to_string(box_number)][2];
                } else if (r == 1) {
                    w = boxes[to_string(box_number)][0];
                    l = boxes[to_string(box_number)][1];
                    h = boxes[to_string(box_number)][2];
                } else if (r == 2) {
                    l = boxes[to_string(box_number)][0];
                    h = boxes[to_string(box_number)][1];
                    w = boxes[to_string(box_number)][2];
                } else if (r == 3) {
                    h = boxes[to_string(box_number)][0];
                    l = boxes[to_string(box_number)][1];
                    w = boxes[to_string(box_number)][2];
                } else if (r == 4) {
                    h = boxes[to_string(box_number)][0];
                    w = boxes[to_string(box_number)][1];
                    l = boxes[to_string(box_number)][2];
                } else {
                    w = boxes[to_string(box_number)][0];
                    h = boxes[to_string(box_number)][1];
                    l = boxes[to_string(box_number)][2];
                }
                if (space_vol >= box_vol && pos[3] >= l && pos[4] >= w && pos[5] >= h) {
                    result.push_back({pos[0], pos[1], pos[2], l, w, h});
                    occupied_vol += box_vol;
                    number_boxes++;
                    value += boxes[to_string(box_number)][4];
                    vector<int> top_space = {pos[0], pos[1], pos[2] + h, l, w, pos[5] - h};
                    vector<int> beside_space = {pos[0], pos[1] + w, pos[2], l, pos[4] - w, pos[5]};
                    vector<int> front_space = {pos[0] + l, pos[1], pos[2], pos[3] - l, pos[4], pos[5]};
                    dblf.erase(find(dblf.begin(), dblf.end(), pos));
                    dblf.push_back(top_space);
                    dblf.push_back(beside_space);
                    dblf.push_back(front_space);
                    box_placed = true;
                    break;
                }
            }
            if (!box_placed) {
                break;
            }
        }
        vector<double> fitness{(occupied_vol / container_vol * 100), (number_boxes / boxes.size() * 100),
                                (value / total_value * 100)};
        ft[key] = fitness;
        population[key]["fitness"] = fitness;
        population[key]["result"] = result;
    }

    return {population, ft};
}
