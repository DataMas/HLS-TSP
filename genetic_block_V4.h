#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include "ac_int.h"

#include "mc_scverify.h"

using namespace std;

static const int populationSize = 512;
static const int numberOfNodes = 128;
static const int totalSwaps = 2*numberOfNodes;
static const int maxGenerations = 100000;
static const int max_pop_survivors = populationSize*0.25;
static const int max_pop_crossover = populationSize*0.5;


// Random Number Generator Class
class LFSR {
private:
    ac_int<32, false> _state;
public:
    LFSR(int seed){
        LFSR::_state = seed;
    }

    ac_int<32, false> run() {
        bool b_32 = _state[31];
        bool b_22 = _state[21];
        bool b_2 = _state[1];
        bool b_1 = _state[0];
        bool new_bit = b_32 ^ b_22 ^ b_2 ^ b_1;
        _state = _state >> 1;
        _state[30] = new_bit;

        return _state;
    }
};

// Genetic Algorithm Class (contains all GA steps required)
class genetic {
public:
    // Constructor
    genetic(){};


    void populationInit(ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize], LFSR &RAND ) {
        // Initialize population chromosomes

        initPOP: for (int i = 0; i < populationSize; i++) {
            initNODES: for (int j = 0; j < numberOfNodes; j++) {
                population[i][j] = j;
            }

            ac_int<11, false> temp;

            // Shuffle
            ac_int<7, false> point1,point3,point5;
            ac_int<7, false> point2,point4,point6;
            ac_int<32, false> randomNumber;

            initSHUFFLE: for (int j = 0; j < totalSwaps; ++j) {
                randomNumber = RAND.run();
                // range 0 - 63
                point1 = randomNumber.slc<6>(0);
                point2 = randomNumber.slc<6>(6);
                point1[0] = 1;  //force numbers to be >0
                point2[0] = 1;

                temp = population[i][point1];
                population[i][point1] = population[i][point2];
                population[i][point2] = temp;

                // range 0 - 127
                point3 = randomNumber.slc<7>(17);
                point4 = randomNumber.slc<7>(13);
                point3[0] = 1;  //force number to be >0
                point4[6] = 1;  //force number to be >64

                temp = population[i][point3];
                population[i][point3] = population[i][point4];
                population[i][point4] = temp;

                point5 = randomNumber.slc<7>(10);
                point6 = randomNumber.slc<7>(22);
                point5[0] = 1;  //force numbers to be >0
                point6[0] = 1;

                temp = population[i][point5];
                population[i][point5] = population[i][point6];
                population[i][point6] = temp;
            }

            // Keep the address of each chromosome in an array of pointers
            populationAddresses[i] = &population[i][0];
        }
    }


    void sortByColumn(ac_int<32, false> scores[populationSize], ac_int<11, false> *populationAddresses[populationSize], LFSR &RAND) {
        // Sort chromosomes based on their fitness Score

        ac_int<32, false> tempScore;
        ac_int<11, false> *temp_pointer_swap;
        sortPOPI: for (int i = 0; i < populationSize; i++){
            sortPOPJ: for (int j = i + 1; j < populationSize; j++){
                if (scores[i] >= scores[j]){
                    // Sort Scores array
                    tempScore = scores[i];
                    scores[i] = scores[j];
                    scores[j] = tempScore;

                    // Sort pointers based on scores array
                    temp_pointer_swap = populationAddresses[i];
                    populationAddresses[i] = populationAddresses[j];
                    populationAddresses[j] = temp_pointer_swap;
                }
            }
        }
    }

    void fitness(ac_int<32, false> scores[populationSize], ac_int<11, false> distances[numberOfNodes][numberOfNodes], ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize], LFSR &RAND) {
        // Fitness function - Calculate generation's chromosome-scores

        ac_int<11, false> city1;
        ac_int<11, false> city2;

        // Calculate the sum of all the distances for a every chromosome
        fitPOP: for (int i = 0; i < populationSize; i++) {
            scores[i] = 0;
            fitNODES: for (int j = 0; j < numberOfNodes - 1; j++) {
                city1 = population[i][j];
                city2 = population[i][j + 1];
                scores[i] = scores[i] + distances[city1][city2];
            }
        }
        // Sort chromosomes
        sortByColumn(scores, populationAddresses, RAND);
    }

    //#pragma_hls_design
    void crossover(ac_int<11, false> *populationAddresses[populationSize], LFSR &RAND) {
        /*
         *  25% of population, survives as it is.
         *  25% of population, copies the 1st 25% and them gets crossover (random area flip)
         *  50% of population, random shuffling on existing chromosomes
         */

        ac_int<11, false> index_copy;
        ac_int<11, false> swapGenes;
        // Second Quarter of the population
        cross: for (int i = (max_pop_survivors); i < (max_pop_crossover); i++) {
            index_copy = i - max_pop_survivors;
            // Copy the best genes
            copyBEST: for (int j = 0; j < numberOfNodes; ++j) {
                *(populationAddresses[i]+j) = *(populationAddresses[index_copy]+j);
             }

            ac_int<7, false> point1;
            ac_int<7, false> point2;
            ac_int<7, false> size;
            ac_int<7, false> middle;
            ac_int<32, false> randomNumber;

            randomNumber = RAND.run();
            point1 = randomNumber.slc<7>(0);
            point2 = randomNumber.slc<7>(5);
            point1[0]=1;
            point2[0]=1;

            if (point1>point2) {
                point1 = randomNumber.slc<7>(5);
                point2 = randomNumber.slc<7>(0);
                point1[0]=1;
                point2[0]=1;
            }

            size = point2 - point1;
            middle = (point1 + size) / 2;

            crossoverSWAP: for (int j = point1; j < middle; ++j) {
                ac_int<11, false> offset = size-j;
                swapGenes = *(populationAddresses[i]+j);
                *(populationAddresses[i]+j) = *(populationAddresses[i]+offset);
                *(populationAddresses[i]+offset) = swapGenes;
            }
        }

        // Regenerate last 50% of the population by random shuffling
        crossoverREG: for (int i = (max_pop_crossover); i < populationSize; i++) {

            ac_int<11, false> temp;

            ac_int<7, false> point1;
            ac_int<7, false> point2;
            ac_int<32, false> randomNumber;

            crossoverSHUFFLE: for (int j = 0; j < totalSwaps; ++j) {
                randomNumber = RAND.run();
                // range 0 - 127
                point1 = randomNumber.slc<7>(2);
                point2 = randomNumber.slc<7>(16);
                point1[0] = 1;  //force numbers to be >0
                point2[0] = 1;
                temp = *(populationAddresses[i]+point1);
                *(populationAddresses[i]+point1) = *(populationAddresses[i]+point2);
                *(populationAddresses[i]+point2) = temp;
            }
        }
    }

    void mutate(ac_int<11, false> population[populationSize][numberOfNodes],  LFSR &RAND) {
        /*
         * Mutation is implemented as a swap in a random chromosome(except for the best 2 ones)
         * between two random points
         */

        ac_int<8, false> index;
        ac_int<7, false> point1;
        ac_int<7, false> point2;
        ac_int<11, false> swapGenes;
        ac_int<32, false> randomNumber;

        mutationSWAP: for (int i = 0; i < 40; i++) {
        randomNumber = RAND.run();
        point1 = randomNumber.slc<7>(0);
        point2 = randomNumber.slc<7>(5);
        point1[0] = 1;  //force numbers to be >0
        point2[0] = 1;
        if (point1>point2) {
            point1 = randomNumber.slc<7>(5);
            point2 = randomNumber.slc<7>(0);
            point1[0] = 1;  //force numbers to be >0
            point2[0] = 1;
        }

        index = randomNumber.slc<9>(13); // 9 for 512
        // leave the best two chromosomes intact
        index[1]=1;

        swapGenes = population[index][point1];
        population[index][point1] = population[index][point2];
        population[index][point2] = swapGenes;
    }
    }
};

#pragma hls_design top
class genetic_block {
private:
    genetic gen;
public:
    // Constructor
    genetic_block(){};

    // assign genetic function as the interface
    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_int<11, false> distance_matrix[numberOfNodes][numberOfNodes], ac_int<11, false> population[populationSize][numberOfNodes]) {

        // Create random number generator
        LFSR RAND(12);

        ac_int<11, false> *populationAddresses[populationSize];

        // Initialize population
        gen.populationInit(population, populationAddresses, RAND);

        int max = 1000000;
        ac_int<32, false> scores[populationSize];
        geneticGENERATIONS: for (int i = 0; i < maxGenerations; i++) {
            gen.fitness(scores, distance_matrix, population, populationAddresses, RAND);
            if (scores[0] < max) {
                max = scores[0];
                cout << "Generation " << i << " - Best Score: " << scores[0] << endl;
            }
            gen.crossover(populationAddresses, RAND);
            gen.mutate(population, RAND);
        }
//        fstream my_file;
//        my_file.open("optimal.txt", ios::out);
//        if (!my_file) {
//            cout << "File not created!"<< endl;
//        }
//        else {
//            cout << "File created successfully!"<< endl;
        cout << "Optimal path : ";
        for (int i = 0; i <numberOfNodes-1; ++i) {
            //               my_file << *(populationAddresses[0]+i) << ",";
            cout << " -> "<< *(populationAddresses[0]+i) ;
        }
        cout << endl;
//            my_file.close();
//        }
    }

};
