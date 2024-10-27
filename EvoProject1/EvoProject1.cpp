#include <iostream>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <math.h> //to use exp() function
using namespace std;

const int GENE = 8;
const int POP_SIZE = 60;
const double XOVER_PROBABILITY = 0.9;
const double MUTATION_PROBABILITY = 0.1;
const int linksDistance[] = { 12, 20, 14, 50, 15, 30, 75, 25 };
const int MaxGen = 40;
const int tableData[8][8] =
{
	//bandwidth, cost, range start, range end
	{10 , 100 , 0, 10},//first row of the table
	{20 , 120 , 11, 30},
	{50 , 150 , 31, 50},
	{100 , 200 , 51, 70},
	{200 , 250 , 71, 90},
	{300 , 300 , 91, 110},
	{500 , 400 , 111, 130},
	{1000 , 500 , 131, 150},
};
const int totalBudget = 1600;
const float weightage1 = 0.5;
const float weightage2 = 0.5;
const float penalty = 0.5;
const float constant = 0.001;//use to control the decay rate

int chromosome[POP_SIZE][GENE];
int totalBandwidthDemand = NULL;
int minCostDifference = NULL;
float fitness[POP_SIZE];
int parents[2][GENE];
int children[2][GENE];
int chromoNewCounter;
int newChromosome[POP_SIZE][GENE];
ofstream averageFitnessFile, bestFitnessFile, bestChromoFile;
float averageFitness, bestFitness = -1;
int bestChromo[GENE];

int getBandwidthDemand(int link) {
	for (int i = 0; i < 8; i++) {
		//if the distance of the link is in the range of from start range and end range
		if (linksDistance[link] >= tableData[i][2] && linksDistance[link] <= tableData[i][3]) {
			return tableData[i][0];
		}
	}
}

void initializePopulation() {
	for (int c = 0; c < POP_SIZE; c++) {
		for (int g = 0; g < GENE; g++) {
			int randomNumber = rand() % GENE;
			chromosome[c][g] = randomNumber;
		}
	}
	//need to be calculated one time only
	//calculate the bandwidth demand here 
	for (int g = 0; g < GENE; g++) {
		//call function to get the bandwidth demand
		totalBandwidthDemand += getBandwidthDemand(g);
	}
	//determine the minimum difference between budget cost and minimum possible cost
	minCostDifference = totalBudget - tableData[0][1] * GENE;


}
void printChromosome() {
	for (int c = 0; c < POP_SIZE; c++) {
		cout << "\tChromosome" << c << " ";
		for (int g = 0; g < GENE; g++) {
			cout << "\t" << chromosome[c][g];
		}
		cout << endl;
	}

	cout << endl << endl;
}
void evaluateChromosome() {
	double totalBandwidthFitness = 0;
	int totalCost = 0;
	int totalBandwidthAllocated = 0;
	int linkDemand = 0;
	int linkAllocated = 0;
	double linkBandwidthFitness = 0;
	for (int c = 0; c < POP_SIZE; c++) {
		totalCost = totalBandwidthAllocated = totalBandwidthFitness = 0;
		cout << "\tChr" << c << " : ";
		for (int g = 0; g < GENE; g++) {
			totalCost += tableData[chromosome[c][g]][1];//fetch the cost in column 2 based on the bandwidth
			totalBandwidthAllocated += tableData[chromosome[c][g]][0];

			linkDemand = getBandwidthDemand(g);
			linkAllocated = tableData[chromosome[c][g]][0];
			linkBandwidthFitness = 0;
			//to determine whether each of the bandwidth allocated to the bandwidth meet the demand
			if (linkAllocated >= linkDemand) {
				linkBandwidthFitness = 1;
			}
			else {
				linkBandwidthFitness = 0;
			}
				
			totalBandwidthFitness += linkBandwidthFitness;
		}
			totalBandwidthFitness = (totalBandwidthFitness) / GENE;
			//determine the fitness value
			if (totalCost > totalBudget || totalBandwidthAllocated < totalBandwidthDemand) {
			//use the first fitness function
				fitness[c] = ((weightage1 * totalBandwidthFitness) + (weightage2 * (1 / exp(constant * (totalCost - totalBudget))))) * penalty;
			}
			else {
			//	//use the second fitness function
				fitness[c] = (weightage1 * totalBandwidthFitness) + (weightage2  * (abs(totalCost - totalBudget)) / minCostDifference);
			}

			cout << "\ttotalCost:\t" << totalCost << "\ttotalBandwidthAllocated:\t" << totalBandwidthAllocated << "\tfitnessValue:\t" << fitness[c] << endl;
		}
		cout << endl << endl;
}

void parentSelection() {
	//declare necessary variables
	int player1, player2, indexParents[2];
	for (int i = 0; i < 2; i++) {
		player1 = rand() % POP_SIZE;
		player2 = rand() % POP_SIZE;
		while (player1 == player2) {
			player2 = rand() % POP_SIZE;
		}

		if (fitness[player1] > fitness[player2]) {
			indexParents[i] = player1;
		}
		else {
			indexParents[i] = player2;
		}
		if (i == 1 && indexParents[0] == indexParents[1]) {
			i = 0;
			//cout << "p1 " << indexParents[0] << " p2 " << indexParents[1];
			//cout << "catch"<<endl;
			continue;
		}
		else {
			cout << "\n\tPlayer 1: " << player1 << " vs Player 2: " << player2 << endl;
			cout << "\n\tWinner: " << indexParents[i] << endl;
		}
	}

	for (int c = 0; c < 2; c++) {
		for (int g = 0; g < GENE; g++) {
			parents[c][g] = chromosome[indexParents[c]][g];
		}
	}
	cout << "\n\tParents: " << endl;
	for (int c = 0; c < 2; c++) {
		cout << "\n\tParent " << c + 1 << ": ";
		for (int g = 0; g < GENE; g++) {
			cout << parents[c][g] << " ";
		}
		cout << endl;
	}

}
void crossover2Point() {
	double randNumber;
	int xoverPoint1, xoverPoint2;
	//1. Copy both parent s chromosome to children chromosomes
	for (int c = 0; c < 2; c++) {
		for (int g = 0; g < GENE; g++) {
			children[c][g] = parents[c][g];
		}
	}
	//2. Generate a random number from 0-1. Make sure it is real value data type
	randNumber = double((rand() % 11) / 10.0);
	//3. If (2) less than crossover probability
	if (randNumber < XOVER_PROBABILITY) {
		//2.1 generate a random crossover point
		do {
			xoverPoint1 = rand() % GENE;
			xoverPoint2 = rand() % GENE;
		} while (xoverPoint2 <= xoverPoint1);

		cout << "\n\tCrossover will happen between index " << xoverPoint1 << " and " << xoverPoint2;

		//2.2 Crossover parent 1 and parent 2 to produce the children
		for (int g = xoverPoint1; g <= xoverPoint2; g++) {
			children[0][g] = parents[1][g];
			children[1][g] = parents[0][g];
		}
	}
	else {
		cout << "\n\tCrossover will not happen ";
	}
	//4. Print children 1 & 2
	for (int c = 0; c < 2; c++) {
		cout << "\n\tChildren " << c + 1 << ":";
		for (int g = 0; g < GENE; g++) {
			cout << children[c][g] << " ";
		}
	}
	cout << endl;
}
void mutation() {
	//1. Declare necessary variables
	double randNumber;
	//2. Declare mutation bit
	int mutBit;
	//3. For both children
	for (int c = 0; c < 2; c++) {
		//3.1. Generate number from 0-1 (real values)
		randNumber = double((rand() % 11) / 10.0);
		//3.2. If 3.1 less than mutation probability	
		if (randNumber < MUTATION_PROBABILITY) {
			//3.2.1. generate a mutation bit
			mutBit = rand() % GENE;
			//3.2.2. Print the mutation bit

			cout << "\n\tMutation will happen at index " << mutBit << " for children " << c + 1;
			//Randomly reset the integer/index
			int initialValue = chromosome[c][mutBit];
			do {
				initialValue = chromosome[c][mutBit];
				children[c][mutBit] = rand() % GENE;

			} while (initialValue == children[c][mutBit]);

		}
		else {
			cout << "\n\tMutation will not happen for children " << c + 1;
		}
		//End if

	}
	//End for
	//5. Print the mutated chromosomes
	for (int c = 0; c < 2; c++) {
		cout << "\n\tChildren " << c + 1 << ":";
		for (int g = 0; g < GENE; g++) {
			cout << children[c][g] << " ";
		}
	}

	cout << endl;

}

void survivalSelection() {
	//1. For each chromosome
	for (int c = 0; c < 2; c++) {
		//1.2. For each gene
		for (int g = 0; g < GENE; g++) {
			//1.3 Copy children to the survival chromosome array
			newChromosome[chromoNewCounter][g] = children[c][g];
		}
		chromoNewCounter++;
	}
	//2. Update array counter
	//3. Print chromosomes in the new population so far
	for (int c = 0; c < POP_SIZE; c++) {

		cout << "\t New Chromosome " << c << ": \t";
		for (int g = 0; g < GENE; g++)
		{

			if (c >= chromoNewCounter ) {
				//to show zeros for empty new chromosome 
				cout <<"0\t";
			}
			else {
				cout << newChromosome[c][g]<< "\t";
			}


		}
		cout << "\n";
	}
}

void copyChromosome()
{
	for (int c = 0; c < POP_SIZE; c++)
	{
		for (int g = 0; g < GENE; g++)
		{
			chromosome[c][g] = newChromosome[c][g];
		}
	}
}

void AverageFitnessCalculation()
{
	float totalFitness = 0;
	for (int c = 0; c < POP_SIZE; c++)
	{
		totalFitness = totalFitness + fitness[c];
	}

	averageFitness = totalFitness / POP_SIZE;

	cout << "\n\tAverage Fitness = " << averageFitness;
	averageFitnessFile << averageFitness << endl;
}

void recordBestFitness() {
	//1. Declare the bestChromosome data structure

	//2. For each chromosome
	for (int c = 0; c < POP_SIZE; c++) {
		//2.1. if (fitness current chromosome better than bestFitness){
		if (fitness[c] > bestFitness) {
			//2.1.1. bestFitness = fitness for the current chromosome
			bestFitness = fitness[c];
			//2.1.2. copy the chromosome to bestChromosome
			for (int g = 0; g < GENE; g++) {
				bestChromo[g] = chromosome[c][g];
			}
		}
	}

	//3. Print the bestFitness and bestChromosome to the screen
	cout << " \n\tBest Fitness = " << bestFitness << endl;
	bestFitnessFile << bestFitness << endl;
	//4. Print the bestFitness and bestChromosome to two separate files
	for (int g = 0; g < GENE; g++) {
		cout <<"\t" << bestChromo[g] << " ";
		bestChromoFile << bestChromo[g] << " ";
	}
	cout << endl;
	bestChromoFile << endl;
}

int main() {

	averageFitnessFile.open("averageFitness.txt");
	bestFitnessFile.open("bestFitness.txt");
	bestChromoFile.open("bestChromo.txt");

	srand(time(NULL));
	cout << "\tGA start" << endl;
	cout << "\tInitialization Population" << endl;
	initializePopulation();
	for (int gen = 0; gen < MaxGen; gen++)
	{
		cout << "\n\Generation " << gen + 1 << "------------------------------------------------------------------" << endl;
		printChromosome();

		cout << "\n\Evaluate Chromosome" << endl;
		evaluateChromosome();
		AverageFitnessCalculation();
		recordBestFitness();
		chromoNewCounter = 0; // Reset counter for each generation

		for (int j = 0; j < POP_SIZE / 2; j++) {

			cout << "\n\tParent Selection 2 Players" << endl;
			parentSelection();

			cout << "\n\tCross Over 2 point";
			crossover2Point();

			cout << "\nMutation";
			mutation();


			cout << "\nSurvival Selection" << endl;
			survivalSelection();
			//getchar();

		}
		copyChromosome();
		cout << "\n\Copy Chromosome" << endl;
	}
	averageFitnessFile.close();
	bestFitnessFile.close();
	bestChromoFile.close();
}