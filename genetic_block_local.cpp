#include <iostream>
#include <cstdlib>
#include <ctime>

#include "ac_int.h"

//#include "mc_scverify.h"

using namespace std;

static const int populationSize = 128;
static const int numberOfNodes = 128;
static const int maxGenerations = 1000;
static const int max_pop_survivors = populationSize*0.25;
static const int max_pop_crossover = populationSize*0.5;

// TODO: Display Final Path Solution

class LSFR {
private:
    ac_int<32, false> _state;
public:
    LSFR(int seed){
        LSFR::_state = seed;
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

class genetic {
public:
    // Constructor
    genetic(){};

    // ----- INITIALIZE -----
    void populationInit(ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize], LSFR RAND ) {
        // This function
        // input population array, distances array
        //ac_int<length, false> population[numberOfNodes];
		//populationAddresses[0] = &population[0];

        initPOP: for (int i = 0; i < populationSize; i++) {
            initNODES: for (int j = 0; j < numberOfNodes; j++) {
                population[i][j] = j;
            }

			
            // Fisher-Yates' shuffling
            short right;
            short index;
            short len;
            ac_int<11, false> temp;
                
        	//TODO: HLS optimized shuffling
            initSHUFLE: for (int l = 1; l < (numberOfNodes - 1); l++) {
                
                right = min(l + 20 + 1, numberOfNodes - 1);
                len = l - right;
                index = l + ( RAND.run() % len);
                temp = population[i][l];
                population[i][l] = population[i][index];
                population[i][index] = temp;
            }

            // Keep the address of the chromosome's beginning
            populationAddresses[i] = &population[i][0];
        }
    }

    // ----- To sort based on column in descending order -----
    void sortByColumn(ac_int<32, false> scores[populationSize], ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize], LSFR RAND) {
        //This function

        ac_int<32, false> tempScore;
        ac_int<11, false> *temp_pointer_swap;

        sortPOPI: for (int i = 0; i < populationSize; i++)
        {
            sortPOPJ: for (int j = i + 1; j < populationSize; j++)
            {

                if (scores[i] >= scores[j])
                {
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

    // ----- FITNESS -----
    void fitness(ac_int<32, false> scores[populationSize], ac_int<11, false> distances[numberOfNodes][numberOfNodes], ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize], LSFR RAND) {
        //This function
        //input distances
        //scores:
        //    1st column holds the total sum of distances in chromosome
        //    2nd column holds the population index which will be reordered at sorting process

        ac_int<11, false> city1;
        ac_int<11, false> city2;
        //calculate the sum of all the distances for a every chromosome
        firPOP: for (int i = 0; i < populationSize; i++) {
            scores[i] = 0;
            fitNODES: for (int j = 0; j < numberOfNodes - 1; j++) {
                city1 = population[i][j];
                city2 = population[i][j + 1];
                scores[i] = scores[i] + distances[city1][city2];
            }
        }
        //return a score vector, one score for every chromosome
        sortByColumn(scores, population, populationAddresses, RAND);

    }
    //#pragma_hls_design
    // ----- CROSSOVER -----
    void crossover(ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize], LSFR RAND) {
        // This function
        // Select the mating pool based on score
        // top 25% of scores keep it intact]
        // second 25% of the population is being crossovered
        // last 2 25% of the population are being randomly regenerated

        ac_int<11, false> swapGenes;

        // crossover second quarter of the population
        cross: for (int i = (max_pop_survivors); i < (max_pop_crossover); i++) {

	        ac_int<7, false> point1;
	        ac_int<7, false> point2;
	        ac_int<7, false> size;
	        ac_int<7, false> middle;
	        ac_int<32, false> randomNumber;
        
			randomNumber = RAND.run();
			point1 = randomNumber.slc<7>(0);
			point2 = randomNumber.slc<7>(5);
			
			if (point1>point2) {
				point1 = randomNumber.slc<7>(5);
				point2 = randomNumber.slc<7>(0);
			}
			
            size = point2 - point1;
            size = point2  - point1;
            middle = (point1 + size) / 2;

            //DONE: Access the right population chromosome using the pointer address-array
            crossoverSWAP: for (int j = point1; j < middle; ++j) {
                ac_int<11, false> offset = size-j;
                swapGenes = *(populationAddresses[i]+j);
                *(populationAddresses[i]+j) = *(populationAddresses[i]+offset);
                *(populationAddresses[i]+offset) = swapGenes;
            }
        }
        // Regenerate 50% of the population last 2 4ths of the population
        crossoverREG: for (int i = (max_pop_crossover); i < populationSize; i++) {

            regINIT: for (int j = 0; j < numberOfNodes; j++) {
                *populationAddresses[i]+j = j;
            }
            // Fisher-Yates' shuffling
            short right;
            short index;
            short len;
            ac_int<11, false> temp;

			//***** TODO: HLS optimized shuffling
            //                      |----- let the starting city be city: 0
            //                      v
            regSHUFLE: for (int l = 1; l < (numberOfNodes - 1); l++) {

                right = min(l + 20 + 1, numberOfNodes - 1);
                len = l - right;
                index = l + (RAND.run() % len);
                temp = *(populationAddresses[i]+l);
                *(populationAddresses[i]+l) = *(populationAddresses[i]+index);
                *(populationAddresses[i]+index) = temp;
            }

        }
    }
    
     // -----  MUTATION -----
    void mutate(ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize], LSFR RAND) {

        ac_int<7, false> index;
        ac_int<7, false> point1;
        ac_int<7, false> point2;
        ac_int<7, false> size;
        ac_int<7, false> middle;
        ac_int<11, false> swapGenes;
        ac_int<32, false> randomNumber;

        mutationPOINTS: for (int i = 0; i < 20; i++) {
			
			randomNumber = RAND.run();
			point1 = randomNumber.slc<7>(0);
			point2 = randomNumber.slc<7>(5);
			
			if (point1>point2) {
				point1 = randomNumber.slc<7>(5);
				point2 = randomNumber.slc<7>(0);
			}
			
            index = randomNumber.slc<7>(13);
            size = point2  - point1;
            middle = (point1 + size) / 2;

            // TODO: Decide where which array to keep, population or pointer array?!
//           mutationSWAP:  for (int j = point1; j < middle; ++j) {
//                ac_int<11, false> offset = size-j;
//                swapGenes = *populationAddresses[index]+j;
//                *populationAddresses[index]+j = *populationAddresses[index]+offset;
//                *populationAddresses[index]+offset = swapGenes;
//            }
            mutationSWAP:  for (int j = point1; j < middle; ++j) { //*************
                swapGenes = population[index][j];
                population[index][j] = population[index][size - j];
                population[index][size - j] = swapGenes;
            }
        }
    }
        

};

// assign class as top
#pragma hls_design top
class genetic_block {
private:
    genetic gen;
public:
    // Constructor
    genetic_block(){};
    
    
    // assign genetic function as the interface
    #pragma hls_design interface
    void run (ac_int<11, false> distance_matrix[numberOfNodes][numberOfNodes], ac_int<11, false> population[populationSize][numberOfNodes]) {


        //gen.LFSR(11,1);
        LSFR RAND(31);

        // -Initialize Population-
        ac_int<11, false> *populationAddresses[populationSize];

        gen.populationInit(population, populationAddresses, RAND);
        int max = 1000000;
        ac_int<32, false> scores[populationSize];
        
        geneticGENERATIONS: for (int i = 0; i < maxGenerations; i++) {
        gen.fitness(scores, distance_matrix, population, populationAddresses, RAND);
        if (scores[0] < max) {
            max = scores[0];
            cout << "Generation " << i << " - Best Score: " << scores[0] << endl;
        }
        gen.crossover(population, populationAddresses, RAND);
        gen.mutate(population, populationAddresses, RAND);
        }
    }

};

int main () {
    srand(time(0));

    genetic_block GHLS;
    //create distance matrix for fully connected graph
    ac_int<11, false> distance_matrix[numberOfNodes][numberOfNodes];
    ac_int<11, false> population[populationSize][numberOfNodes];
    mainDISTANCE_I: for (int i = 0; i < numberOfNodes; ++i) {
        mainDISTANCE_J: for (int j = 0; j < numberOfNodes; ++j) {
            //distances vary from 40 + 1000 km
            if (i == j) { continue; }
            int distance = rand() % 1000 + 40;
            distance_matrix[i][j] = distance;
            distance_matrix[j][i] = distance;
        }
    }
    //    for (int i = 0; i < numberOfNodes; ++i) {
    //        for (int j = 0; j < i; ++j) {                          <---- OPTIMIZED
    //            int distance = rand() % 1000 + 40;
    //            distance_matrix[i][j] = distance;
    //            distance_matrix[j][i] = distance;
    //        }
    //        distance_matrix[i][i] = 0;
    //    }
    cout << "Algorithm Starts..!" << endl;
    GHLS.run(distance_matrix,population);
    cout << "Algorithm Finished!" << endl;
    return 0;
}
