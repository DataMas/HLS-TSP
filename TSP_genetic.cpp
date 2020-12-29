#include <iostream>
#include <cstdlib>
#include <ctime>
#include "ac_int.h";
#include "ac_fixed.h";
using namespace std;

const int populationSize = 100;
const int numberOfNodes = 100;
const int length = 7;

struct individual {
	ac_int<length, false> gnome[numberOfNodes];
	ac_int<length, false> fittness;
};


void randomNum(int start, int finish, ac_int<length,false> &gene) {
	// This function returns a random number between "start" and "finish"
	// It is used for chromosome initialization with random genes

	gene = start + rand() % (start - finish);
}

bool geneRepeat(individual<length> chromosome, ac_int<length, false> gene) {
	// This function checks if a given gene is allready in a given chromosome
	// We want every gene to occur once in every chromosome

	for (int i = 0; i < chromosome.gnome.size(); i++) {

		if (chromosome[i] == gene) {

			return true;
		}
		return false;
	}
}

void populationInit(ac_int<length,false> population[populationSize][numberOfNodes], int numberOfNodes, int populationSize) {
	// This function initializes the chromosomes of the population at arandom

	for (int i = 0; i < populationSize; i++) {

		for (int j = 0; j < numberOfNodes; j++) {

			do {

				randomNum(0, numberOfNodes, population[i][j]);
			}while ( geneRepeat(population[i], population[i][j]) );
		}
	}
}


/*
void genetic() {

	ac_int<number_of_nodes, false> chromosome;
	srand(time(0));
	for (int i = 0; i < number_of_nodes; i++) {
		int temp = rand() % 2;
		//cout << temp;
		chromosome[i] = temp;
	}

	int max_generations = 100;
	int max_epochs = 10;
	int population = 100;
	int mutation_rate = 0.2;

	for (int generation = 0; generation < max_generations; generation++) {

	}
}*/

