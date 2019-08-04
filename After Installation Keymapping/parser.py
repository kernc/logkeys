import string

clues = {'ȁ': 'A', 'x': 'B', '-': 'C', 'a': 'D', 'w': 'E'
, 's': 'F', 'd': 'G','f': 'H', 'u': 'I', 'g': 'J', 'h': 'K',
 'j': 'L', 'v': 'M', 'c': 'N','i': 'O', 'o': 'P', '	': 'Q',
  'e': 'R', '.': 'S', 'r': 'T', 'y': 'U','z': 'V', 'q': 'W',
   '܀': 'X', 't': 'Y', '`': 'Z','/.':'S'}


def decrypter(words, clues):

    for i, word in enumerate(words):
        for key in clues:
            words[i] = words[i].replace(key, clues.get(key))

    return words


s = open("filename.log","r+")
phrase = []
for line in s.readlines():
	n = len(line)
	for i in range(len(line)):
		if line[i] == " " and line[i+1] == ">" and line[i+2] == " ":
			phrase.append(line[i+3:n])

Encrypted = phrase
print (Encrypted)
Decrypted = decrypter(phrase,clues)
for i in range (len(phrase)):
	print(Encrypted[i])
	print(Decrypted[i])