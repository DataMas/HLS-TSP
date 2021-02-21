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
static const int maxGenerations = 10000;
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

#pragma hls_design top
class genetic {
private:
	
    void populationInit(ac_int<11, false> population[populationSize][numberOfNodes], LFSR &RAND ) {
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

            // Keep the index of each chromosome
            populationAddresses[i] = i;
        }
    }


    void sortByColumn(ac_int<32, false> scores[populationSize], LFSR &RAND) {
        // Sort chromosomes based on their fitness Score

        ac_int<32, false> tempScore;
        ac_int<11, false> index_swap;
        // temp variables to avoid too many memory accesses
        ac_int<32, false> score_i,score_j;

        sortPOPI: for (int i = 0; i < populationSize; i++){
            sortPOPJ: for (int j = i + 1; j < populationSize; j++){
                score_i = scores[i];
                score_j = scores[j];
                if (score_i >= score_j){
                    // Sort Scores array
                    tempScore = score_i;
                    scores[i] = score_j;        //<----OPTIMIZED
                    scores[j] = tempScore;

                    // Sort indexes based on scores array
                    index_swap = populationAddresses[i];
                    populationAddresses[i] = populationAddresses[j];
                    populationAddresses[j] = index_swap;
                }
            }
        }
    }

    void fitness(ac_int<32, false> scores[populationSize], ac_int<11, false> distances[numberOfNodes][numberOfNodes], ac_int<11, false> population[populationSize][numberOfNodes], LFSR &RAND) {
            // Fitness function - Calculate generation's chromosome-scores

            ac_int<11, false> city1;
            ac_int<11, false> city2;
            ac_int<32, false> sum;

            // Calculate the sum of all the distances for a every chromosome
            fitPOP: for (int i = 0; i < populationSize; i++) {
                // use local sum variable to avoid accessing scores array many times
                sum = 0;
                city1 = population[i][0];
                fitNODES: for (int j = 1; j < numberOfNodes; j++) {
                    city2 = population[i][j];                           // <--OPTIMIZED
                    sum  = sum + distances[city1][city2];
                    city1 = city2;
                }
                scores[i] = sum;
            }
            // Sort chromosomes
            sortByColumn(scores, RAND);
    }

    //#pragma_hls_design
    void crossover(ac_int<11, false> population[populationSize][numberOfNodes], LFSR &RAND) {
        /*
         *  25% of population, survives as it is.
         *  25% of population, copies the 1st 25% and them gets crossover (random area flip)
         *  50% of population, random shuffling on existing chromosomes
         */

        ac_int<11, false> index_copy;
        ac_int<11, false> swapGenes;
        ac_int<11, false> popAddr_i,popAddr_copy;
        ac_int<11, false> toy;

        // Second Quarter of the population
        cross: for (int i = (max_pop_survivors); i < (max_pop_crossover); i++) {
            index_copy = i - max_pop_survivors;
            // Keep populationAddresses[i] and populationAddresses[index_copy] to reuse it in loop copyBEST
            popAddr_copy = populationAddresses[index_copy];
            popAddr_i = populationAddresses[i];
            // Copy the best genes
            copyBEST: for (int j = 0; j < numberOfNodes; ++j) {
                population[popAddr_i][j] =  population[popAddr_copy][j];;                              
            }                                                                                       

            ac_int<7, false> point1;
            ac_int<7, false> point2;
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

            ac_int<7, false> size = point2 - point1;
            ac_int<11, false> total = point1 + point2;
            ac_int<7, false> middle = point1 + (size >> 1);   // Replaced /2 with right bit shift..Bitwise operations are faster
            ac_int<11, false> offset;

            crossoverSWAP: for (int j = point1; j < middle; ++j) {
                offset = total - j;
                swapGenes = population[popAddr_i][j];
                population[popAddr_i][j] = population[popAddr_i][offset];
                population[popAddr_i][offset] = swapGenes;
            }
        }

        // Regenerate last 50% of the population by random shuffling
        crossoverREG: for (int i = (max_pop_crossover); i < populationSize; i++) {

            ac_int<11, false> temp;
            ac_int<7, false> point1;
            ac_int<7, false> point2;
            ac_int<32, false> randomNumber;

            // Keep populationAddresses[i] to reuse it in loop
            ac_int<11, false> popAddr_i = populationAddresses[i];
            crossoverSHUFFLE: for (int j = 0; j < totalSwaps; ++j) {
                randomNumber = RAND.run();
                // range 0 - 127
                point1 = randomNumber.slc<7>(2);
                point2 = randomNumber.slc<7>(16);
                point1[0] = 1;  //force numbers to be >0
                point2[0] = 1;
                temp = population[popAddr_i][point1];
                population[popAddr_i][point1] = population[popAddr_i][point2];
                population[popAddr_i][point2] = temp;
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

            index = randomNumber.slc<9>(13); // 9 for 512
            // leave the best two chromosomes intact
            index[1]=1;

            swapGenes = population[index][point1];
            population[index][point1] = population[index][point2];
            population[index][point2] = swapGenes;
        }
    }

    ac_int<11, false> populationAddresses[populationSize];
public:
    // Constructor
    genetic(){};

    // assign genetic function as the interface
    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_int<11, false> distance_matrix[numberOfNodes][numberOfNodes], ac_int<11, false> population[populationSize][numberOfNodes]) {

        // Create random number generator
        LFSR RAND(12);

        // Initialize population
        populationInit(population, RAND);
//        for (int i = 0; i <numberOfNodes-1; ++i) {
//            cout <<  population[*populationAddresses[i]] << "  " ;
//        }
//        cout << endl;
        int max = 1000000;
        ac_int<32, false> scores[populationSize];
        geneticGENERATIONS: for (int i = 0; i < maxGenerations; i++) {
            fitness(scores, distance_matrix, population, RAND);
            if (scores[0] < max) {
                max = scores[0];
                cout << "Generation " << i << " - Best Score: " << scores[0] << endl;
            }
            crossover(population, RAND);
            mutate(population, RAND);
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
                cout << " -> "<< population[populationAddresses[0]][i] ;
            }
            cout << endl;
//            my_file.close();
//        }
    }

};


