#include <iostream>
#include <fstream>
#include <map>
#include <json>
#include <vector>
#include <algorithm>

using namespace std;

const int NUM_OF_ITERATIONS = 1;
const int NUM_OF_INDIVIDUALS = 36;
const int NUM_OF_GENERATIONS = 200;
const int PC = int(0.8 * NUM_OF_INDIVIDUALS);
const double PM1 = 0.2;
const double PM2 = 0.02;
const int K = 2;
const int ROTATIONS = 6; // 1 or 2 or 6;


int main() {
    ifstream infile("input.json");
    map <string, vector<int> > data;
    if (infile.is_open()) {
        infile >> data;
    }
    infile.close();
    vector <string> problem_indices;
    for (auto &it: data) {
        problem_indices.push_back(it.first);
    }

    for (auto &p_ind: problem_indices) {
        cout << "Running Problem Set " << p_ind << endl;
        cout << "Parameter\tValue\n";
        cout << "Generations\t" << NUM_OF_GENERATIONS << endl;
        cout << "Individuals\t" << NUM_OF_INDIVIDUALS << endl;
        cout << "Rotations\t" << ROTATIONS << endl;
        cout << "Crossover Prob.\t" << PC << endl;
        cout << "Mutation Prob1\t" << PM1 << endl;
        cout << "Mutation Prob2\t" << PM2 << endl;
        cout << "Tournament Size\t" << K << endl;
        cout << endl;

        // Extracting inputs from the json file
        vector<int> truck_dimension = data[p_ind]["truck dimension"];
        vector <vector<int> > packages = data[p_ind]["solution"];
        vector <vector<int> > boxes = data[p_ind]["boxes"];
        int total_value = data[p_ind]["total value"];
        int box_count = data[p_ind]["number"];
        map<int, vector<int> > box_params;
        for (int i = 0; i < boxes.size(); i++) {
            box_params[i] = boxes[i];
        }
        // Storing the average values over every single iteration
        vector<double> average_vol;
        vector<double> average_num;
        vector<double> average_value;

        for (int i = 0; i < NUM_OF_ITERATIONS; i++) {
            // Generate the initial population
            // gen.generate_pop(box_params, NUM_OF_INDIVIDUALS, ROTATIONS);
            // TODO: Implement generate_pop() function in C++

            int gen = 0;
            vector<double> average_fitness;
            while (gen < NUM_OF_GENERATIONS) {
                // ft.evaluate(population, truck_dimension, box_params, total_value);
                // TODO: Implement evaluate() function in C++

                // ns.rank(population, fitness);
                // TODO: Implement rank() function in C++

                // re.crossover(deepcopy(population), PC, k=K);
                // TODO: Implement crossover() function in C++

                // mt.mutate(offsprings, PM1, PM2, ROTATIONS);
                // TODO: Implement mutate() function in C++

                // ss.select(population, offsprings, truck_dimension, box_params, total_value, NUM_OF_INDIVIDUALS);
                // TODO: Implement select() function in C++

                // average_fitness.append(calc_average_fitness(population))
                // TODO: Implement calc_average_fitness() function in C++

                gen += 1;
            }
            vector <vector<int> > results;

            // Storing the final Rank 1 solutions
            // for key, value in population.items():
        }
    }
}
