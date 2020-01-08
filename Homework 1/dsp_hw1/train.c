#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hmm.h"

#define MAX_NUMBER_OF_OBSERVATION_SEQUENCE 10000
#define MAX_LENGTH_OF_OBSERVATION_SEQUENCE 51

void forward_algorithm(HMM *hmm , char *observation_sequence , int length_of_observation_sequence , double alpha[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE])
{
	for (int i = 0 ; i < hmm->state_num ; i++)
		alpha[0][i] = hmm->initial[i] * hmm->observation[observation_sequence[0] - 'A'][i];
	for (int i = 1 ; i < length_of_observation_sequence ; i++)
		for (int j = 0 ; j < hmm->state_num ; j++)
		{
			alpha[i][j] = 0;
			for (int k = 0 ; k < hmm->state_num ; k++)
				alpha[i][j] += alpha[i - 1][k] * hmm->transition[k][j] * hmm->observation[observation_sequence[i] - 'A'][j];
		}

	return;
}

void backward_algorithm(HMM *hmm , char *observation_sequence , int length_of_observation_sequence , double beta[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE])
{
	for (int i = 0 ; i < hmm->state_num ; i++)
		beta[length_of_observation_sequence - 1][i] = 1;
	for (int i = length_of_observation_sequence - 2 ; i >= 0 ; i--)
		for (int j = 0 ; j < hmm->state_num ; j++)
		{
			beta[i][j] = 0;
			for (int k = 0 ; k < hmm->state_num ; k++)
				beta[i][j] += hmm->transition[j][k] * hmm->observation[observation_sequence[i + 1] - 'A'][k] * beta[i + 1][k];
		}

	return;
}

void calculate_gamma(HMM *hmm , char *observation_sequence , int length_of_observation_sequence , int number_of_observation_sequence , double alpha[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] , double beta[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] , double gamma[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] , double total_gamma_1[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] , double total_gamma_2[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_OBSERV])
{
	for (int i = 0 ; i < length_of_observation_sequence ; i++)
	{
		double sum = 0;
		for (int j = 0 ; j < hmm->state_num ; j++)
			sum += alpha[i][j] * beta[i][j];

		for (int j = 0 ; j < hmm->state_num ; j++)
		{
			gamma[i][j] = alpha[i][j] * beta[i][j] / sum;
			total_gamma_1[i][j] += gamma[i][j] / number_of_observation_sequence;
			total_gamma_2[i][j][observation_sequence[i] - 'A'] += gamma[i][j] / number_of_observation_sequence;
		}
	}

	return;
}

void calculate_epsilon(HMM *hmm , char *observation_sequence , int length_of_observation_sequence , int number_of_observation_sequence , double alpha[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] , double beta[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] , double epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE] , double total_epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE])
{
	for (int i = 0 ; i < length_of_observation_sequence - 1 ; i++)
	{
		double sum = 0;
		for (int j = 0 ; j < hmm->state_num ; j++)
			for (int k = 0 ; k < hmm->state_num ; k++)
				sum += alpha[i][j] * hmm->transition[j][k] * hmm->observation[observation_sequence[i + 1] - 'A'][k] * beta[i + 1][k];

		for (int j = 0 ; j < hmm->state_num ; j++)
			for (int k = 0 ; k < hmm->state_num ; k++)
			{
				epsilon[i][j][k] = alpha[i][j] * hmm->transition[j][k] * hmm->observation[observation_sequence[i + 1] - 'A'][k] * beta[i + 1][k] / sum;
				total_epsilon[i][j][k] += epsilon[i][j][k] / number_of_observation_sequence;
			}
	}

	return;
}

void updateHMM(HMM *hmm , int length_of_observation_sequence , double total_gamma_1[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] , double total_gamma_2[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_OBSERV] , double total_epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE])
{
	for (int i = 0 ; i < hmm->state_num ; i++)
		hmm->initial[i] = total_gamma_1[0][i];

	for (int i = 0 ; i < hmm->state_num ; i++)
		for (int j = 0 ; j < hmm->state_num ; j++)
		{
			double sum_1 = 0 , sum_2 = 0;
			for (int k = 0 ; k < length_of_observation_sequence - 1 ; k++)
			{
				sum_1 += total_epsilon[k][i][j];
				sum_2 += total_gamma_1[k][i];
			}

			hmm->transition[i][j] = sum_1 / sum_2;
		}

	for (int i = 0 ; i < hmm->observ_num ; i++)
		for (int j = 0 ; j < hmm->state_num ; j++)
		{
			double sum_1 = 0 , sum_2 = 0;
			for (int k = 0 ; k < length_of_observation_sequence ; k++)
			{
				sum_1 += total_gamma_2[k][j][i];
				sum_2 += total_gamma_1[k][j];
			}

			hmm->observation[i][j] = sum_1 / sum_2;
		}

	return;
}

void Baum_Welch_algorithm(HMM *hmm , int number_of_observation_sequence , int length_of_observation_sequence , char observation_sequence[MAX_OBSERV][MAX_LENGTH_OF_OBSERVATION_SEQUENCE] , int iteration)
{
	for (int i = 0 ; i < iteration ; i++)
	{
		double alpha[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE];
		double beta[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE];
		double gamma[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE];
		double epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE];
		double total_gamma_1[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE] = {{0}};
		double total_gamma_2[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_OBSERV] = {{{0}}};
		double total_epsilon[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE][MAX_STATE] = {{{0}}};

		for (int j = 0 ; j < number_of_observation_sequence ; j++)
		{
			forward_algorithm(hmm , observation_sequence[j] , length_of_observation_sequence , alpha);
			backward_algorithm(hmm , observation_sequence[j] , length_of_observation_sequence , beta);
			calculate_gamma(hmm , observation_sequence[j] , length_of_observation_sequence , number_of_observation_sequence , alpha , beta , gamma , total_gamma_1 , total_gamma_2);
			calculate_epsilon(hmm , observation_sequence[j] , length_of_observation_sequence , number_of_observation_sequence , alpha , beta , epsilon , total_epsilon);
		}

		updateHMM(hmm , length_of_observation_sequence , total_gamma_1 , total_gamma_2 , total_epsilon);
	}

	return;
}

void trainHMM(HMM *hmm , char *file_name , int iteration)
{
	int number_of_observation_sequence = 0;
	int length_of_observation_sequence;
	char observation_sequence[MAX_NUMBER_OF_OBSERVATION_SEQUENCE][MAX_LENGTH_OF_OBSERVATION_SEQUENCE] = {{0}};

	FILE *training_data = fopen(file_name , "r");
	while (fscanf(training_data , "%s" , observation_sequence[number_of_observation_sequence]) != EOF)
		number_of_observation_sequence++;
	length_of_observation_sequence = strlen(observation_sequence[0]);
	fclose(training_data);

	Baum_Welch_algorithm(hmm , number_of_observation_sequence , length_of_observation_sequence , observation_sequence , iteration);

	return;
}

int main(int argc , char **argv)
{
	HMM hmm;
	loadHMM(&hmm , argv[2]);
	trainHMM(&hmm , argv[3] , atoi(argv[1]));
	FILE *dump_file = fopen(argv[4] , "w");
	dumpHMM(dump_file , &hmm);
	fclose(dump_file);
	return 0;
}
