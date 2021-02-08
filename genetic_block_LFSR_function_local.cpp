#include <iostream>
#include <cstdlib>
#include <ctime>
#include "ac_int.h"

//#include "mc_scverify.h"

using namespace std;

static const int populationSize = 256;
static const int numberOfNodes = 128;
static const int maxGenerations = 10000;
static const int max_pop_survivors = populationSize*0.25;
static const int max_pop_crossover = populationSize*0.5;

class genetic {
public:
    // Constructor
    genetic(){};
    
	//TODO: remove seed, load init
	int LFSR(unsigned int seed=0,int load=0) { 

	static ac_int<32, false> lfsr;
    	if (load ==1 )
        	lfsr = seed;

	   bool b_32 = lfsr[31];
	   bool b_22 = lfsr[21];
	   bool b_2 = lfsr[1];
	   bool b_1 = lfsr[0];
	   bool new_bit = b_32 ^ b_22 ^ b_2 ^ b_1;
	   lfsr = lfsr >> 1;
	   lfsr[30] = new_bit;
	   
	   return lfsr;

    }

    // ----- INITIALIZE -----
    void populationInit(ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize]) {

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
                index = l + (LFSR() % len);
                temp = population[i][l];
                population[i][l] = population[i][index];
                population[i][index] = temp;
            }

            // Keep the address of the chromosome's beginning
            populationAddresses[i] = &population[i][0];
        }
    }

    // ----- To sort based on column in descending order -----
    void sortByColumn(ac_int<32, false> scores[populationSize], ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize]) {
        //This function

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

    // ----- FITNESS -----
    void fitness(ac_int<32, false> scores[populationSize], ac_int<11, false> distances[numberOfNodes][numberOfNodes], ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize]) {
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
        sortByColumn(scores, population, populationAddresses);

    }
    //#pragma_hls_design
    // ----- CROSSOVER -----
    void crossover(ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize]) {
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
        
			randomNumber = LFSR();
            point2 = LFSR() % (numberOfNodes - point1) + point1;
			point1 = randomNumber.slc<7>(0);
			point2 = randomNumber.slc<7>(5);
			
			if (point1>point2) {
				point1 = randomNumber.slc<7>(5);
				point2 = randomNumber.slc<7>(0);
			}
			
            size = point2 - point1;
            size = point2  - point1;
            middle = (point1 + size) / 2;

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
                *(populationAddresses[i]+j) = j;
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
                index = l + (LFSR() % len);
                temp = *(populationAddresses[i]+l);
                *(populationAddresses[i]+l) = *(populationAddresses[i]+index);
                *(populationAddresses[i]+index) = temp;
            }
        }
    }
    
     // -----  MUTATION -----
    void mutate(ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize]) {

        ac_int<7, false> index;
        ac_int<7, false> point1;
        ac_int<7, false> point2;
        ac_int<7, false> size;
        ac_int<7, false> middle;
        ac_int<11, false> swapGenes;
        ac_int<32, false> randomNumber;

        mutationPOINTS: for (int i = 0; i < 40; i++) {
			
			randomNumber = LFSR();
			point1 = randomNumber.slc<7>(0);
			point2 = randomNumber.slc<7>(5);
			
			if (point1>point2) {
				point1 = randomNumber.slc<7>(5);
				point2 = randomNumber.slc<7>(0);
			}
			
            index = randomNumber.slc<7>(13);
            size = point2  - point1;
            middle = (point1 + size) / 2;

            mutationSWAP:  for (int j = point1; j < middle; ++j) {
                int offset = size-j;
                swapGenes = population[index][j];
                population[index][j] = population[index][offset];
                population[index][offset] = swapGenes;
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

        //LSFR RAND(31);
        gen.LFSR(11,1);
        
        // -Initialize Population-
        ac_int<11, false> *populationAddresses[populationSize];

        gen.populationInit(population, populationAddresses);
        int max = 1000000;
        ac_int<32, false> scores[populationSize];
        
        geneticGENERATIONS: for (int i = 0; i < maxGenerations; i++) {
            gen.fitness(scores, distance_matrix, population, populationAddresses);
            if (scores[0] < max) {
                max = scores[0];
                cout << "Generation " << i << " - Best Score: " << scores[0] << endl;
            }
            gen.crossover(population, populationAddresses);
            gen.mutate(population, populationAddresses);
        }
        cout << "Optimal path : ";
        for (int i = 0; i <numberOfNodes-1; ++i) {
            cout << " -> "<< *(populationAddresses[0]+i) ;
        }
        cout << endl;
    }

};

int main () {
    srand(time(0));

    genetic_block GHLS;
    //create distance matrix for fully connected graph
    ac_int<11, false> distance_matrix[numberOfNodes][numberOfNodes];
    ac_int<11, false> population[populationSize][numberOfNodes];
    mainDISTANCE_I: for (int i = 0; i < numberOfNodes; ++i) {
        mainDISTANCE_J: for (int j = 0; j < i; ++j) {
            int distance = rand() % 1000 + 40;
            distance_matrix[i][j] = distance;
            distance_matrix[j][i] = distance;
        }
        distance_matrix[i][i] = 0;
    }
    cout << "Algorithm Starts..!" << endl;
    GHLS.run(distance_matrix,population);
    cout << "Algorithm Finished!" << endl;
    return 0;
}
