
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <math.h>
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
//this is our EDGE object definition
//for any given edge, we need its begin and end points,
//sheet#, as well as the set of edges that intersect it
//this list consists of pointers to the edges of intersection
struct EDGE
{
	int originPoint;
	int endPoint;
    int sheetNumber;
    list<EDGE*>intersectingEdges;
};
//SHEET object definition
//each sheet has a list of edges it contains
//and then the number of the edges in that sheet
struct SHEET
{
    list<EDGE*>edgesInSheet;
    int numEdgesInSheet;
};

//BOOK object definition
//each book has the list of its sheets
struct BOOK
{
    list<SHEET*>sheetsInBook;
    list<SHEET>sheetsInBookStorage;
    int sheetNumber;
};
int numVertices, numInteriorEdges, numSheets, maxEdgesPerSheet;
stringstream fileOutputStream;

int validEmbedCount = 0;
int masterSheetCount = 0;
int outputCount = 0;
int totalEmbeddings = 0;
int totalLevels;
//stores sizes for given distributions.. K_6 1233 would be 9,21,14,14 9 sheets w/1 edge, 21 sheets 2 edges, etc.
int sizes [35];
//holds sizes for given edge distributions.. K_6 has 9 edges with 1 edge, for ex
int distribution[7];
int numValidPartitions = 0;
int validPartitions[1000];
int onePartitionArray[35];

EDGE allEDGEs[45];
EDGE interiorEDGEsForOneGraph[35];
SHEET allMySheets[20000];
SHEET sheetsForIteration[2000];
SHEET outputList [54];
SHEET masterSheetList [10000];
BOOK validEmbeddings [5000];
int numSheetsForIteration = 0;



void integerPartition(int n, int * a, int level);
void print(int n, int * a);
void findPartitionsOfSheetNumLength(int n, int * a, int sheetNum);
void iteration (int currentLevel);
int mapLevelToIndex (int level, int offset);
void dumpOutputList (int level, int offset);





list<BOOK*>allEmbeddings;
list<BOOK*>lessEmbeddings;

int getReadStartPoint(int partitionValue);
void addSheetsInRangeToArray(int partitionValue);

void addSheetsInRangeToArray(SHEET iteratingSheets[], int start, int end, int partitionValue);
void buildSizes();
void buildSheets(list<EDGE*> validSheets[], int numValidSheets);
unsigned int count(unsigned int i);
BOOK createBookFromSheets(SHEET someSheets[], int numSheets);
void crunchEmbeddingCombinations(SHEET mySheets[], int numValidSheets);
//bool determineIfValidEmbedding(list<EDGE*> bookRepresentation[], int numSheets);
bool determineIfValidEmbedding(SHEET bookRepresentation[], int numSheets);
bool determineIfTwoEdgesEquivalent(EDGE a, EDGE b);
bool determineIfTwoSheetsAreEqual(SHEET a, SHEET b);
bool determineIfTwoSheetsCooperate(list<EDGE*> sheetOne, list<EDGE*> sheetTwo);
void generateSheets(int numVertices, int sheetNum);
void getIntArrayForMultiDigitInt(int);
int determineMaxEdgesPerSheet();
void discardInvalidSheets(list<EDGE*> allSheets[], int sheetCount);
int calculateNumberOfEdges(int numVertices);
int calculateNumberOfInteriorEdges(int numVertices);
bool determineIfTwoEdgesIntersect(EDGE a, EDGE b);
int generateEdges(int numVertices, EDGE allEDGEs[]);
int generateInteriorEdges(int numVertices, EDGE allEDGEs[], EDGE interiorEDGEs[]);
void getUserInput();
void prepareSheetsArrayForRecursiveIteration();
bool checkForIntersectionsBetweenEdgeAndSet(EDGE oneEdge, list<EDGE*> setOfEdges);
std::ostream& operator << (std::ostream &o, SHEET &a);
std::ostream& operator << (std::ostream &o, BOOK &a);
std::ostream& operator << (std::ostream &o, EDGE &a);
std::istream& operator>> (std::istream &in, EDGE &a);

int main (int argc, char *args[])
{
    
    getUserInput();
    maxEdgesPerSheet = determineMaxEdgesPerSheet();

    //prepare edges..
    generateEdges(numVertices, allEDGEs);
    generateInteriorEdges(numVertices, allEDGEs, interiorEDGEsForOneGraph);
    //array used in partition creation
    int * a = (int * ) malloc(sizeof(int) * numInteriorEdges);
    integerPartition (numInteriorEdges, a, 0);
    totalLevels = numSheets;
    
    cout << endl << "Below are the valid partitions for K" << numVertices << " on " << numSheets << " sheets: " << endl;
    for(int i = 0; i < numValidPartitions; i++)
    {
        printf("%d. %d \n", i+1 , validPartitions[i]);
    }
    cout << "They all add up to " << numInteriorEdges << ", and have "<< numSheets << " parts." << endl;
    cout << "Additionally, all integers in partition are <= " << maxEdgesPerSheet << ", the max edges per sheet." << endl;

    generateSheets(numVertices, numSheets);
    
    prepareSheetsArrayForRecursiveIteration();
   
    return 0;
} // main
//==============================================
//this method outputs the current level of combinations
void dumpOutputList (int level, int offset)
{
    int index;
    SHEET oneEmbedding[35];
    totalEmbeddings++;
    for (index = 0; index < outputCount; ++index)
    {
        oneEmbedding[index] = outputList[index];
        
    }
    if(determineIfValidEmbedding(oneEmbedding, numSheets))
    {
        
        for (index = 0; index < outputCount; ++index)
        {
            masterSheetList[masterSheetCount + index] = outputList[index];
            validEmbeddings[validEmbedCount].sheetsInBook.push_back(&masterSheetList[masterSheetCount + index]);
            fileOutputStream << endl << outputList [index] << "===";
            masterSheetCount++;
        }

        
        fileOutputStream << endl << endl;
        
        validEmbedCount++;
        cout << endl << "valid embeddings: " << validEmbedCount;
        
    }
    
    
}

//==============================================
//returns the index of the current level
int mapLevelToIndex (int level, int offset)
{
    int sum = 0;
    int index;
    
    for (index = 0; index < level; ++index)
        sum += sizes [index];
    
    return (sum+offset);
}

//==============================================
//recursively iterates the sheetsForIteration[] structure
//it combines all the sheets of different sizes with all the
//other sheets to create our embeddings. must be called after
//prepareSheetsArrayForRecursiveIteration()
void iteration (int currentLevel)
{
    int index;
    for (index = 0; index < sizes [currentLevel]; ++index) {
        
        outputList [outputCount++] = sheetsForIteration [mapLevelToIndex(currentLevel, index)];
        if (currentLevel < (totalLevels - 1))
            iteration (currentLevel+1);
        else
            dumpOutputList (currentLevel, index);
        
        --outputCount;
        
    } // for each one
    
}

/*
 ********************************************************************
 Author:    Monsi Terdex
 Below code creates partitions for a given value n. Thanks, Mr. Terdex,
 ********************************************************************
 
 =====================
 Routine for printing out array contents
 =====================
 */
void print(int n, int * a) {
    int i, numValues = 0;
    std::string oneValue;
    bool validPartition = true;
    for (i = 0; i <= n; i++) {
        if(a[i] > maxEdgesPerSheet)
        {
            //break;
            validPartition = false;
        }
        //printf("%d=", a[i]);
        oneValue += to_string(static_cast<long long>(a[i]));
        numValues++;
        
    }
    if(numValues == numSheets && validPartition == true)
    {/*
        if(validPartition)
            cout << "validPartition is true.." << endl;
        else
            cout << "validPartition is false" << endl;*/
        validPartitions[numValidPartitions] = std::stoi(oneValue);
        numValidPartitions++;
    }
    printf("\n");
}
/*
 =====================
 The algorithm
 =====================
 */
void integerPartition(int n, int * a, int level){
    int first;
    int i;
    if (n < 1) return ;
    a[level] = n;
    print(level, a);
   
    first = (level == 0) ? 1 : a[level-1];
    for(i = first; i <= n / 2; i++){
        a[level] = i;
        integerPartition(n - i, a, level + 1);
    }
}

//4 methods below can be used to generate permutations
//could be used to get all permutations of our partitions
//currently not implemented
/*
void swap(int *v, const int i, const int j)
{
    int t;
    t = v[i];
    v[i] = v[j];
    v[j] = t;
}

void printPerm(const int *v, const int size)
{
    if (v != 0) {
        for (int i = 0; i < size; i++) {
            printf("%4d", v[i] );
        }
        printf("\n");
    }
}

void rotateLeft(int *v, const int start, const int n)
{
    int tmp = v[start];
    for (int i = start; i < n-1; i++) {
        v[i] = v[i+1];
    }
    v[n-1] = tmp;
} // rotateLeft


void permute(int *v, const int start, const int n)
{
    printPerm(v, n);
    if (start < n) {
        int i, j;
        for (i = n-2; i >= start; i--) {
            for (j = i + 1; j < n; j++) {
                swap(v, i, j);
                permute(v, i+1, n);
            } // for j
            rotateLeft(v, i, n);
        } // for i
    }
} // permute*/

//===============================================
//Methods for embedding generation and validation
//===============================================

void generateSheets(int numVertices, int sheetNum)
{
    //int numInteriorEdges;//, sheetNumMin, sheetNumMax;
    
    //numInteriorEdges = calculateNumberOfInteriorEdges(numVertices);
    //ofstream RTKFileOutput1("K" + numVertices + "_" + count);//file output stream
    
    
    //this loop builds the set of intersecting edges for each interior edge
    for(int i = 0; i < numInteriorEdges; i++)
    {
        
        //every time we look at a different interior edge, compare it to all the other
        //interior edges and see if they intersect. if so, add intersecting edge to list
        for(int j = 0; j < numInteriorEdges; j++)
        {
            
            //we don't want to compare the same edge
            if(i != j)
            {
                //do intersection test, if it passes..
                if(determineIfTwoEdgesIntersect(interiorEDGEsForOneGraph[i], interiorEDGEsForOneGraph[j]))
                    // || (determineIfTwoEdgesIntersect(interiorEDGEs[j], interiorEDGEs[i])))
                {
                    //put it in the list
                    interiorEDGEsForOneGraph[i].intersectingEdges.push_back(&interiorEDGEsForOneGraph[j]);
                }
                //else
                //{
                 //   interiorEDGEsForOneGraph[i].cooperatingEdges.push_back(&interiorEDGEsForOneGraph[j]);
                //}
            }
        }
    }
    //determine upper bound of max edges per sheet, can be set manually for more accuracy
    //maxEdgesPerSheet = determineMaxEdgesPerSheet();//3;
    
    std::list<EDGE*>oneSheet;
    std::list<EDGE*> allSheets[10000];
    int sheetCount = 0;
    /* this loop enumerates from 1 to the maximal number of edges in a sheet, and it
     * generates all possible sheets containing that number of edges using the set
     * of interior edges. intersections are not considered, so thousands of invalid
     * sheets are created.
     */
    
    for(int edgesInCurrentSheet = 1; edgesInCurrentSheet <= maxEdgesPerSheet; edgesInCurrentSheet++)
    {
        switch(edgesInCurrentSheet)
        {
                //for instance, this builds all sheets with 1 edge
            case 1:
                for(int currentEdge = 0; currentEdge < numInteriorEdges; currentEdge++)
                {
                    oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdge]);
                    allSheets[sheetCount] = oneSheet;
                    oneSheet.clear();
                    sheetCount++;
                }
                break;
                //builds all sheets with 2 edges, etc..
            case 2:
                for(int currentEdgeA = 0; currentEdgeA < numInteriorEdges; currentEdgeA++)
                {
                    for(int currentEdgeB = currentEdgeA+1; currentEdgeB < numInteriorEdges; currentEdgeB++)
                    {
                        oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeA]);
                        oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeB]);
                        allSheets[sheetCount] = oneSheet;
                        oneSheet.clear();
                        sheetCount++;
                    }
                }
                break;
                
            case 3:
                for(int currentEdgeA = 0; currentEdgeA < numInteriorEdges; currentEdgeA++)
                {
                    for(int currentEdgeB = currentEdgeA+1; currentEdgeB < numInteriorEdges; currentEdgeB++)
                    {
                        for(int currentEdgeC = currentEdgeB + 1; currentEdgeC < numInteriorEdges; currentEdgeC++)
                        {
                            oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeA]);
                            oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeB]);
                            oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeC]);
                            allSheets[sheetCount] = oneSheet;
                            oneSheet.clear();
                            sheetCount++;
                        }
                    }
                }
                break;
            case 4:
                for(int currentEdgeA = 0; currentEdgeA < numInteriorEdges; currentEdgeA++)
                {
                    for(int currentEdgeB = currentEdgeA+1; currentEdgeB < numInteriorEdges; currentEdgeB++)
                    {
                        for(int currentEdgeC = currentEdgeB + 1; currentEdgeC < numInteriorEdges; currentEdgeC++)
                        {
                            for(int currentEdgeD = currentEdgeC + 1; currentEdgeD < numInteriorEdges; currentEdgeD++)
                            {
                                oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeA]);
                                oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeB]);
                                oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeC]);
                                oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeD]);
                                allSheets[sheetCount] = oneSheet;
                                oneSheet.clear();
                                sheetCount++;
                            }
                        }
                    }
                }
                break;
            case 5:
                for(int currentEdgeA = 0; currentEdgeA < numInteriorEdges; currentEdgeA++)
                {
                    for(int currentEdgeB = currentEdgeA+1; currentEdgeB < numInteriorEdges; currentEdgeB++)
                    {
                        for(int currentEdgeC = currentEdgeB + 1; currentEdgeC < numInteriorEdges; currentEdgeC++)
                        {
                            for(int currentEdgeD = currentEdgeC + 1; currentEdgeD < numInteriorEdges; currentEdgeD++)
                            {
                                for(int currentEdgeE = currentEdgeD + 1; currentEdgeE < numInteriorEdges; currentEdgeE++)
                                {
                                    oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeA]);
                                    oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeB]);
                                    oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeC]);
                                    oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeD]);
                                    oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeE]);
                                    allSheets[sheetCount] = oneSheet;
                                    oneSheet.clear();
                                    sheetCount++;
                                }
                            }
                        }
                    }
                }
                break;
            case 6:
                for(int currentEdgeA = 0; currentEdgeA < numInteriorEdges; currentEdgeA++)
                {
                    for(int currentEdgeB = currentEdgeA+1; currentEdgeB < numInteriorEdges; currentEdgeB++)
                    {
                        for(int currentEdgeC = currentEdgeB + 1; currentEdgeC < numInteriorEdges; currentEdgeC++)
                        {
                            for(int currentEdgeD = currentEdgeC + 1; currentEdgeD < numInteriorEdges; currentEdgeD++)
                            {
                                for(int currentEdgeE = currentEdgeD + 1; currentEdgeE < numInteriorEdges; currentEdgeE++)
                                {
                                    for(int currentEdgeF = currentEdgeE + 1; currentEdgeF < numInteriorEdges; currentEdgeF++)
                                    {
                                        oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeA]);
                                        oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeB]);
                                        oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeC]);
                                        oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeD]);
                                        oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeE]);
                                        oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeF]);
                                        allSheets[sheetCount] = oneSheet;
                                        oneSheet.clear();
                                        sheetCount++;
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            case 7:
                for(int currentEdgeA = 0; currentEdgeA < numInteriorEdges; currentEdgeA++)
                {
                    for(int currentEdgeB = currentEdgeA+1; currentEdgeB < numInteriorEdges; currentEdgeB++)
                    {
                        for(int currentEdgeC = currentEdgeB + 1; currentEdgeC < numInteriorEdges; currentEdgeC++)
                        {
                            for(int currentEdgeD = currentEdgeC + 1; currentEdgeD < numInteriorEdges; currentEdgeD++)
                            {
                                for(int currentEdgeE = currentEdgeD + 1; currentEdgeE < numInteriorEdges; currentEdgeE++)
                                {
                                    for(int currentEdgeF = currentEdgeE + 1; currentEdgeF < numInteriorEdges; currentEdgeF++)
                                    {
                                        for(int currentEdgeG = currentEdgeF + 1; currentEdgeG < numInteriorEdges; currentEdgeG++)
                                        {
                                            oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeA]);
                                            oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeB]);
                                            oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeC]);
                                            oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeD]);
                                            oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeE]);
                                            oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeF]);
                                            oneSheet.push_back(&interiorEDGEsForOneGraph[currentEdgeG]);
                                            allSheets[sheetCount] = oneSheet;
                                            oneSheet.clear();
                                            sheetCount++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                break;
                
            default:
                break;
        }
        
    }
    
    
    
    cout << "number of sheets: " << sheetCount << endl;
    discardInvalidSheets(allSheets, sheetCount);
}

//returns an upper bound for how many edges could be in a sheet
int determineMaxEdgesPerSheet()
{
 //use what we know about minimal embeddings, or Canonical BR, which have
        //most edges per sheet possible
    return numVertices - 2 - 1;

}

//this method iterates over our list of sheets and gets rid of all sheets that
//contain intersections
void discardInvalidSheets(list<EDGE*> allSheets[], int sheetCount) 
{
    bool discard = false;
    list<EDGE*> validSheets[1000];
    int numValidSheets = 0;
    for(int currentSheet = 0; currentSheet < sheetCount; currentSheet++)
    {
        //if it has 1 one edge just put it in storage because it must be valid
        //as it cannot have intersections
        if(allSheets[currentSheet].size() == 1)
        {
            validSheets[numValidSheets] = allSheets[currentSheet];
            numValidSheets++;
        }
        else
        {
            //if there's more than 1 edge then we need to check all edges against
            //each other to ensure no intersections are present
            for(list<EDGE*>::iterator iter = allSheets[currentSheet].begin(),
                end = allSheets[currentSheet].end();
                iter != end && discard == false;
                ++iter)
            {
                //if an intersection occurs discard is set to true
                discard = checkForIntersectionsBetweenEdgeAndSet(**iter, allSheets[currentSheet]);
            }
            if(discard == false)
            {
                //no intersections have occurred, so add it to our validSheets storage
                validSheets[numValidSheets] = allSheets[currentSheet];
                numValidSheets++;
            }
        }
        //reset discard boolean
        discard = false;
    }
    
    cout << "numValidSheets: " << numValidSheets << endl;
    
    buildSheets(validSheets, numValidSheets);
}

//Gives the number of edges present in a given Kn
int calculateNumberOfEdges(int numVertices)
{
	int n = numVertices;
    
	return (n*(n-1))/2;
}
//Gives the number of interior edges present in a given Kn
int calculateNumberOfInteriorEdges(int numVertices) 
{
	int n = numVertices;
    
	return ((n*(n-1))/2) - n;
}
//returns true if edges intersect
//TODO:add case so we don't have to or it.. also test b to a
bool determineIfTwoEdgesIntersect(EDGE a, EDGE b)
{
    
	if((a.originPoint < b.originPoint) && (b.originPoint < a.endPoint) && (a.endPoint < b.endPoint))
	{
        
		return true;
	}
	else if((b.originPoint < a.originPoint) && (a.originPoint < b.endPoint) && (b.endPoint < a.endPoint))
	{
		return true;
	}
	else
    {
        
		return false;
    }
    
}


//fills allEDGEs[] array with values, passed by reference so value isn't returned
//returns count of edges added for verification
int generateEdges(int numVertices, EDGE allEDGEs[]) 
{
    int edgeCount = 0;
    //origin range is from 1 to numVertices (for k5, 1-5)
    for(int i = 1; i < numVertices; i++)
    {
        //so, for k5 we'll add 4 edges with origin 1..{1:2,1:3,1:4,1:5}
        for(int j = i+1; j <= numVertices; j++)
        {
            allEDGEs[edgeCount].originPoint = i;
            allEDGEs[edgeCount].endPoint = j;
            edgeCount++;
        }
    }
    return edgeCount;
}

//fills interiorEDGEs[] array with values, passed by reference so value isn't returned
//works in conjunction with generateEDGEs, which builds allEDGEs[] first
//it returns the numInteriorEdges count so we can validate that all were defined
//keep in mind that allEDGEs edges are always consecutive.. 1:3 exists but not 3:1
int generateInteriorEdges(int numVertices, EDGE allEDGEs[], EDGE interiorEDGEs[]) 
{
    int interiorEdgeCount = 0;
    //do this numInteriorEdges number of times
    for(int index = 0; index < calculateNumberOfEdges(numVertices); index++)
    {
        //check to make sure given edge is an interior edge
        if((allEDGEs[index].endPoint - allEDGEs[index].originPoint > 1) &&
           (allEDGEs[index].endPoint - allEDGEs[index].originPoint != numVertices-1))
        {
            //if so, add it to set of interiorEdges
            interiorEDGEs[interiorEdgeCount] = allEDGEs[index];
            interiorEdgeCount++;
        }
    }
    
    return interiorEdgeCount;
}

//determines if there exist any intersections between a given edge and each
//edge in a given set
bool checkForIntersectionsBetweenEdgeAndSet(EDGE oneEdge, list<EDGE*> setOfEdges)
{
    for(list<EDGE*>::iterator iter = setOfEdges.begin(),
        end = setOfEdges.end();
        iter != end;
        ++iter)
    {
        //intersection found
        if(determineIfTwoEdgesIntersect(oneEdge, **iter))
            return true;
    }
    return false;
    
}

//Output operator for SHEET struct.. allows cout << mySheet
//to be used.
std::ostream& operator << (std::ostream &o, SHEET &a)
{
    //o << a.edgesInSheet << "-" << a.endPoint;
    for(list<EDGE*>::iterator iter = a.edgesInSheet.begin(),
        end = a.edgesInSheet.end();
        iter != end;
        ++iter)
    {
        o << **iter << endl;
    }
    return o;
}

//Output operator for BOOK struct.. allows cout << myBook
//to be used.
std::ostream& operator << (std::ostream &o, BOOK &a) 
{
    //o << a.edgesInSheet << "-" << a.endPoint;
    for(list<SHEET*>::iterator iter = a.sheetsInBook.begin(),
        end = a.sheetsInBook.end();
        iter != end;
        ++iter)
    {
        o << **iter << endl;
    }
    o << "===" << endl;
    return o;
}

//overloading output operator so we can use << operator to output edges
std::ostream& operator << (std::ostream &o, EDGE &a) 
{
    o << a.originPoint << "-" << a.endPoint;
    return o;
}
//overloaded input operator for edge.. not in use currently
//TODO: implement this for sheets and books and then input
//generated embeddings and evaluate them in more ways
std::istream& operator>> (istream &in, EDGE &a)
{
    char garbage;
    in >> a.originPoint;
    in >>  garbage;
    in >> a.endPoint;
    
    return in;
}

//Adds information to our array of valid sheets, such as number of edges
//Then, outputs all of them to a text file. Useful to ensure that the list
//of sheets you're working with is valid.
void buildSheets(list<EDGE*> validSheets[], int numValidSheets)
{
    //SHEET mySheets[1000];
    int prevNumEdgesInSheet = 1;
    for(int i = 0; i < numValidSheets; i++)
    {
        allMySheets[i].edgesInSheet = validSheets[i];
        allMySheets[i].numEdgesInSheet = (int)validSheets[i].size();
        if(allMySheets[i].numEdgesInSheet == prevNumEdgesInSheet)
        {
            distribution[prevNumEdgesInSheet-1]++;
        }
        else
        {
            distribution[allMySheets[i].numEdgesInSheet-1]++;
            prevNumEdgesInSheet++;
        }
    }
    ofstream OneSheetOutput ("sheetList.txt");
    for(int i = 0; i < numValidSheets; i++)
    {
        OneSheetOutput << allMySheets[i] << endl << endl;
    }
    OneSheetOutput.close();
}

//makes sure that no two given sheets in an embedding have the same edges
bool determineIfValidEmbedding(SHEET bookRepresentation[], int numSheets)
{
    SHEET oneSheet;
    for(int sheetA = 0; sheetA < numSheets-1; sheetA++)
    {
        for(int sheetB = sheetA+1; sheetB < numSheets; sheetB++)
        {
            //when we find two sheets that have same edges then we know the embedding is invalid
            if(!determineIfTwoSheetsCooperate(bookRepresentation[sheetA].edgesInSheet, bookRepresentation[sheetB].edgesInSheet))
                return false;
        }
        
    }
    //base case, if no other embeddings to compare don't bother
    if(validEmbeddings == 0)
        return true;
    
    int sheetMatches = 0;
    //iterate over entire list of all valid embeddings
    for(int embedding = 0; embedding < validEmbedCount; embedding++)
    {
        //looking at every sheet in each book embedding in storage
        for(list<SHEET*>::iterator iterA = validEmbeddings[embedding].sheetsInBook.begin(),
            end = validEmbeddings[embedding].sheetsInBook.end();
            iterA != end;
            ++iterA)
        {
            oneSheet = **iterA;
            //look at each sheet in the book we want to add to storage
            for(int sheetA = 0; sheetA < numSheets; sheetA++)
            {
                
                if(determineIfTwoSheetsAreEqual(bookRepresentation[sheetA], oneSheet))
                    ++sheetMatches;
                
                
            }
            
        }
        //cout << sheetMatches << " matches." << endl;
        if(sheetMatches == numSheets)
            return false;
        sheetMatches = 0;
    }
    
    
    return true;
    
}

bool determineIfTwoSheetsAreEqual(SHEET a, SHEET b)
{
    //cout << a.edgesInSheet.size() << " and " << b.edgesInSheet.size() << endl;
    if(a.edgesInSheet.size() != b.edgesInSheet.size())
    {
        //cout << "SHEETS INEQUAL SIZE!" << endl;
        return false;
    }
    
    int matches = 0;
    
    
    //we have to look at the edges in the sheet to know if the sheets are comparable
    //so, if 2 sheets have the same number of edges, and contain the same edges,
    //they are equal..
    for(list<EDGE*>::iterator iterA = a.edgesInSheet.begin(),
        end = a.edgesInSheet.end();
        iterA != end;
        ++iterA)
    {
        for(list<EDGE*>::iterator iterB = b.edgesInSheet.begin(),
            end = b.edgesInSheet.end();
            iterB != end;
            ++iterB)
        {
            //cout << **iterA << "," << **iterB << endl;
            if(determineIfTwoEdgesEquivalent(**iterA, **iterB))
            {
                matches++;
            }
        }
    }
    if(matches == (int)a.edgesInSheet.size())
    {
        return true;
    }
    else
    {
        return false;
    }
}

//Evaluates two edges and determines if they are the same edge
bool determineIfTwoEdgesEquivalent(EDGE a, EDGE b)
{
    if((a.originPoint == b.originPoint) && (a.endPoint == b.endPoint))
        return true;
    else if ((a.originPoint == b.endPoint) && (a.endPoint == b.originPoint))
        return true;
    else
        return false;
}

//Looks at the edges in two given sheets and checks if any two are the same edge
//if they are the same then we know these two sheets can't go together
bool determineIfTwoSheetsCooperate(list<EDGE*> sheetOne, list<EDGE*> sheetTwo)
{
    int firstSize, secondSize;
    firstSize = (int)sheetOne.size();
    secondSize = (int)sheetTwo.size();

    for(list<EDGE*>::iterator iterA = sheetOne.begin(),
        end = sheetOne.end();
        iterA != end;
        ++iterA)
    {
        for(list<EDGE*>::iterator iterB = sheetTwo.begin(),
            end = sheetTwo.end();
            iterB != end;
            ++iterB)
        {
            if(*iterA == *iterB)
                return false;
        }
    }
    return true;
}

//creates a book representation from the array of sheets passed
BOOK createBookFromSheets(SHEET someSheets[], int numSheets)
{

    BOOK oneBookEmbedding;
    for(int index = 0; index < numSheets; index++)
    {
        oneBookEmbedding.sheetsInBook.push_back(&someSheets[index]);
    }
    return oneBookEmbedding;
}

//lets the user enter numVertices and numSheets for given embedding
void getUserInput()
{
    cout << "How many vertices?" << endl;
    cin >> numVertices;
    
    //error catching loop for numvertices, forces user to enter numV value 4-10
    while(numVertices < 5 || numVertices > 10)
    {
        cout << "Invalid number of vertices, valid range is 5-10 inclusive." << endl;
        cout << "Try again. How many vertices?" << endl;
        cin >> numVertices;
    }
    
    
    numInteriorEdges = calculateNumberOfInteriorEdges(numVertices);
    
    //TODO: add method to use what we know about min/max sheet num
    //      and modify the ranges accordingly
    cout << "How many sheets?" << endl;
    cin >> numSheets;
    
    //when an invalid sheet num is entered, keep prompting until a valid one is entered
    while((numSheets > calculateNumberOfInteriorEdges(numVertices)) || (numSheets < 1))
    {
        cout << "Invalid sheet number: " << calculateNumberOfInteriorEdges(numVertices) << " is maximum." << endl;
        cout << "How many sheets?" << endl;
        cin >> numSheets;
    }

}
//this turns our stored integer partition into an integer array so we
//can access each element individually
void getIntArrayForMultiDigitInt(int onePartition)
{
    
    int sheetNum = numSheets;
    while (sheetNum--)
    {
        onePartitionArray[sheetNum] = onePartition % 10;
        onePartition /= 10;
        cout << endl << onePartitionArray[sheetNum];
        
    }
    cout << endl << "Enter any key to continue and begin data generation:";
    char k; cin >> k;
}
//this method creates an array of sheets in the order we need to iterate
//it recursively. it repeats ranges of sheets as necessary. for instance,
//for a 11133 embedding for K6 the 9 1 edge sheets to appear 3 times, so
//they will be the first 27 sheets in the array
void prepareSheetsArrayForRecursiveIteration()
{
    string fileName;
    ofstream fileOutput;

    
    cout << "We can only generate one set of embeddings for each session. Select the partition you'd like to generate embeddings for by entering its corresponding number:" << endl;
    cout << "A .txt file will be created that indicates the number of vertices, sheet number, and partition, e.g. 1233 if working with K_6 on 4 sheets." << endl;
    cout << "Please enter an integer in the range [1, " << numValidPartitions << "]:" << endl;
    int selection;
    //make sure integer is in correct range.. entering a char
    //that is non-int will make it go bananas more than likely
    while (true)
    {
        cin >> selection;
        
        if ((cin) && (selection >= 1) && (selection <= numValidPartitions))
            break;
        
        cin.clear();
        
        cout << "Try again: " << endl;
    }
    cout << "Great!\nGenerating embeddings in " << validPartitions[selection-1] << ".\n";
    int index = selection - 1;
    //for(int index = 0; index < numValidPartitions; index++)
    //{
        getIntArrayForMultiDigitInt(validPartitions[index]);
        for(int partitionIndex = 0; partitionIndex < numSheets; partitionIndex++)
        {
            sizes[partitionIndex] = distribution[onePartitionArray[partitionIndex]-1];
            cout << endl << sizes[partitionIndex];
            addSheetsInRangeToArray(onePartitionArray[partitionIndex]);
        }
        
        //this iterates the sheet data structure each time to create a new
        //embedding set of a given configuration
        iteration(0);
      
        //handle file output for given embedding config
        fileName = "K_" + to_string(static_cast<long long>(numVertices)) + "_#" + to_string(static_cast<long long>(numSheets)) + "_" + to_string(static_cast<long long>(validPartitions[index])) + ".txt";
        //(thanks for not implementing all of the standard c++ to_string overloads, MSVS!)
		fileOutput.open(fileName.c_str());
        fileOutput << "K_" << numVertices << " on " << numSheets << " with a " << validPartitions[index]
                   << " config. There are " << validEmbedCount << " embeddings.";
        fileOutput << fileOutputStream.str();
        fileOutput.close();
    
    //if we were going to run multiple iterations we'd have to do all this zeroing out
    //but because of memory issues and other whackiness we'll run 1 at a time..
        /*
        fileOutputStream.clear();
        validEmbedCount = 0;
        numSheetsForIteration = 0;
        masterSheetCount = 0;
    */
        cout << endl << "Done! Enter any key to quit.." << endl;
        char k; cin >> k;

        
    //}
}

//this method sets up an array with sheets in the order necessary
//so that they can be traversed by our recursive combination algorithm
void addSheetsInRangeToArray(int partitionValue)
{
    int readStart = getReadStartPoint(partitionValue);
    
    for(int i = 0; i < distribution[partitionValue-1]; i++)
    {
        sheetsForIteration[numSheetsForIteration] = allMySheets[readStart+i];
        //cout <<  endl << sheetsForIteration[numSheetsForIteration] << "===";
        numSheetsForIteration++;
    }
    //cout << endl << "==========" << endl;
    
}

//returns the index of where the given sheets we want start
int getReadStartPoint(int partitionValue)
{
    int startPoint = 0;
    //we know that sheets with 1 edge are always first
    if(partitionValue == 1)
    {
        return 0;
    }
    //if we are looking for sheets with > 1 edge,
    //they start at the point where all the sheets
    //with less edges end
    else
    {
        for(int i = 1; i < partitionValue; i++)
        {
            startPoint += distribution[i-1];
        }
        return startPoint;
    }
}




