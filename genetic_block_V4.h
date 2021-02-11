#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include "ac_int.h"

#include "mc_scverify.h"

using namespace std;

static const int populationSize = 512;
static const int numberOfNodes = 128;
static const int maxGenerations = 100000;
static const int max_pop_survivors = populationSize*0.25;
static const int max_pop_crossover = populationSize*0.5;


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
    void populationInit(ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize], LSFR &RAND ) {

        initPOP: for (int i = 0; i < populationSize; i++) {
            initNODES: for (int j = 0; j < numberOfNodes; j++) {
                population[i][j] = j;
            }
//            // Fisher-Yates' shuffling
//            short right;
//            short index;
//            short len;
            ac_int<11, false> temp;
                
//        	// HLS optimized shuffling
//            initSHUFLE: for (int l = 1; l < (numberOfNodes - 1); l++) {
//
//                right = min(l + 20 + 1, numberOfNodes - 1);
//                len = l - right;
//                index = l + ( RAND.run() % len);
//                temp = population[i][l];
//                population[i][l] = population[i][index];
//                population[i][index] = temp;
//            }

            // 2nd try of shuffling
            ac_int<7, false> point1,point3,point5;
            ac_int<7, false> point2,point4,point6;
            ac_int<32, false> randomNumber;

            initSHUFFLE:for (int j = 0; j < 100; ++j) {
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

            // Keep the address of the chromosome's beginning
            populationAddresses[i] = &population[i][0];
        }
    }

    // ----- To sort based on column in descending order -----
    void sortByColumn(ac_int<32, false> scores[populationSize], ac_int<11, false> *populationAddresses[populationSize], LSFR &RAND) {

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
    void fitness(ac_int<32, false> scores[populationSize], ac_int<11, false> distances[numberOfNodes][numberOfNodes], ac_int<11, false> population[populationSize][numberOfNodes], ac_int<11, false> *populationAddresses[populationSize], LSFR &RAND) {
        // This function
        // input distances
        // scores:
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
        sortByColumn(scores, populationAddresses, RAND);
    }

    //#pragma_hls_design
    // ----- CROSSOVER -----
    void crossover(ac_int<11, false> *populationAddresses[populationSize], LSFR &RAND) {
        // This function
        // Select the mating pool based on score
        // top 25% of scores keep it intact]
        // second 25% of the population is being crossovered
        // last 2 25% of the population are being randomly regenerated

        ac_int<11, false> index_copy;
        ac_int<11, false> swapGenes;
        // crossover second quarter of the population
        cross: for (int i = (max_pop_survivors); i < (max_pop_crossover); i++) {
            index_copy = i - max_pop_survivors;
            // copy best genes to 2nd quarter of population
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
        // Regenerate 50% of the population last 2 4ths of the population
        crossoverREG: for (int i = (max_pop_crossover); i < populationSize; i++) {

            // Fisher-Yates' shuffling
            short right;
            short index;
            short len;
            ac_int<11, false> temp;

//            //                      |----- let the starting city be city: 0
//            //                      v
//            regSHUFLE: for (int l = 1; l < (numberOfNodes - 1); l++) {
//                right = min(l + 20 + 1, numberOfNodes - 1);
//                len = l - right;
//                index = l + (RAND.run() % len);
//                temp = *(populationAddresses[i]+l);
//                *(populationAddresses[i]+l) = *(populationAddresses[i]+index);
//                *(populationAddresses[i]+index) = temp;
//            }

            // 2nd try of shuffling
            ac_int<7, false> point1,point3,point5;
            ac_int<7, false> point2,point4,point6;
            ac_int<32, false> randomNumber;

            for (int j = 0; j < 100; ++j) {
                randomNumber = RAND.run();
                // range 0 - 63
                point1 = randomNumber.slc<6>(0);
                point2 = randomNumber.slc<6>(6);
                point1[0] = 1;  //force numbers to be >0
                point2[0] = 1;
                temp = *(populationAddresses[i]+point1);
                *(populationAddresses[i]+point1) = *(populationAddresses[i]+point2);
                *(populationAddresses[i]+point2) = temp;

                // range 0 - 127
                point3 = randomNumber.slc<7>(17);
                point4 = randomNumber.slc<7>(13);
                point3[0] = 1;  //force numbers to be >64
                point4[6] = 1;
                temp = *(populationAddresses[i]+point3);
                *(populationAddresses[i]+point3) = *(populationAddresses[i]+point4);
                *(populationAddresses[i]+point4) = temp;

                point5 = randomNumber.slc<7>(10);
                point6 = randomNumber.slc<7>(22);
                point5[0] = 1;  //force numbers to be >0
                point6[0] = 1;
                temp = *(populationAddresses[i]+point5);
                *(populationAddresses[i]+point5) = *(populationAddresses[i]+point6);
                *(populationAddresses[i]+point6) = temp;
            }
        }
    }
    
     // -----  MUTATION -----
    void mutate(ac_int<11, false> population[populationSize][numberOfNodes],  LSFR &RAND) {

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
            // leave the best two chromosomes as they are
            index[1]=1;

            swapGenes = population[index][point1];
            population[index][point1] = population[index][point2];
            population[index][point2] = swapGenes;
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
    void CCS_BLOCK(run)(ac_int<11, false> distance_matrix[numberOfNodes][numberOfNodes], ac_int<11, false> population[populationSize][numberOfNodes]) {

        LSFR RAND(12);
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
            gen.crossover(populationAddresses, RAND);
            gen.mutate(population, RAND);
        }
        /*fstream my_file;
        my_file.open("optimal.txt", ios::out);
        if (!my_file) {
            cout << "File not created!"<< endl;
        }
        else {*/
            cout << "File created successfully!"<< endl;
            cout << "Optimal path : ";
            for (int i = 0; i <numberOfNodes-1; ++i) {
               // my_file << *(populationAddresses[0]+i) << ",";
                cout << " -> "<< *(populationAddresses[0]+i) ;
            }
            cout << endl;
            //my_file.close();
        //}
    }

};
