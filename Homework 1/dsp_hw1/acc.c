#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH_OF_MODEL_NAME 256

int main(int argc , char **argv)
{
	FILE *result = fopen(argv[1] , "r");
	FILE *answer = fopen(argv[2] , "r");
	FILE *dump_file = fopen(argv[3] , "w");

	double likelihood;
	char model_1[MAX_LENGTH_OF_MODEL_NAME] = {0} , model_2[MAX_LENGTH_OF_MODEL_NAME] = {0};
	int count = 0 , all = 0;

	while (fscanf(result , "%s %lf" , model_1 , &likelihood) != EOF && fscanf(answer , "%s" , model_2) != EOF)
	{
		all++;
		if (strcmp(model_1 , model_2) == 0)
			count++;
	}

	fprintf(dump_file , "%f\n" , (double)count / (double)all);

	fclose(result);
	fclose(answer);
	fclose(dump_file);
	return 0;
}
