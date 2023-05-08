#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <random>
#include <set>
#include <chrono>

#define PALLET_WIDTH 1200
#define PALLET_DEPTH 800

using json = nlohmann::json;
using namespace std;
using namespace std::chrono;

const int NUM_OF_ITERATIONS = 1;
const int NUM_OF_INDIVIDUALS = 86;
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
    int id{};
    vector<int> boxOrderIds;
    vector<int> boxRotation;
    vector<float> fitness;
    vector<vector<int> > result;
    int rank{};
    float CD{};
    int totalPerimeter;
    int totalIntersection;
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

struct DrawingDist {
    int ind;
    int id;
    vector<float> fitness;
};

class Random {
public:
    Random() = default;
    Random(std::mt19937::result_type seed) : eng(seed) {}
    int DrawNumber(int min, int max){
        return std::uniform_int_distribution<int>{min, max}(eng);
    }

    float DrawFloat(float min, float max) {
        return std::uniform_real_distribution<float>{min, max}(eng);
    }

private:
    std::mt19937 eng{std::random_device{}()};
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

void printDrawingDist(const vector<DrawingDist>& dict1){
    for (int i = 0; i < dict1.size(); ++i) {
        cout << "id: " << dict1[i].id << '\n';
        for (int j = 0; j < dict1[i].fitness.size(); ++j) {
            cout << dict1[i].fitness[j] << ' ';
        }
        cout << '\n';
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

void printPopulation(const vector<Population> population) {
    cout << "POPULATION: " << endl;
    for (const auto & pop : population) {
        cout << "id: " << pop.id << endl;
        cout << "rank: " << pop.rank << endl;
        cout << "CD: " << pop.CD << endl;
    }
}

void printResult(const vector<Population>& population){
    cout << "RESULT: " << endl;
    for (int j = 0; j < population.size(); ++j) {
        cout << "POPULATION: \n";
        for (int k = 0; k < population[j].result.size(); ++k) {
            for (int l = 0; l < population[j].result[k].size(); ++l) {
                cout << population[j].result[k][l] << ' ';
            }
            cout << endl;
        }
        cout << endl;
    }
}

void resetPopulationInds(vector<Population>& population){
    for (int i = 0; i < population.size(); ++i) {
        population[i].id = i;
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
    Random distr;

    for (int i = 0; i < size; ++i) {
        rotations[i] = distr.DrawNumber(0, numOfInd - 1);
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
    Random distr;
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
            rotations[j] = distr.DrawNumber(0, ROTATIONS - 1); // test random difference
        }

        population[i] = { .id = i, .boxOrderIds = boxOrderIds, .boxRotation = rotations };
    }

    for (int i = 4; i < NUM_OF_INDIVIDUALS; ++i) {
        population[i] = {
                .id = i,
                .boxOrderIds = shuffleBoxIds(boxOrderIds),
                .boxRotation = genRotations(ROTATIONS - 1, boxes.size())
        };
    }

    return population;
}

int calculateIntersection(const vector<int>& box1, const vector<int>& box2){
    // calculate the boundaries of the two boxes
    int x1_min = box1[0], x1_max = box1[0] + box1[3];
    int y1_min = box1[1], y1_max = box1[1] + box1[4];
    int z1_min = box1[2], z1_max = box1[2] + box1[5];

    int x2_min = box2[0], x2_max = box2[0] + box2[3];
    int y2_min = box2[1], y2_max = box2[1] + box2[4];
    int z2_min = box2[2], z2_max = box2[2] + box2[5];

    // calculate the intersection of the x, y, and z coordinates
    int x_overlap = max(0, min(x1_max, x2_max) - max(x1_min, x2_min));
    int y_overlap = max(0, min(y1_max, y2_max) - max(y1_min, y2_min));
    int z_overlap = max(0, min(z1_max, z2_max) - max(z1_min, z2_min));

    // calculate the total perimeter intersection
    return 2 * (x_overlap + y_overlap + z_overlap);
}

void evaluate(vector<Population>& population, const vector<Box>& boxes) {
//    cout << population.size() << endl;
//    vector<vector<float> > fitnesses;
//    fitnesses.reserve(population.size());
    vector<vector<int> > result;
    float occupiedVol, numberBoxes, value;
    size_t boxSize = boxes.size();

    int containerVol = PALLET_WIDTH * PALLET_WIDTH * 2000;
    for (int i = 0; i < population.size(); ++i) {
        result.clear();
        // TODO: research DBLF heuristic over again. Maybe there's another decent algorithm
        vector<DBLF> dblf = { { .f1 = 0, .f2 = 0,  .f3 = 0, .f4 = PALLET_DEPTH, .f5 = PALLET_WIDTH, .f6 = 2000 } };
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
                unsigned long long spaceVol = currentPosition.f4 * currentPosition.f5 * currentPosition.f6;
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
                    numberBoxes += 1;
                    occupiedVol += boxVol;
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

                    dblf.push_back(beside_space);
                    dblf.push_back(front_space);
                    dblf.push_back(top_space);
                    break;
                }
            }
        }

        int totalIntersections = 0, totalPerimeter = 0;
        for(int resInd = 0; resInd < result.size(); resInd++){
            if(result[resInd][2] != 0){
                totalPerimeter += 2 * (result[resInd][3] + result[resInd][4]);
            } else {
                totalPerimeter += (result[resInd][3] + result[resInd][4]);
            }

            for(int boxInd = resInd + 1; boxInd < result.size(); boxInd++){
                if (result[resInd][0] != result[boxInd][0] || result[resInd][1] != result[boxInd][1]) {
                    continue;
                }
                if (result[resInd][2] + result[resInd][5] != result[boxInd][2] && result[boxInd][2] + result[boxInd][5] != result[resInd][2]) {
                    continue;
                }

                // Calculate length of matching edges
                int matching_length = 0;
                for (int k = 0; k < 2; k++) {
                    for (int l = 0; l < 4; l++) {
                        int edge1[3] = {result[resInd][k], result[resInd][(k+1)%3], result[resInd][(k+2)%3]};
                        int edge2[3] = {result[boxInd][k], result[boxInd][(k+1)%3], result[boxInd][(k+2)%3]};
                        if (edge1[0] == edge2[0] && edge1[1] == edge2[1]) {
                            matching_length += min(result[resInd][k+3], result[boxInd][k+3]);
                        }
                    }
                }

                totalIntersections += matching_length;
//                vector<int> box1 = result[resInd];
//                vector<int> box2 = result[boxInd];
//                if (box1[0]+box1[3] > box2[0] && box1[0] < box2[0]+box2[3] &&
//                        box1[1]+box1[4] > box2[1] && box1[1] < box2[1]+box2[4]) {
//                    totalIntersections += calculateIntersection(box1, box2);
//                }
            }
        }
//        cout << "INTER: " << totalIntersections << endl;
//        cout << "per: " << totalPerimeter << endl;
        vector<float > fitness = {
                (float)(totalPerimeter - totalIntersections),
                ((numberBoxes / boxes.size() * 100.f)),
                ((value / totalValue * 100.f)),
        };

        population[i].fitness = fitness;
        population[i].totalPerimeter = totalPerimeter;
        population[i].totalIntersection = totalIntersections;
//        cout << "RESULT: " << endl;
//        for (int j = 0; j < result.size(); ++j) {
//            for (int k = 0; k < result[j].size(); ++k) {
//                cout << result[j][k] << ' ';
//            }
//            cout << endl;
//        }
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

    if(dominance[0]){
        return pFitness;
    } else if(!dominance[0] && dominance[0] == dominance[1] && dominance[1] == dominance[2]){
        return qFitness;
    } else {
        return {};
    }
}

std::vector<DrawingDist> calcDrawingDist(const vector<Population>& population){
    int obj = (int)population[0].fitness.size() - 1;
    std::vector<DrawingDist> dict1;
    dict1.reserve(population.size());
    set<int> ranks;

    for (int i = 0; i < population.size(); ++i) {
        ranks.insert((int)population[i].fitness[3]);
    }

    vector<DrawingDist> group;
    group.reserve(population.size());

    for (auto const& rank : ranks) {
        group.clear();

        int cntr = 0;
        for (int groupInd = 0; groupInd < population.size(); ++groupInd) {
            if ((int)population[groupInd].fitness[3] == rank) {
                group.push_back({ .ind = cntr, .id = groupInd, .fitness = population[groupInd].fitness });
                cntr++;
            }
        }
        for (int i = 0; i < group.size(); ++i) {
            group[i].fitness.push_back(0);
        }

        for (int i = 0; i < obj; i++) {
            std::vector<DrawingDist> sortedGroup(group.begin(), group.end());
            std::sort(sortedGroup.begin(), sortedGroup.end(),
                      [i](auto const& x, auto const& y) { return x.fitness[i] < y.fitness[i]; });

            sortedGroup[0].fitness[4] = 5000;
            group[sortedGroup[0].ind].fitness[4] = 5000;
            sortedGroup.back().fitness[4] = 5000;
            group[sortedGroup.back().ind].fitness[4] = 5000;

            for (int j = 1; j < sortedGroup.size() - 1; j++) {
                sortedGroup[j].fitness[4] = (sortedGroup[j + 1].fitness[i] - sortedGroup[j - 1].fitness[i]) / 100.f;
                group[sortedGroup[j].ind].fitness[4] = (sortedGroup[j + 1].fitness[i] - sortedGroup[j - 1].fitness[i]) / 100.f;
            }
        }
        dict1.insert(dict1.end(), group.begin(), group.end());
    }
    return dict1;
}

vector<Population> selectParents(vector<Population>& population) {
    int lhsInd, rhsInd;
//    cout << population.size() << endl;

    vector<Population> parents(PC);
    for(int i = 0; i < PC; ++i){
        Random distr;

        lhsInd = distr.DrawNumber(0, population.size() - 1);
        rhsInd = distr.DrawNumber(0, population.size() - 1);
        while(rhsInd == lhsInd) {
            rhsInd = distr.DrawNumber(0, population.size() - 1);
        }

        vector<Population> pool = { population[lhsInd], population[rhsInd] };
        if(pool[0].rank > pool[1].rank) {
            parents[i] = pool[0];
            population.erase(population.begin() + lhsInd);
        } else if(pool[0].rank < pool[1].rank) {
            parents[i] = pool[1];
            population.erase(population.begin() + rhsInd);
        } else if(pool[0].CD > pool[1].CD) {
            parents[i] = pool[0];
            population.erase(population.begin() + lhsInd);
        } else {
            parents[i] = pool[1];
            population.erase(population.begin() + rhsInd);
        }
    }

    return parents;
}

vector<Population> recombine(vector<Population> parents) {
    vector<Population> offsprings(parents.size());
    resetPopulationInds(parents); // TODO: check if at this stage parents should be immutable (N.S 6.04.2023)
    vector<int> keys(parents.size()); // INFO: size == 28 IMPORTANT!!
    for (size_t i = 0; i < keys.size(); ++i) {
        keys[i] = i;
    }
    shuffleBoxIds(keys);
    for (size_t i = 0; i < parents.size(); i += 2) {
        Random distr;

        int k1 = distr.DrawNumber(0, (int)keys.size() - 1);
        vector<int> boxOrder1 = parents[k1].boxOrderIds;
        vector<int> rotations1 = parents[k1].boxRotation;
        keys.erase(keys.begin() + k1);

        int k2 = distr.DrawNumber(0, (int)keys.size() - 1);
        vector<int> boxOrder2 = parents[k2].boxOrderIds;
        vector<int> rotations2 = parents[k2].boxRotation;
        keys.erase(keys.begin() + k2);

        int ik = distr.DrawNumber(1, (int)boxOrder1.size() / 2 + 1);
        int jk = distr.DrawNumber(i + 1, boxOrder1.size() - 1);

        vector<int> co1(boxOrder1.size(), -1);
        vector<int> co2(boxOrder2.size(), -1);
        vector<int> cr1(rotations1.size(), -1);
        vector<int> cr2(rotations2.size(), -1);

        for (int k = ik; k <= jk; ++k) {
            co1[k] = boxOrder1[k];
            co2[k] = boxOrder2[k];
            cr1[k] = rotations1[k];
            cr2[k] = rotations2[k];
        }
        int pos = (jk + 1) % boxOrder2.size();

        for (int k = 0; k < boxOrder2.size(); ++k) {
            if (std::find(co1.begin(), co1.end(), boxOrder2[k]) == co1.end() && co1[pos] == -1) {
                co1[pos] = boxOrder2[k];
                pos = (pos + 1) % boxOrder2.size();
            }
        }

        pos = (jk + 1) % boxOrder2.size();
        for (int k = 0; k < boxOrder1.size(); ++k) {
            if (std::find(co2.begin(), co2.end(), boxOrder1[k]) == co2.end() && co2[pos] == -1) {
                co2[pos] = boxOrder1[k];
                pos = (pos + 1) % boxOrder1.size();
            }
        }

        pos = (jk + 1) % boxOrder2.size();
        for (int k = 0; k < rotations2.size(); ++k) {
            if (cr1[pos] == -1) {
                cr1[pos] = rotations2[k];
                pos = (pos + 1) % rotations2.size();
            }
        }

        pos = (jk + 1) % boxOrder2.size();
        for (int k = 0; k < rotations1.size(); ++k) {
            if (cr2[pos] == -1) {
                cr2[pos] = rotations1[k];
                pos = (pos + 1) % rotations1.size();
            }
        }

        offsprings[i] = { .boxOrderIds = co1, .boxRotation = cr1 };
        offsprings[i + 1] = { .boxOrderIds = co2, .boxRotation = cr2 };
    }

    return offsprings;
}

vector<Population> crossover(vector<Population> population){
    vector<Population> parents = selectParents(population);
    vector<Population> child = recombine(parents);
    return child;
}

void ranK(vector<Population>& population) {
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
    while(!frontal.empty()){
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
    vector<DrawingDist> result = calcDrawingDist(population);
//    cout << "POPULATION:\n";
    for (int j = 0; j < result.size(); ++j) {
        population[result[j].id].rank = (int)result[j].fitness[3];
        population[result[j].id].CD = result[j].fitness[4];
    }
//    for (int j = 0; j < population.size(); ++j) {
//        cout << population[j].rank << ' ' << population[j].CD << '\n';
//    }
}

void mutate(vector<Population>& offsprings){
    Random rand;
//    cout << "BEFORE: \n";
//    for (int i = 0; i < offsprings[0].boxOrderIds.size(); ++i) {
//        cout << offsprings[0].boxOrderIds[i] << ' ';
//    }
//    cout << endl;

    for (int i = 0; i < offsprings.size(); ++i) {
        vector<int>& boxOrderIds = offsprings[i].boxOrderIds;
        vector<int>& rotations = offsprings[i].boxRotation;

        if(rand.DrawFloat(0.0, 1.0) <= PM1){
            int ik = rand.DrawNumber(1, static_cast<int>(boxOrderIds.size()) / 2 + 1);
            int jk = rand.DrawNumber(i + 1, static_cast<int>(boxOrderIds.size()) - 1);
//            cout << "IK: " << ik << " JK: " << jk << endl; // DEBUG
            reverse(boxOrderIds.begin() + ik, boxOrderIds.begin() + jk + 1);
            reverse(rotations.begin() + ik, rotations.begin() + jk + 1);
        }

        // Second lvl of mutation
        for (int & rotation : rotations) {
            if (rand.DrawFloat(0.0, 1.0) <= PM2) {
                rotation = rand.DrawNumber(0, ROTATIONS);
            }
        }

//        cout << "MUTATION NUMBER " << i << endl;
//        for (int j = 0; j < boxOrderIds.size(); ++j) {
//            cout << boxOrderIds[j] << ' ';
//        }
//        cout << endl;

    }

//    cout << "AFTER: \n";
//    for (int i = 0; i < offsprings[0].boxOrderIds.size(); ++i) {
//        cout << offsprings[0].boxOrderIds[i] << ' ';
//    }
//    cout << endl;
}

vector<Population> select(
        vector<Population> population,
        vector<Population> offsprings,
        vector<Box> boxes){
    vector<Population> survivors;
    evaluate(offsprings, boxes);
    ranK(offsprings);
    auto pool = population;
    pool.insert(pool.end(), offsprings.begin(), offsprings.end());

    int i = 1;
    while (survivors.size() < NUM_OF_INDIVIDUALS) {
        vector<Population> group;

        for (const auto& entry : pool) {
            if (entry.rank == i) {
                group.push_back(entry);
            }
        }

        if (group.size() <= NUM_OF_INDIVIDUALS - survivors.size()) {
            for (std::size_t j = 0; j < group.size(); ++j) {
                survivors.push_back(group[j]);
            }
        }
        else {
            std::sort(group.begin(), group.end(),
                      [](const Population& a,
                         const Population& b) {
                          return a.CD > b.CD;
                      });
            int size = NUM_OF_INDIVIDUALS - survivors.size();
            for (std::size_t j = 0; j < size; j++) {
                survivors.push_back(group[j]);
            }
        }
        i++;
    }
    return survivors;
}

vector<float> calculateAverageFitness(const vector<Population>& population){
    vector<float> avgFitness = { 0.f, 0.f, 0.f, 0.f };

    int count = 0;
    for (const auto & pop : population) {
        if(pop.rank == 1){
            count++;
            avgFitness[0] += (float)pop.totalIntersection;
            avgFitness[1] += (float)pop.totalPerimeter;
            avgFitness[2] += pop.fitness[2];
            avgFitness[3] += (float)pop.totalIntersection / (float)pop.totalPerimeter;
        }
    }

    avgFitness[0] /= count;
    avgFitness[1] /= count;
    avgFitness[2] /= count;
    avgFitness[3] /= count;

    return avgFitness;
}

void geneticAlg(const vector<Box>& boxes) {
    for (int i = 0; i < NUM_OF_ITERATIONS; ++i) {
        vector<Population> population = generatePopulation(boxes);

        int gens = 0;
        vector<vector<float> > averageFitness;
        while(gens < NUM_OF_GENERATIONS) {
            gens += 1;
            cout << "DEBUG geneticALG iteration: " << gens << '\n';

            evaluate(population, boxes);
            cout << "Population: " << population.size() << '\n';

            ranK(population);
            vector<Population> offsprings = crossover(population);

            mutate(offsprings);
            population = select(population, offsprings, boxes);
            averageFitness.push_back(calculateAverageFitness(population));
        }

        vector<Population> results;
        results.reserve(population.size());
        for (int j = 0; j < population.size(); ++j) {
            if(population[j].rank == 1){
                results.push_back(population[j]);
            }
        }



        json j = json::array();
        json avgFit = json::array();

        for (const auto& p : results) {
            json jp;
            jp["result"] = p.result;
            j.push_back(jp);
        }
        for(const auto& p : averageFitness) {
            avgFit.push_back(p);
        }

        std::ofstream ofs("populations.json");
        std::ofstream ofs1("avg_fitness.json");
        ofs1 << avgFit.dump(4);
        ofs << j.dump(4);  // the "4" argument adds indentation for better readability
        ofs.close();

//        printBoxes(boxes);
    }
}

int main() {
    std::ifstream infile("test-2.json");
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

    auto start = high_resolution_clock::now();
    geneticAlg(boxes);
    auto end = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Time taken: " << duration.count() / 1000.f << " seconds" << endl;

    return 0;
}
