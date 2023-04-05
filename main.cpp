#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <random>

#define PALLET_WIDTH 150.f
#define PALLET_DEPTH 150.f

using json = nlohmann::json;
using namespace std;

const int NUM_OF_ITERATIONS = 1;
const int NUM_OF_INDIVIDUALS = 36;
const int NUM_OF_GENERATIONS = 200;
const int PC = int(0.8 * NUM_OF_INDIVIDUALS);
const double PM1 = 0.2;
const double PM2 = 0.02;
const int K = 2;
const int ROTATIONS = 6; // 1 or 2 or 6;

float totalValue { 0.f };

struct Box {
    int id;
    float sizeX;
    float sizeY;
    float sizeZ;
    float lPerimeter;
    int seedValue;
};

struct Population {
    vector<int> boxOrderIds;
    vector<int> boxRotation;
    vector<float> fitness;
    vector<vector<float> > result;
};

struct Solution {
    float totalLPerimeter;
};

Solution solution { .totalLPerimeter = 0 };

void printBoxes(const vector<Box>& boxs){
    for (auto box : boxs) {
        cout << "ID: " << box.id << endl;
        cout << "SizeX: " << box.sizeX << endl;
        cout << "SizeY: " << box.sizeY << endl;
        cout << "SizeZ: " << box.sizeZ << endl;
        cout << endl;
    }
}

void printArray(const vector<int>& arr) {
    for (int i = 0; i < arr.size(); ++i) {
        cout << arr[i] << ' ';
    }
}

vector<int> getBoxesIds(const vector<Box>& boxes) {
    vector<int> boxIds(boxes.size());
    for (int i = 0; i < boxes.size(); ++i) {
        boxIds[i] = boxes[i].id;
    }

    return boxIds;
}

vector<int> shuffleBoxIds(vector<int>& boxIds) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(boxIds.begin(), boxIds.end(), g);

    return boxIds;
}

vector<int> genRotations(int numOfInd, int size) {
    vector<int> rotations(size);
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, numOfInd - 1);

    for (int i = 0; i < size; ++i) {
        rotations[i] = distr(eng);
    }

    return rotations;
}

vector<Population> generatePopulation(const vector<Box>& boxes) {
    int x;
    if(NUM_OF_INDIVIDUALS > 4) {
        x = 4;
    } else {
        x = NUM_OF_INDIVIDUALS;
    }

    vector<int> boxOrderIds(boxes.size());
    vector<int> rotations(boxes.size());
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, ROTATIONS - 1);
    vector<Population> population(NUM_OF_INDIVIDUALS);

    for (int i = 0; i < x; ++i) {
        vector<Box> sortedBoxes = boxes;
        switch(i){
            case 0:
                sort(sortedBoxes.begin(), sortedBoxes.end(), [](auto lhs, auto rhs) {
                    return lhs.id < rhs.id;
                });
            case 1:
                sort(sortedBoxes.begin(), sortedBoxes.end(), [](auto lhs, auto rhs) {
                    return lhs.sizeX < rhs.sizeX;
                });
            case 2:
                sort(sortedBoxes.begin(), sortedBoxes.end(), [](auto lhs, auto rhs) {
                    return lhs.sizeY < rhs.sizeY;
                });
            case 3:
                sort(sortedBoxes.begin(), sortedBoxes.end(), [](auto lhs, auto rhs) {
                    return lhs.sizeZ < rhs.sizeZ;
                });
        }

        for (int j = 0; j < sortedBoxes.size(); ++j) {
            boxOrderIds[j] = sortedBoxes[j].id;
            rotations[j] = distr(eng);
        }

        population[i] = { .boxOrderIds = boxOrderIds, .boxRotation = rotations };
    }

    for (int i = 4; i < NUM_OF_INDIVIDUALS; ++i) {
        population[i] = {
                .boxOrderIds = shuffleBoxIds(boxOrderIds),
                .boxRotation = genRotations(ROTATIONS - 1, boxes.size())
        };
    }

    return population;
}

struct DBLF {
    float f1;
    float f2;
    float f3;
    float f4;
    float f5;
    float f6;
};

void evaluate(vector<Population>& population, const vector<Box>& boxes) {
    cout << population.size() << endl;
    vector<vector<float> > fitnesses(population.size());
    vector<vector<float> > result;

    float containerVol = PALLET_WIDTH * PALLET_WIDTH * 300;
    for (int i = 0; i < population.size(); ++i) {
        result.clear();
        vector<DBLF> dblf = { { .f1 = 0.f, .f2 = 0.f,  .f3 = 0.f, .f4 = PALLET_DEPTH, .f5 = PALLET_WIDTH, .f6 = INT32_MAX } };
        float occupiedVol = 0, numberBoxes = 0, value = 0;

        for (int boxNumber = 0; boxNumber < boxes.size(); ++boxNumber) {
            float boxVol = boxes[boxNumber].lPerimeter;
            float boxValue = boxes[boxNumber].seedValue;

            sort(dblf.begin(), dblf.end(), [](auto lhs, auto rhs) {
                return lhs.f4 < rhs.f4;
            });

            for (int j = 0; j < dblf.size(); j++) {

                DBLF currentPosition = dblf[j];
                float l, w, h;
                float spaceVol = currentPosition.f4 * currentPosition.f5 * currentPosition.f6;

                int rotation = population[i].boxRotation[boxNumber];

                if(rotation == 0) {
                    l = boxes[boxNumber].sizeX,
                    w = boxes[boxNumber].sizeY,
                    h = boxes[boxNumber].sizeZ;
                } else if(rotation == 1) {
                    w = boxes[boxNumber].sizeX,
                    l = boxes[boxNumber].sizeY,
                    h = boxes[boxNumber].sizeZ;
                } else if(rotation == 2) {
                    l = boxes[boxNumber].sizeX,
                    h = boxes[boxNumber].sizeY,
                    w = boxes[boxNumber].sizeZ;
                } else if(rotation == 3) {
                    h = boxes[boxNumber].sizeX,
                    l = boxes[boxNumber].sizeY,
                    w = boxes[boxNumber].sizeZ;
                } else if(rotation == 4) {
                    h = boxes[boxNumber].sizeX,
                    w = boxes[boxNumber].sizeY,
                    l = boxes[boxNumber].sizeZ;
                } else {
                    w = boxes[boxNumber].sizeX,
                    h = boxes[boxNumber].sizeY,
                    l = boxes[boxNumber].sizeZ;
                }

                if(spaceVol >= boxVol && currentPosition.f4 >= l && currentPosition.f5 >= w && currentPosition.f6 >= h) {
                    result.push_back({currentPosition.f1, currentPosition.f2, currentPosition.f3, l, w, h});
                    occupiedVol += boxVol;
                    numberBoxes += 1;
                    value += boxValue;
                    DBLF top_space = {
                            currentPosition.f1,
                            currentPosition.f2,
                            currentPosition.f3 + h,
                            l,
                            w,
                            currentPosition.f6 - h
                    };
                    DBLF beside_space = {
                            currentPosition.f1,
                            currentPosition.f2 + w,
                            currentPosition.f3,
                            l,
                            currentPosition.f5 - w,
                            currentPosition.f6
                    };
                    DBLF front_space = {
                            currentPosition.f1 + l,
                            currentPosition.f2,
                            currentPosition.f3,
                            currentPosition.f4 - l,
                            currentPosition.f5,
                            currentPosition.f6
                    };
                    dblf.erase(dblf.begin() + j);

                    dblf.push_back(top_space);
                    dblf.push_back(beside_space);
                    dblf.push_back(front_space);
                    break;
                }
            }
        }

        vector<float > fitness = {
                (occupiedVol / containerVol * 100.f),
                (numberBoxes / boxes.size() * 100.f),
                (value / totalValue * 100.f),
        };
        fitnesses[i] = fitness;
        population[i].fitness = fitness;
        population[i].result = result;
    }
}

void geneticAlg(const vector<Box>& boxes) {
    for (int i = 0; i < NUM_OF_ITERATIONS; ++i) {
        vector<Population> population = generatePopulation(boxes);

        int gens = 0;
        vector<float> averageFitness;
        while(gens < NUM_OF_GENERATIONS) {
            gens += 1;
            cout << "DEBUG geneticALG iteration: " << gens << '\n';

            evaluate(population, boxes);
        }
    }
}

int main() {
    std::ifstream infile("test.json");
    json j;
    infile >> j;

    vector<Box> boxes;

    // Parse the array of hashes
    for (const auto& item : j) {
        Box box;
        box.id =         item["id"].get<int>();
        box.sizeX =      item["sizeX"].get<float>();
        box.sizeY =      item["sizeY"].get<float>();
        box.sizeZ =      item["sizeZ"].get<float>();
        box.lPerimeter = item["lPerimeter"].get<float>();
        box.seedValue  = item["value"].get<int>();
        boxes.push_back(box);

        solution.totalLPerimeter += (box.sizeX + box.sizeY) * 4;
        totalValue += box.seedValue;
    }

    geneticAlg(boxes);

    return 0;
}
