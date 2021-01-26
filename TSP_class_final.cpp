#include <iostream>
#include <cstdlib>
#include <ctime>
#include "ac_int.h"

#include "mc_scverify.h"

using namespace std;

//TODO: Name ALL loops OK
//TODO: Assign CSS_BLOCKS OK
//TODO: Assign pragmas OK

static const int populationSize = 200;
static const int numberOfNodes = 100;
static const int maxGenerations = 1000;

// assign class as top
#pragma hls_design top
class Genetic_HLS {
public:
    // Constructor
    Genetic_HLS();

    // ----- INITIALIZE -----
    void populationInit(ac_int<11, false>(&population)[populationSize][numberOfNodes]) {
        // This function
        // input population array, distances array
        //ac_int<length, false> population[numberOfNodes];

        initPOP: for (int i = 0; i < populationSize; i++) {
            initNODES: for (int j = 0; j < numberOfNodes; j++) {
                population[i][j] = j;
            }

            // Fisher-Yates' shuffling
            short right;
            short index;
            short len;
            ac_int<11, false> temp;


            initSHUFLE: for (int l = 1; l < (numberOfNodes - 1); l++) {

                right = min(l + 20 + 1, numberOfNodes - 1);
                len = l - right;
                index = l + (rand() % len);
                temp = population[i][l];
                population[i][l] = population[i][index];
                population[i][index] = temp;
            }
        }
    }

    // ----- To sort based on column in descending order -----
    void sortByColumn(ac_int<32, false>(&scores)[populationSize], ac_int<11, false>(&population)[populationSize][numberOfNodes]) {
        //This function

        ac_int<32, false> tempScore;
        ac_int<11, false> temp_pop[numberOfNodes];

        sortPOPI: for (int i = 0; i < populationSize; i++)
        {
            sortPOPJ: for (int j = i + 1; j < populationSize; j++)
            {
                if (scores[i] >= scores[j])
                {
                    tempScore = scores[i];
                    scores[i] = scores[j];
                    scores[j] = tempScore;

                    // sort the population
                  sortNODES:  for (int k = 0; k < numberOfNodes; k++) {
                        temp_pop[k] = population[i][k];
                        population[i][k] = population[j][k];
                        population[j][k] = temp_pop[k];
                    }
                }
            }
        }
    }

    // ----- FITNESS -----
    void fitness(ac_int<32, false>(&scores)[populationSize], ac_int<11, false>(&distances)[numberOfNodes][numberOfNodes], ac_int<11, false>(&population)[populationSize][numberOfNodes]) {
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
        sortByColumn(scores, population);

    }

    // ----- CROSSOVER -----
    void crossover(ac_int<11, false>(&population)[populationSize][numberOfNodes]) {
        // This function
        // Select the mating pool based on score
        // top 25% of scores keep it intact]
        // second 25% of the population is being crossovered
        // last 2 25% of the population are being randomly regenerated

        short point1;
        short point2;
        short size;
        ac_int<11, false> swapGenes;

        // crossover second quarter of the population
        crossover: for (int i = (populationSize * 0.25); i < (populationSize * 0.5); i++) {

            point1 = rand() % (numberOfNodes - 10);
            point2 = rand() % (numberOfNodes - point1) + point1;
            size = point2 - point1;

            crossoverSWAP: for (int j = point1; j < point2 / 2; ++j) {
                swapGenes = population[i][j];
                population[i][j] = population[i][size - j];
                population[i][size - j] = swapGenes;
            }
        }
        // Regenerate 50% of the population, last 2 4ths of the population
        crossoverREG: for (int i = (populationSize * 0.5); i < populationSize; i++) {

            regINIT: for (int j = 0; j < numberOfNodes; j++) {
                population[i][j] = j;
            }
            // Fisher-Yates' shuffling
            short right;
            short index;
            short len;
            ac_int<11, false> temp;

            //   |----- let the starting city be city: 0
            //   v
            regSHUFLE: for (int l = 1; l < (numberOfNodes - 1); l++) {

                right = min(l + 20 + 1, numberOfNodes - 1);
                len = l - right;
                index = l + (rand() % len);
                temp = population[i][l];
                population[i][l] = population[i][index];
                population[i][index] = temp;
            }

        }
    }

    // -----  MUTATION -----
    void mutate(ac_int<11, false>(&population)[populationSize][numberOfNodes]) {

        short index;
        short point1;
        short point2;
        short size;
        short start = populationSize * 0.25;
        ac_int<11, false> swapGenes;

        mutationPOINTS: for (int i = 0; i < 20; i++) {

            index = rand() % (populationSize - start) + (populationSize * 0.25);
            point1 = rand() % (numberOfNodes - 10);
            point2 = rand() % (numberOfNodes - point1) + point1;
            size = point2 - point1;

           mutationSWAP:  for (int j = point1; j < point2 / 2; ++j) {
                swapGenes = population[index][j];
                population[index][j] = population[index][size - j];
                population[index][size - j] = swapGenes;
            }
        }
    }
    // assign genetic function as the interface
    #pragma hls_design interface
    void CCS_BLOCK(genetic) (ac_int<11, false>(&distance_matrix)[numberOfNodes][numberOfNodes]) {
        // -Initialize Population-
        ac_int<11, false> population[populationSize][numberOfNodes];
        populationInit(population);
        int max = 1000000;
        // DONE: scores HAS to be larger(in bits) because it cuts off the distances sum after some point
        ac_int<32, false> scores[populationSize];
        geneticGENERATIONS: for (int i = 0; i < maxGenerations; i++) {
            fitness(scores, distance_matrix, population);
            if (scores[0] < max) {
                max = scores[0];
                cout << "Generation " << i << " - Best Score: " << scores[0] << endl;
            }
            crossover(population);
            mutate(population);
        }
    }

};

CCS_MAIN(int argc, char* argv[]) {
    srand(time(0));

    Genetic_HLS GHLS;
    //create distance matrix for fully connected graph
    ac_int<11, false> distance_matrix[numberOfNodes][numberOfNodes];
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
    GHLS.genetic(distance_matrix);
    cout << "Algorithm Finished!" << endl;
    CCS_RETURN(0);
}
