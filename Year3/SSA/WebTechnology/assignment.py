import copy
import numpy
import re
import scipy.cluster.vq
import matplotlib.pyplot as plt

#Parse the text, remove unwanted words, store the sentences and the words 
def parseText(text, wordsOut, sentencesOut, unwantedWords): #text is the document to be parsed, wordsOut is the list of all words in the document, sentencesOut is the list of all sentences in the document
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

#Builds the count matrix
def buildCountMatrix(wordsIn, sentencesIn, matrix):
    for s in sentencesIn: #Go through each sentence
        count = [] #This will hold the count
        for w in wordsIn:
            count.append(float(s.count(w)))
        matrix.append(count)
    return numpy.array(matrix).transpose()

#Outputs the count matrix
def printMatrix(w, mat):
    for m in range(0, len(mat[0])): #Go through each word
        print(w[m], end=": ") #Print the mth word
        for i in range(0,12-len(w[m])): #Makes the columns in line
            print(" ", end="")
        for i in range(0, len(mat)): #Print the number of occurences of this word in each line
            print(mat[i][m], " ", end="")
            for i in range(0,3-len(str(mat[i][m]))): #Makes the columns in line
                print(" ", end="")
        print("")

#Converts a vector to a diagonal matrix. Used on S
def populateDiagonal(a, size):
    diag = []
    for i in range(0, size):
        diagrow = []
        for j in range(0, size):
            if(i==j):
                diagrow.append(float(a[i]))
            else:
                diagrow.append(float(0))
        diag.append(diagrow)
    return diag

#doSVD - decompoese the count matrix into the U, S and V matrices, and then reduce the dimensions to 3
#returns U, S, V
def doSVD(c):
    U,S,V = numpy.linalg.svd(c, False) #Split it up into the 3 matrices
    #U describes the relationships between the words in the document
    #V describes the relationships between the sentences in the document
    #The dimensions of these matrices can be reduced to filter out "noisy" relationships
    U = U[:,:3] #WORDS - all rows, 3 cols
    S = populateDiagonal(S, 3) 
    V = V[:3,:] #SENTENCES - 3 rows, all cols
    #If thr producet U*S*V was to be computed, a matrix similar to the original count matrix would be produced, but with som of the information removed.
    #This simplified matrix has filtered out the "noise" (i.e. the weaker relationships in the document)
    return U, S, V

#Applies k means clustering to the specified observations (sentences)
#Returns a list of class labels where label[i] is the class/label of sentence[i]
def getClusteredClasses(toplot, k):
    cluster, _ = scipy.cluster.vq.kmeans(toplot, 3)
    classes, _ = scipy.cluster.vq.vq(toplot, cluster) #Used the kclustered information to gt class labels
    return classes

#Plots the points from the specified matrix (in this case V)
#Colours each point such that the colour represents the custer it belongs to
#Can support up to 4 individual clusters.
#Cluster 1 = red
#Cluster 2 = blue
#Cluster 3 = green
#Cluster 4 = white
def plotClusters(mat, cl):
    for i in range(0,len(cl)):
        if cl[i] == 0: #The first class
            plt.scatter(mat[1,:][i],mat[2,:][i], c=[1,0,0]) #Has the red points
        elif cl[i] == 1: #Second class
            plt.scatter(mat[1,:][i],mat[2,:][i], c=[0,1,0]) #Has green labels
        elif cl[i] == 2: #Third class
            plt.scatter(mat[1,:][i],mat[2,:][i], c=[0,0,1]) #Has blue labels
        else: #Any other class
            plt.scatter(mat[1,:][i],mat[2,:][i], c=[1,1,1]) #Has white labels
    plt.ylabel('Dimension 3')
    plt.xlabel("Dimension 2")
    plt.show()
    
#Runs everything to summarise the given document
def textSummarization(filename):
    f = open(filename, 'r')
    doc = f.read()
    unwantedWords = ['a', 'about', 'all', 'an', 'and', 'are', 'as', 'at', 'be', 'because', 'by', 'for',
                 'from', 'had', 'have', 'i', 'in', 'is', 'it', 'not', 'of',
                 'on', 'or', 'said', 'that', 'that', 'the', 'then', 'there', 'they', 'though', 'this',
                 'to', 'too', 'was', 'we', 'were', 'when', 'which', 'will', 'with', 'you', 'your', '']
    words = []
    sentences = []
    countMatrix = []
    
    #PARSE TEXT
    parseText(doc, words, sentences, unwantedWords)

    #BUILD COUNT MATRIX
    count = buildCountMatrix(words, sentences, countMatrix)

    #PERFORM SVD
    U, S, V = doSVD(count)

    #CLUSTER
    toPlot = V[1:3,:].transpose() #I plot V because that describes the relationships between the sentences in the documents - so it groups the similar sentences when clustered
    classes = getClusteredClasses(toPlot, 3)

    #PLOT GRAPHS
    plotClusters(V, classes)


def runExamples():
    #This has just 12 sentences, 3 groups of 4 very similar and simple sentences.
    #By doing this example it shows that the different sentences are in almost opposite places on the plotted graph - i.e. they are very different sentences
    #The sentences are clearly sorted into distinct classes
    textSummarization("test2.txt")

    #This is the first chapter of the book "lord of the Flies". This has clearly 3 distinct clusters, although it could have up to about 6.
    #I chose 3 clusters arbitratily
    #This one has got the best results, most likely because there is the most amount of text so can form the most obvious clusters
    textSummarization("LOTF.txt")

    #This includes extracts from 3 different books - "lord of the Flies", "1984", and the Bible.
    #I clustered it into 3 clusters to find the 3 distinct texts.
    textSummarization("books.txt")


#In this assignemnt I have chosen to look at the relationships between sentences in a document, and to try to classify similar sentences.
runExamples()


