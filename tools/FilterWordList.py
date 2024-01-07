import os
import sys
import string

if (len(sys.argv) < 2):
#
	print("Missing required argument(s)");
	print("Usage: %s [input_file0] {{input_file1}} ..." % (sys.argv[0]));
	sys.exit();
#

numLetters = 0;

inputPath = sys.argv[1];
outputPath = None;
# print("inputPath: %s" % (inputPath));

allowedChars = set(string.ascii_lowercase + string.ascii_uppercase);
dictionary = {};

fIndex = 0;
for inputPath in sys.argv[1:]:
#
	isBlacklist = False;
	if (inputPath.startswith("-")):
	#
		isBlacklist = True;
		inputPath = inputPath[1:];
	#
	
	if (inputPath.startswith("@")):
	#
		outputPath = inputPath[1:];
		continue;
	#
	
	if (inputPath.startswith("#")):
	#
		numLetters = int(inputPath[1:]);
		print("Filtering to only %d letter words" % (numLetters));
		continue;
	#
	
	try:
	#
		with open(inputPath, "r") as inputFile:
		#
			numWordsInFile = 0;
			numValidWordsInFile = 0;
			for word in inputFile:
			#
				word = word.rstrip("\n\r");
				word = word.lower();
				# print("Word[%d]: \"%s\"" % (numValidWordsInFile, word));
				wordIsValid = True;
				
				if (not set(word) <= allowedChars):
				#
					# print("Word[%d]: \"%s\" has invalid letters" % (numValidWordsInFile, word));
					wordIsValid = False;
				#
				
				if (numLetters > 0 and len(word) != numLetters):
				#
					wordIsValid = False;
				#
				
				if (wordIsValid):
				#
					numValidWordsInFile += 1;
					if (isBlacklist):
					#
						if (word in dictionary):
						#
							print("Removing blacklist word[%d] \"%s\" in %s" % (numValidWordsInFile, word, inputPath));
							del dictionary[word];
						#
					#
					else:
					#
						if (word in dictionary):
						#
							print("Word[%d]: \"%s\" was discovered %d time(s) before" % (numValidWordsInFile, word, dictionary[word]));
							dictionary[word] = dictionary[word] + 1;
						#
						else:
						#
							dictionary[word] = 1;
						#
					#
				#
				
				numWordsInFile += 1;
			#
			else:
			#
				if (not isBlacklist):
				#
					print("Found %d/%d words in %s" % (numValidWordsInFile, numWordsInFile, inputPath));
				#
			#
		#
	#
	except Exception as e:
	#
		print("Failed to open %s: %s" % (inputPath, e));
	#
	fIndex += 1;
#


print("Dictionary contains %d words" % (len(dictionary)));
# for word in dictionary:
# #
# 	print("x%d \"%s\"" % (dictionary[word], word));
# #

if (outputPath == None):
#
	print("No output path was defined. Please give a path for us to write to by prefixing it with @");
	sys.exit();
#

with open(outputPath, "w") as outputFile:
#
	for word in dictionary:
	#
		outputFile.write(word + "\n");
	#
#
print("Wrote to \"%s\"" % (outputPath));
