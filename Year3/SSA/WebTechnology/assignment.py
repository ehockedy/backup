#The first link on web tech assignment tab is useful for SVD

import copy
import numpy
import re

unwantedWords = ['a', 'all', 'an', 'and', 'are', 'as', 'at', 'be', 'by', 'for',
                 'from', 'had', 'have', 'i', 'in', 'is', 'it', 'not', 'of',
                 'on', 'or', 'said', 'that', 'that', 'the', 'then', 'there', 'they', 'this',
                 'to', 'too', 'was', 'we', 'were', 'will', 'with', 'you', 'your', '']

def parseText(text, wordsOut, sentencesOut): #text is the document to be parsed, wordsOut is the list of all words in the document, sentencesOut is the list of all sentences in the document
    wordsOutTemp = []
    text = text.translate({ord(c): None for c in '\n",;:@#()[]-'}) #Remove any unwanted characters
    sentences = re.split('\.|\?|!', text) #Split into sentences. Sentences are considered to end with a . ! or ?
    for s in sentences: #For each sentence
        words = s.lower().split(' ') #Get the individual words, convert it to lowercase so can be used for comparison
        sentence = copy.copy(words) #Make a copy so we dont remove something when it is the current thing being iterated in the for loop
        for word in sentence: #For each word in the sentence
            if word in unwantedWords: #If the word is one we want to get rid of
                words.remove(word) #Remove it from the original sentence
            else:
                wordsOutTemp.append(word) #Add it to the list of words
        if words != []:
            sentencesOut.append(words) #Update the inputted array that stores all the sentences
    for w in wordsOutTemp: #Keep only the words that appear in more than one sentence
        if wordsOutTemp.count(w) > 1 and w not in wordsOut:
            wordsOut.append(w)
    
def buildCountMatrix(wordsIn, sentencesIn, matrix):
    for s in sentencesIn: #Go through each sentence
        count = [] #This will hold the count
        for w in wordsIn:
            count.append(s.count(w))
        matrix.append(count)

def printMatrix(w, mat):
    for m in range(0, len(mat[0])): #Go through each word
        print(w[m], end=": ") #Print the mth word
        for i in range(0,12-len(w[m])): #Makes the columns in line
            print(" ", end="")
        for i in range(0, len(mat)): #Print the number of occurences of this word in each line
            print(mat[i][m], end="")
            for i in range(0,3-len(str(mat[i][m]))): #Makes the columns in line
                print(" ", end="")
        print("")


f = open("LOTFshort.txt", 'r')
doc = f.read()

words = []
sentences = []
countMatrix = []

parseText(doc, words, sentences)
buildCountMatrix(words, sentences, countMatrix)
printMatrix(words, countMatrix)


#test = []
#test.append(orderedWordFreqs)
a,b,c = numpy.linalg.svd(countMatrix)
#print(b)


