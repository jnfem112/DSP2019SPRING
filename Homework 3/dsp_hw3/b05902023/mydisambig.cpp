#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Vocab.h"
#include "VocabMap.h"
#include "Ngram.h"
#include "Prob.h"

#define MAX_NUMBER_OF_STRING 128
#define MAX_LENGTH_OF_STRING 512
#define MAX_NUMBER_OF_CANDIDATE 1024

void list_candidate(VocabString string_BIG5[MAX_LENGTH_OF_STRING] , int length_of_string_BIG5 , VocabIndex candidate[MAX_LENGTH_OF_STRING][MAX_NUMBER_OF_CANDIDATE] , int number_of_candidate[MAX_LENGTH_OF_STRING] , Vocab Zhuyin , Vocab character , VocabMap map)
{
	candidate[0][0] = character.getIndex("<s>");
	candidate[length_of_string_BIG5 + 1][0] = character.getIndex("</s>");
	number_of_candidate[0] = number_of_candidate[length_of_string_BIG5 + 1] = 1;
	for (int i = 1 ; i <= length_of_string_BIG5 ; i++)
	{
		VocabIndex temp_candidate;
		Prob temp_probability;
		VocabMapIter iterator(map , Zhuyin.getIndex(string_BIG5[i]));

		iterator.init();
		while (iterator.next(temp_candidate , temp_probability))
		{
			candidate[i][number_of_candidate[i]] = temp_candidate;
			number_of_candidate[i]++;
		}
	}

	return;
}

void dynamic_programming(int length_of_string_BIG5 , VocabIndex candidate[MAX_LENGTH_OF_STRING][MAX_NUMBER_OF_CANDIDATE] , int number_of_candidate[MAX_LENGTH_OF_STRING] , LogP max_probability[MAX_LENGTH_OF_STRING][MAX_NUMBER_OF_CANDIDATE] , int back_tracking[MAX_LENGTH_OF_STRING][MAX_NUMBER_OF_CANDIDATE] , Vocab character , Vocab bigram_vocabulary , Ngram bigram)
{
	for (int i = 1 ; i <= length_of_string_BIG5 ; i++)
		for (int j = 0 ; j < number_of_candidate[i] ; j++)
		{
			VocabIndex current_candidate , previous_candidate;
			LogP probability;
			max_probability[i][j] = LogP_Zero;

			current_candidate = bigram_vocabulary.getIndex(character.getWord(candidate[i][j]));
			if (current_candidate == Vocab_None)
				current_candidate = bigram_vocabulary.getIndex(Vocab_Unknown);
				
			for (int k = 0 ; k < number_of_candidate[i - 1] ; k++)
			{
				previous_candidate = bigram_vocabulary.getIndex(character.getWord(candidate[i - 1][k]));
				if (previous_candidate == Vocab_None)
					previous_candidate = bigram_vocabulary.getIndex(Vocab_Unknown);
				
				VocabIndex context[2] = {previous_candidate , Vocab_None};
				probability = max_probability[i - 1][k] + bigram.wordProb(current_candidate , context);
				if (probability > max_probability[i][j])
				{
					max_probability[i][j] = probability;
					back_tracking[i][j] = k;
				}
			}
		}

	return;
}

void print_result(int length_of_string_BIG5 , VocabIndex candidate[MAX_LENGTH_OF_STRING][MAX_NUMBER_OF_CANDIDATE] , Vocab character , int back_tracking[MAX_LENGTH_OF_STRING][MAX_NUMBER_OF_CANDIDATE])
{
	VocabString result[MAX_LENGTH_OF_STRING] = {0};
	int j = 0;
	for (int i = length_of_string_BIG5 ; i > 0 ; i--)
	{
		result[i - 1] = character.getWord(candidate[i][j]);
		j = back_tracking[i][j];
	}

	printf("<s> ");
	for (int i = 0 ; i < length_of_string_BIG5 ; i++)
		printf("%s " , result[i]);
	printf("</s>\n");

	return;
}

void translate(char string[MAX_LENGTH_OF_STRING] , Vocab Zhuyin , Vocab character , Vocab bigram_vocabulary , VocabMap map , Ngram bigram)
{
	VocabString string_BIG5[MAX_LENGTH_OF_STRING] = {0};
	int length_of_string_BIG5 = Vocab::parseWords(string , string_BIG5 + 1 , MAX_LENGTH_OF_STRING);
	VocabIndex candidate[MAX_LENGTH_OF_STRING][MAX_NUMBER_OF_CANDIDATE] = {{0}};
	int number_of_candidate[MAX_LENGTH_OF_STRING] = {0};
	LogP max_probability[MAX_LENGTH_OF_STRING][MAX_NUMBER_OF_CANDIDATE] = {{0}};
	int back_tracking[MAX_LENGTH_OF_STRING][MAX_NUMBER_OF_CANDIDATE] = {{0}};

	list_candidate(string_BIG5 , length_of_string_BIG5 , candidate , number_of_candidate , Zhuyin , character , map);
	dynamic_programming(length_of_string_BIG5 , candidate , number_of_candidate , max_probability , back_tracking , character , bigram_vocabulary , bigram);
	print_result(length_of_string_BIG5 , candidate , character , back_tracking);

	return;
}

int main(int argc , char **argv)
{
	Vocab Zhuyin , character , bigram_vocabulary;

	VocabMap map(Zhuyin , character);
	{
		File file(argv[4] , "r");
		map.read(file);
		file.close();
	}

	character.addWord("<s>");
	character.addWord("</s>");

	Ngram bigram(bigram_vocabulary , atoi(argv[8]));
	{
		File file(argv[6] , "r");
		bigram.read(file);
		file.close();
	}

	int number_of_string = 0;
	char string[MAX_NUMBER_OF_STRING][MAX_LENGTH_OF_STRING] = {{0}} , *temp_string;
	File file(argv[2] , "r");
	while (temp_string = file.getline())
	{
		strcpy(string[number_of_string] , temp_string);
		number_of_string++;
	}
	file.close();

	for (int i = 0 ; i < number_of_string ; i++)
		translate(string[i] , Zhuyin , character , bigram_vocabulary , map , bigram);

	return 0;
}
