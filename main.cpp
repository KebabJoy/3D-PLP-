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
    int id{};
    vector<int> boxOrderIds;
    vector<int> boxRotation;
    vector<float> fitness;
    vector<vector<int> > result;
    int rank{};
    float CD{};
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
            rotations[j] = distr.DrawNumber(0, ROTATIONS - 1);
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
        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(0, population.size() - 1);

        lhsInd = distr(eng);
        rhsInd = distr(eng);
        while(rhsInd == lhsInd) {
            rhsInd = distr(eng);
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
    vector<Population> offsprings;
    resetPopulationInds(parents); // TODO: check if at this stage parents should be immutable (N.S 6.04.2023)
    vector<int> keys(parents.size()); // INFO: size == 28 IMPORTANT!!
    for (size_t i = 0; i < keys.size(); ++i) {
        keys[i] = i;
    }
    shuffleBoxIds(keys);
    for (size_t i = 0; i < parents.size(); i += 2) {
        Random distr;
//        std::random_device rd;
//        std::mt19937 eng(rd());
//        std::uniform_int_distribution<> distr(0, (int)keys.size() - 1);

        int k1 = distr.DrawNumber(0, (int)keys.size() - 1);
        vector<int> boxOrder1 = parents[k1].boxOrderIds;
        vector<int> rotations1 = parents[k1].boxRotation;
        keys.erase(keys.begin() + k1);
        std::uniform_int_distribution<> distr2(0, (int)keys.size() - 1);
        int k2 = distr.DrawNumber(0, (int)keys.size() - 1);
        vector<int> boxOrder2 = parents[k2].boxOrderIds;
        vector<int> rotations2 = parents[k2].boxRotation;
        keys.erase(keys.begin() + k2);
        int ik = distr.DrawNumber(1, (int)boxOrder1.size() / 2 + 1);

//        i = random.randint(1, int(len(o1) / 2) + 1)
//        j = random.randint(i + 1, int(len(o1) - 1))
//        # print("Values of i is {} and j is {}".format(i, j))
//        co1 = [-1] * len(o1)
//        co2 = [-1] * len(o2)
//        cr1 = [-1] * len(r1)
//        cr2 = [-1] * len(r2)
//
//        co1[i:j + 1] = o1[i:j + 1]
//        co2[i:j + 1] = o2[i:j + 1]
//        cr1[i:j + 1] = r1[i:j + 1]
//        cr2[i:j + 1] = r2[i:j + 1]
//        pos = (j + 1) % len(o2)
//        for k in range(len(o2)):
//            if o2[k] not in co1 and co1[pos] == -1:
//                co1[pos] = o2[k]
//                pos = (pos + 1) % len(o2)
//        pos = (j + 1) % len(o2)
//        for k in range(len(o1)):
//            if o1[k] not in co2 and co2[pos] == -1:
//                co2[pos] = o1[k]
//                pos = (pos + 1) % len(o1)
//        pos = (j + 1) % len(o2)
//        for k in range(len(r2)):
//            if cr1[pos] == -1:
//                cr1[pos] = r2[k]
//                pos = (pos + 1) % len(r2)
//        pos = (j + 1) % len(o2)
//        for k in range(len(r1)):
//            if cr2[pos] == -1:
//                cr2[pos] = r1[k]
//                pos = (pos + 1) % len(r1)
//        offsprings[x] = {'order': deepcopy(co1), 'rotate': deepcopy(cr1)}
//        offsprings[x + 1] = {'order': deepcopy(co2), 'rotate': deepcopy(cr2)}
    }

    return offsprings;
}

void crossover(vector<Population> population){
    vector<Population> parents = selectParents(population);
    vector<Population> child = recombine(parents);
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
            crossover(population);
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
