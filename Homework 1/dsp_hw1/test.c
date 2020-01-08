#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hmm.h"

#define NUMBER_OF_MODEL 5
#define MAX_NUMBER_OF_OBSERVATION_SEQUENCE 2500
#define MAX_LENGTH_OF_OBSERVATION_SEQUENCE 51

struct Result
{
	int index;
	double likelihood;
};

typedef struct Result Result;

Result Viterbi_algorithm(HMM hmm[NUMBER_OF_MODEL] , char *observation_sequence , int length_of_observation_sequence)
{
	Result result = {0};

	for (int i = 0 ; i < NUMBER_OF_MODEL ; i++)
	{
		double delta[MAX_LENGTH_OF_OBSERVATION_SEQUENCE][MAX_STATE];
		for (int j = 0 ; j < hmm[i].state_num ; j++)
			delta[0][j] = hmm[i].initial[j] * hmm[i].observation[observation_sequence[0] - 'A'][j];
		for (int j = 1 ; j < length_of_observation_sequence ; j++)
			for (int k = 0 ; k < hmm[i].state_num ; k++)
			{
				double max_likelihood = 0;
				for (int l = 0 ; l < hmm[i].state_num ; l++)
					if (delta[j - 1][l] * hmm[i].transition[l][k] > max_likelihood)
						max_likelihood = delta[j - 1][l] * hmm[i].transition[l][k];

				delta[j][k] = max_likelihood * hmm[i].observation[observation_sequence[j] - 'A'][k];
			}

		double max_likelihood = 0;
		for (int j = 0 ; j < hmm[i].state_num ; j++)
			if (delta[length_of_observation_sequence - 1][j] > max_likelihood)
				max_likelihood = delta[length_of_observation_sequence - 1][j];

		if (max_likelihood > result.likelihood)
		{
			result.index = i;
			result.likelihood = max_likelihood;
		}
	}

	return result;
}

void testHMM(HMM hmm[NUMBER_OF_MODEL] , char *file_name_1 , char *file_name_2)
{
	int number_of_observation_sequence = 0;
	int length_of_observation_sequence;
	char observation_sequence[MAX_NUMBER_OF_OBSERVATION_SEQUENCE][MAX_LENGTH_OF_OBSERVATION_SEQUENCE] = {{0}};

	FILE *testing_data = fopen(file_name_1 , "r");
	while (fscanf(testing_data , "%s" , observation_sequence[number_of_observation_sequence]) != EOF)
		number_of_observation_sequence++;
	length_of_observation_sequence = strlen(observation_sequence[0]);
	fclose(testing_data);

	FILE *dump_file = fopen(file_name_2 , "w");
	for (int i = 0 ; i < number_of_observation_sequence ; i++)
	{
		Result result = Viterbi_algorithm(hmm , observation_sequence[i] , length_of_observation_sequence);
		fprintf(dump_file , "%s %e\n" , hmm[result.index].model_name , result.likelihood);
	}
	fclose(dump_file);

	return;
}

int main(int argc , char **argv)
{
	HMM hmm[NUMBER_OF_MODEL];
	load_models(argv[1] , hmm , NUMBER_OF_MODEL);
	testHMM(hmm , argv[2] , argv[3]);
	return 0;
}
