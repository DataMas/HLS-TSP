#include <iostream>
#include <cstdlib>
#include <ctime>
#include "ac_int.h";
using namespace std;

const int populationSize = 100;
const int numberOfNodes = 100;
const int length = 7;
const int maxGenerations = 100;

/*###################################################
Some usefull links
https://www.obitko.com/tutorials/genetic-algorithms/ga-basic-description.php
https://2ality.com/2013/03/permutations.html
https://www.codeproject.com/articles/16286/ai-simple-genetic-algorithm-ga-to-solve-a-card-pro
###################################################*

// **Might be useless in the end**
/*
void randomNum(int start, int finish, ac_int<length, false>& gene) {
	// This function returns a random number between "start" and "finish"
	// It is used for chromosome initialization with random genes

	gene = start + rand() % (start - finish);
}

bool geneRepeat(ac_int<length, false> gene, individual<length> chromosome) {
	// This function checks if a given gene is allready in a given chromosome
	// We want every gene to occur once in every chromosome

	for (int i = 0; i < numberOfNodes; i++) {

		if (chromosome.gnome[i] == gene) {

			return true;
		}
		return false;
	}
}*/

// ----- INITIALIZE -----
void populationInit(ac_int<11, false> (&distances) [populationSize][numberOfNodes]) {
	// This function
	// input population array, distances array
	ac_int<length, false> population[numberOfNodes];
	for (int i = 0; i < populationSize; i++) {

		for (int j = 0; j < numberOfNodes; j++) {

			population[i][j] = j;
		}

		// Fisher-Yates' shuffling
		short k;
		for (int l = 0; l < (numberOfNodes - 1); l++) {
		
			k = min(l + 5 + 1, numberOfNodes);
			ac_int<length, false> temp = population[i][l];
			population[i][l] = population[i][k];
			population[i][k] = temp;
		}
	}
}

// ----- To sort based on column in descending order -----
void sortByColumn(ac_int<11, false>(&vector)[populationSize][2]) {
	//This function

	ac_int<11, false> tempScore;
	ac_int<7, false> tempIndex;

	for (int i = 0; i < (populationSize - 1); i++) {

		if (vector[i][0] < vector[i + 1][0]) {

			tempScore = vector[i][0];
			tempIndex = vector[i][1];
			vector[i][0] = vector[i + 1][0];
			vector[i][1] = vector[i + 1][1];
			vector[i + 1][0] = tempScore;
			vector[i + 1][1] = tempIndex;
		}
	}
}

// ----- FITNESS -----
void fitness(ac_int<11, false> (&scores)[populationSize][2], ac_int<11,false> distances[numberOfNodes][numberOfNodes], ac_int<length,false> population[populationSize][numberOfNodes]) {
	//This function
	//input distances

	ac_int<length, false> city1;
	ac_int<length, false> city2;

	//calculate the sum of all the distances for a every chromosome
	for (int i = 0; i < populationSize; i++) {

		scores[i][0] = 0;

		for (int j = 0; j < numberOfNodes - 1; j++) {

			city1 = population[i][j];
			city2 = population[i][j + 1];
			scores[i][0] = scores[i][0] + distances[city1][city2];
			scores[i][1] = city1;
		}
	}
	//return a score vector, one score for every chromosome
	sortByColumn(scores);
}

// ----- CROSSOVER -----
void crossover(ac_int<length, false> (&population)[populationSize][numberOfNodes], ac_int<11, false> scores[populationSize]) {
	// This function
	// Select the mating pool based on score
	// top 25% of scores keep it intact]
	// second 25% of the population is being crossovered
	// last 2 25% of the population are being randomly regenerated

	short point1;
	short point2;
	short size;
	ac_int<length, false> swapGenes;

	// crossover 25% of the population, 2nd 2th of the population
	for (int i = (populationSize * 0.25); i < (populationSize * 0.5); i++) {

		point1 = rand() % (numberOfNodes - 10);
		point2 = rand() % (numberOfNodes - point1) + point1;
		size = point2 - point1;

		for (int j = point1; j < point2 / 2; ++j) {
			swapGenes = population[i][j];
			population[i][j] = population[i][size - j];
			population[i][size - j] = swapGenes;
		}
	}
	// Regenerate 50% of the population, last 2 4ths of the population
	for (int i = (populationSize * 0.5); i < populationSize; i++) {

		for (int j = 0; j < numberOfNodes; j++) {

			population[i][j] = j;
		}

		// Fisher-Yates' shuffling
		short k;
		for (int l = 0; l < (numberOfNodes - 1); l++) {

			k = min(l + 5 + 1, numberOfNodes);
			ac_int<length, false> temp = population[i][l];
			population[i][l] = population[i][k];
		}
	}
}

// -----  MUTATION -----
void mutate(ac_int<length, false>(&population)[populationSize][numberOfNodes]) {

	int index;
	short point1;
	short point2;
	short size;
	ac_int<length, false> swapGenes;


	for (int i = 0; i < 10; i++) {

		index = rand() % (populationSize - 25) + (populationSize * 0.25);
		point1 = rand() % (numberOfNodes - 10);
		point2 = rand() % (numberOfNodes - point1) + point1;
		size = point2 - point1;

		for (int j = point1; j < point2 / 2; ++j) {
			swapGenes = population[i][j];
			population[index][j] = population[index][size - j];
			population[index][size - j] = swapGenes;
		}
	}
}

void genetic(short distance_matrix[][numberOfNodes]) {
	populationInit(distance_matrix);
	ac_int<11, false> scores[populationSize][2];
	for (int i = 0; i < maxGenerations; i++) {
		
		fitness();
		cout << scores[0][0]<<endl;
		crossover();
		mutate();
	}
}

// ** NOT THE FINAL MAIN FUNCTION **
int main() {
	//create distance matrix for fully connected graph
	short distance_matrix[numberOfNodes][numberOfNodes];
	for (int i = 0; i < numberOfNodes; ++i) {
		for (int j = 0; j < numberOfNodes; ++j) {
			//distances vary from 40 + 1000 km
			if (i == j) { continue; }
			int distance = rand() % 1000 + 40;
			distance_matrix[i][j] = distance;
			distance_matrix[j][i] = distance;
		}
	}


	return 0;
}


