#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <random>
#include <set>

#define PALLET_WIDTH 150
#define PALLET_DEPTH 150

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
    int sizeX;
    int sizeY;
    int sizeZ;
    int lPerimeter;
    int seedValue;
};

struct Population {
    vector<int> boxOrderIds;
    vector<int> boxRotation;
    vector<float> fitness;
    vector<vector<int> > result;
};

struct Solution {
    float totalLPerimeter;
    set<int> sP;
    int numP;
    int rank;
};

struct DBLF {
    int f1;
    int f2;
    int f3;
    int f4;
    int f5;
    int f6;
};

Solution solution { .totalLPerimeter = 0, .numP = 0 };

void printBoxes(const vector<Box>& boxs){
    for (auto box : boxs) {
        cout << "ID: " << box.id << endl;
        cout << "SizeX: " << box.sizeX << endl;
        cout << "SizeY: " << box.sizeY << endl;
        cout << "SizeZ: " << box.sizeZ << endl;
        cout << "lPerimeter: " << box.lPerimeter << endl;
        cout << endl;
    }
}

void printArray(const vector<int>& arr) {
    for (int i = 0; i < arr.size(); ++i) {
        cout << arr[i] << ' ';
    }
}

template<class T>
void print2DArray(const vector<vector<T> > group){
    for (int i = 0; i < group.size(); ++i) {
        for (int j = 0; j < group[i].size(); ++j) {
            cout << group[i][j] << ' ';
        }
        cout << endl;
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

void evaluate(vector<Population>& population, const vector<Box>& boxes) {
//    cout << population.size() << endl;
//    vector<vector<float> > fitnesses;
//    fitnesses.reserve(population.size());
    vector<vector<int> > result;
    float occupiedVol, numberBoxes, value;
    size_t boxSize = boxes.size();

    int containerVol = PALLET_WIDTH * PALLET_WIDTH * 300;
    for (int i = 0; i < population.size(); ++i) {
        result.clear();
        vector<DBLF> dblf = { { .f1 = 0, .f2 = 0,  .f3 = 0, .f4 = PALLET_DEPTH, .f5 = PALLET_WIDTH, .f6 = 300 } };
        occupiedVol = 0, numberBoxes = 0, value = 0;

        for (int boxNumber = 0; boxNumber < boxSize; ++boxNumber) {
            int boxVol = boxes[boxNumber].lPerimeter;
            int boxValue = boxes[boxNumber].seedValue;

            sort(dblf.begin(), dblf.end(), [](auto lhs, auto rhs) {
                return lhs.f4 < rhs.f4;
            });

            for (int j = 0; j < dblf.size(); j++) {
                DBLF currentPosition = dblf[j];
                int l, w, h;
                int spaceVol = currentPosition.f4 * currentPosition.f5 * currentPosition.f6;
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
                ((numberBoxes / boxes.size() * 100.f)),
                ((value / totalValue * 100.f)),
        };
        population[i].fitness = fitness;
        population[i].result = result;
    }
}

vector<float> getDominantSolution(const vector<float>& pFitness, const vector<float>& qFitness){
    int obj = (int)pFitness.size();
    vector<bool> dominance;

    for (int i = 0; i < obj; ++i) {
        if (pFitness[i] >= qFitness[i]){
            dominance.push_back(true);
        } else{
            dominance.push_back(false);
        }
    }

    if(dominance[0] && dominance[0] == dominance[1] && dominance[1] == dominance[2]){
        return pFitness;
    } else if(!dominance[0] && dominance[0] == dominance[1] && dominance[1] == dominance[2]){
        return qFitness;
    } else {
        return {};
    }
}

std::vector<vector<float> > calcDrawingDist(const vector<Population>& population){
    int obj = population[0].fitness.size() - 1;
    std::vector<vector<float> > dict1;
    dict1.reserve(population.size());
    set<int> ranks;

    for (int i = 0; i < population.size(); ++i) {
        ranks.insert((int)population[i].fitness[3]);
    }

    std::vector<std::vector<float> > group;
    group.reserve(population.size());

    for (auto const& rank : ranks) {
        group.clear();

        for (int groupInd = 0; groupInd < population.size(); ++groupInd) {
            if ((int)population[groupInd].fitness[3] == rank) {
                group.push_back(population[groupInd].fitness);
            }
        }
        for (int i = 0; i < group.size(); ++i) {
            group[i].push_back(0);
        }

        for (int i = 0; i < obj; i++) {
            std::vector<std::vector<float> > sortedGroup(group.begin(), group.end());
            std::sort(sortedGroup.begin(), sortedGroup.end(),
                      [i](auto const& x, auto const& y) { return x[i] < y[i]; });
            std::vector<std::vector<float> > list1;
            list1.reserve(sortedGroup.size());

            for (auto const& item : sortedGroup) {
                list1.push_back(item);
            }

            list1[0][4] = 5000;
            list1.back()[4] = 5000;
            for (int j = 1; j < list1.size() - 1; j++) {
                list1[j][4] += (list1[j + 1][i] - list1[j - 1][i]) / 100;
            }
        }
        dict1.insert(dict1.end(), group.begin(), group.end());
    }
    return dict1;
}

void ranK(vector<Population> population) {
    vector<Solution> solutions(population.size(), { .totalLPerimeter = 0, .numP = 0 });
    set<int> frontal;

    for (int currentSolutionInd = 0; currentSolutionInd < population.size(); ++currentSolutionInd) { // loop over fitnesses
        for (int j = 0; j < population.size(); ++j) { // loop over fitnesses
            if(currentSolutionInd != j){
                vector<float> dominance = getDominantSolution(population[currentSolutionInd].fitness,population[j].fitness);
                if (dominance.empty()){
                    continue;
                } else if(dominance == population[currentSolutionInd].fitness) {
                    solutions[currentSolutionInd].sP.insert(j);
                } else if(dominance == population[j].fitness) {
                    solutions[currentSolutionInd].numP += 1;
                }
            }
        }

        if(solutions[currentSolutionInd].numP == 0){
            solutions[currentSolutionInd].rank = 1;
            population[currentSolutionInd].fitness.push_back(1);
            frontal.insert(currentSolutionInd);
        }
    }

    int i = 1;
    while(frontal.size() != 0){
        set<int> sub;
        for (auto sol : frontal) {
            for(auto dominatedSolution : solutions[sol].sP){
                solutions[dominatedSolution].numP -= 1;
                if(solutions[dominatedSolution].numP == 0) {
                    solutions[dominatedSolution].rank = i + 1;
                    population[dominatedSolution].fitness.push_back(i + 1);
                    sub.insert(dominatedSolution);
                }
            }
        }

        i += 1;
        frontal = sub;
    }
    calcDrawingDist(population);
//
//    for key, value in result.items():
//        population[key]['Rank'] = value[3]
//        population[key]['CD'] = value[4]

//    return population;
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
            ranK(population);
        }
//        printBoxes(boxes);
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
        box.sizeX =      item["sizeX"].get<int>();
        box.sizeY =      item["sizeY"].get<int>();
        box.sizeZ =      item["sizeZ"].get<int>();
        box.lPerimeter = item["lPerimeter"].get<int>();
        box.seedValue  = item["value"].get<int>();
        boxes.push_back(box);

        solution.totalLPerimeter += (box.sizeX + box.sizeY) * 4;
        totalValue += box.seedValue;
    }

    geneticAlg(boxes);

    return 0;
}
