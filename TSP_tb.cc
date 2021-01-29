#include <iostream>
#include <cstdlib>
#include <ctime>

#include "genetic.h"
#include "genetic_block.h"


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
