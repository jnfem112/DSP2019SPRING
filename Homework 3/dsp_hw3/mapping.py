import sys

file = open(sys.argv[1] , 'r' , encoding = 'big5hkscs')
dictionary = dict()
for line in file:
	temp = line.split()
	character = temp[0]
	Zhuyin_list = temp[1].split('\\')
	for Zhuyin in Zhuyin_list:
		if Zhuyin[0] not in dictionary:
			dictionary[Zhuyin[0]] = set()
		dictionary[Zhuyin[0]].add(character)

file = open(sys.argv[2] , 'w' , encoding = 'big5hkscs')
for (Zhuyin , character_set) in dictionary.items():
	character_list = list(character_set)
	print(Zhuyin + '\t' + ' '.join(character_list) , file = file)
	for character in character_list:
		print(character + '\t' + character , file = file)
