

/*
 *This program generates an RTK file in the directory where you store
 *the .exe file. It will be named RTKFile.txt. To generate an RTKFile
 *for a given embedding, enter the n value for Kn, the num sheets,
 *and assign a sheet number for all interior edges as prompted. Edges
 *can't be assigned to non-existent sheets or sheets where a conflicting
 *edge exists.
 *Additionally, this code can be used to generate book representations
 *for complete graphs of a given sheet number.
 *
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <list>
#include <string>
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
    std::list<EDGE*>intersectingEdges;
    std::list<EDGE*>cooperatingEdges;
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
    int sheetNumber;
};

//evaluates a list of generated sheets and disposes of invalid ones
void discardInvalidSheets(list<EDGE*> allSheets[], int sheetCount);

//determines if the given edge intersects with any edges in given set
bool checkForIntersectionsBetweenEdgeAndSet(EDGE oneEdge, list<EDGE*> setOfEdges);

//gets an upper bound on the number of edges in a sheet
int determineMaxEdgesPerSheet(EDGE interiorEdges[], int numInteriorEdges, int sheetNum);

//combines all valid sheets in all possible ways for given sheet number
void crunchEmbeddingCombinations(SHEET mySheets[], int numValidSheets);

std::ostream& operator << (std::ostream &o, SHEET &a);
void buildSheets(list<EDGE*> validSheets[], int numValidSheets);
bool determineIfTwoSheetsCooperate(list<EDGE*> sheetOne, list<EDGE*> sheetTwo);

void generateEmbeddingsForGivenSheetNumAndNumVertices(int numVertices, int sheetNum);
//inputs ordered template for kn so we can output rtk data in same order
void getFileInput(int numVertices, int numEdges, int numInteriorEdges, EDGE allEDGEs[], EDGE interiorEDGEs[]);

//builds set of edges for complete graph, returns how many generated
int generateEdges(int numVertices, EDGE allEDGEs[]);

//build set if interior edges for given complete graph, returns how many generated
int generateInteriorEdges(int numVertices, EDGE allEDGEs[], EDGE interiorEDGEs[]);

//build set of edges needed for RTKFile, so all edges including repetition
int generateAllRTKEdges(int numVertices, EDGE allRTKEDGEs[]);

//returns true if edges passed are same edge.. true if 1:2 and 2:1 passed, for ex.
bool determineIfTwoEdgesEquivalent(EDGE a, EDGE b);

//creates RTKFile for interior edges
//bool generateRTKFileOutput(int numVertices, int numInteriorEdges, EDGE allEDGEs[], EDGE interiorEDGEs[]);

//creates RTKFile for all edges in complete graph, including repetition
//bool generateRTKFileOutputAllEdges(int numVertices, int numInteriorEdges, EDGE allRTKEDGEs[], EDGE interiorEDGEs[]);

//returns number of interior edges in complete graph
int calculateNumberOfInteriorEdges(int numVertices);

//returns number of edges in complete graph
int calculateNumberOfEdges(int numVertices);

//returns true if edges passed intersect
bool determineIfTwoEdgesIntersect(EDGE a, EDGE b);

//overloaded output operator so we can use cout << to output any edge
std::ostream& operator << (std::ostream &o, EDGE &a);
std::ostream& operator << (std::ostream &o, BOOK &a);

/*
void swap(int *v, const int i, const int j);
void print(const int *v, const int size);
void rotateLeft(int *v, const int start, const int n);
void permute(int *v, const int start, const int n);*/

bool areEmbeddingsEqual(BOOK embedA, BOOK embedB, int sheetNum);

int numVertices, numSheets;

SHEET allMySheets[1000];
BOOK myEmbeddings[11111];
BOOK reducedEmbeddings[1500];

list<BOOK*>allEmbeddings;
list<BOOK*>lessEmbeddings;

int main(int argc, const char * argv[])
{
   // int numVertices;
    int currentSheet;
    //int numSheets;
    
	cout << "This program will generate an rtk .txt file based on" << endl;
	cout << "a book representation of a complete graph on kn vertices." << endl;
	cout << "for each interior edge you will be prompted to enter a sheet" << endl;
	cout << "number. After all edges are assigned the program terminates." << endl;
	cout << "The file will be in the .exe home dir, under the name RTKFile.txt" << endl;
    cout << " *Additionally, this code can be used to generate book representations" << endl;
    cout << "for complete graphs of a given sheet number." << endl << endl;
     cout << "How many vertices?" << endl;
     cin >> numVertices;
     
     //error catching loop for numvertices, forces user to enter numV value 4-10
     while(numVertices < 5 || numVertices > 10)
     {
         cout << "Invalid number of vertices, valid range is 5-10 inclusive." << endl;
         cout << "Try again. How many vertices?" << endl;
         cin >> numVertices;
     }
     
     
     int numInteriorEdges = calculateNumberOfInteriorEdges(numVertices);
    
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
     //set of all edges without repetition
     EDGE allEDGEs[200];
     //subset of all edges, contains only interior edges
     EDGE interiorEDGEs[200];
     //contains all edges in Kn, and includes repetition (1-2 and 2-1 both included)
     EDGE allRTKEDGEs[200];
     
     
     //this builds the set of all edges
     generateEdges(numVertices, allEDGEs);
    //this builds the set of interior edges
     generateInteriorEdges(numVertices, allEDGEs, interiorEDGEs);
     
     bool isValidSheetAssignment;
     int numEdgesAssigned = 0;
    char selection;
    cout << "Would you like to generate embeddings, or manually enter embeddings?";
    cout << endl << "Enter G to generate, M to manually enter:" << endl;
    cin >> selection;
    if((selection != 'M') && (selection != 'm') && (selection != 'G') && (selection != 'g'))
    {
        do {
            cout << endl << "Enter G to generate, M to manually enter:" << endl;
            cin >> selection;
        } while ((selection != 'M') && (selection != 'm') && (selection != 'G') && (selection != 'g'));
        
    }
    if(selection == 'M' || selection == 'm')
    {
         //continue to prompt for edge assignment until all edges (interior edges) have sheet num
         while(numEdgesAssigned < numInteriorEdges)
         {
             isValidSheetAssignment = true;
             
             do
             {
                 isValidSheetAssignment = true;
             
                 cout << "Enter desired sheet number for: " << interiorEDGEs[numEdgesAssigned].originPoint <<
                 ":" << interiorEDGEs[numEdgesAssigned].endPoint << endl;
                 cin >> currentSheet;
                 //if sheet number is out of valid range based on numSheets, keep asking until valid one entered
                 while((currentSheet > numSheets) || (currentSheet < 1))
                 {
                     cout << "Invalid sheet number. Enter another sheet number: " << endl;
                     cin >> currentSheet;
                 }
                 
                 
                 //enumerate through all edges with index lower than current edge
                 for(int index = 0; index < numEdgesAssigned; index++)
                 {
                 //check if any of them have a sheet number equal to current sheet num
                 if(interiorEDGEs[index].sheetNumber == currentSheet)
                 //if so, they will be in the same sheet, meaning they can't intersect.. check intersection
                 if(determineIfTwoEdgesIntersect(interiorEDGEs[index], interiorEDGEs[numEdgesAssigned]))
                 {
                 //if intersection occurs, we have invalid sheet assignment
                 isValidSheetAssignment = false;
                 //indicate which edges are intersecting
                 cout << "Intersection: " << interiorEDGEs[index].originPoint << ":" << interiorEDGEs[index].endPoint << " intersects " << interiorEDGEs[numEdgesAssigned].originPoint << ":" << interiorEDGEs[numEdgesAssigned].endPoint << endl <<"Try again!" << endl;
                 }
                 }
                 
             
             }while(isValidSheetAssignment == false);
             
             //exiting above loop means we have a valid sheet num for current edge, so make assignment..
             interiorEDGEs[numEdgesAssigned].sheetNumber = currentSheet;
             
             //increment counter for edges assigned
             numEdgesAssigned++;
         
         }
         
         
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
                     if((determineIfTwoEdgesIntersect(interiorEDGEs[i], interiorEDGEs[j])))
                     // || (determineIfTwoEdgesIntersect(interiorEDGEs[j], interiorEDGEs[i])))
                     {
                         //put it in the list
                         interiorEDGEs[i].intersectingEdges.push_back(&interiorEDGEs[j]);
                     }
                 }
             }
         }
         
         //generateRTKFileOutput(numVertices, numInteriorEdges, allEDGEs, interiorEDGEs);
         
         //build the set of edges to be used for RTK output format
         generateAllRTKEdges(numVertices, allRTKEDGEs);
         
         //then, output those edges and their intersections to an output file - deprecated
         //generateRTKFileOutputAllEdges(numVertices, numInteriorEdges, allRTKEDGEs, interiorEDGEs);
         
         //input ordered template for corresponding complete graph
         getFileInput(numVertices, (numVertices)*(numVertices-1), numInteriorEdges, allRTKEDGEs, interiorEDGEs);
         
         
         
         ofstream RTKFileOutput1("RTKFile.txt");//file output stream
         
         EDGE tempEDGE;//holds the edge we are currently outputting
         //cycle through all the edges in our rtk edge list..
         for(int k = 0; k < ((numVertices-1)*numVertices); k++)
         {
             //output the edge
             RTKFileOutput1 << allRTKEDGEs[k].originPoint << ":" << allRTKEDGEs[k].endPoint << " ";
             //if the list of intersections is empty output a 0
             if(allRTKEDGEs[k].intersectingEdges.empty())
             {
                 RTKFileOutput1 << "0" << endl;
             }
             else
             {
             //cycle through all elements in the list of intersections for this given edge
             for(list<EDGE*>::iterator iter = allRTKEDGEs[k].intersectingEdges.begin(), end = allRTKEDGEs[k].intersectingEdges.end();
             iter != end;
             ++iter)
             {
                 //output a comma if it's not the first item
                 if(iter != allRTKEDGEs[k].intersectingEdges.begin())
                     RTKFileOutput1 << ", ";
                 
                 //temporarily store edge data of current list item
                 tempEDGE = **iter;
                 RTKFileOutput1 << tempEDGE;
                 
                 //compare sheet numbers to determine over/under
                 if(allRTKEDGEs[k].sheetNumber > tempEDGE.sheetNumber)
                 {
                     RTKFileOutput1 << "U";
                 }
                 else
                 {
                     RTKFileOutput1 << "O";
                 }
                 
             }
             RTKFileOutput1 << endl;
         }
         }
         RTKFileOutput1.close();
    }
    else
    {
        generateEmbeddingsForGivenSheetNumAndNumVertices(numVertices, numSheets);
    }
	//char holdup;
	//cin >> holdup;
    
    return 0;
}

//////////////////////////////////////////
///////////Function Definitions///////////
//////////////////////////////////////////

void generateEmbeddingsForGivenSheetNumAndNumVertices(int numVertices, int sheetNum)
{
    int numInteriorEdges;//, sheetNumMin, sheetNumMax;
    
    numInteriorEdges = calculateNumberOfInteriorEdges(numVertices);
    //ofstream RTKFileOutput1("K" + numVertices + "_" + count);//file output stream
    EDGE allEDGEs[45];
    EDGE interiorEDGEsForOneGraph[35];
    
    generateEdges(numVertices, allEDGEs);
    generateInteriorEdges(numVertices, allEDGEs, interiorEDGEsForOneGraph);
    
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
                else
                {
                    interiorEDGEsForOneGraph[i].cooperatingEdges.push_back(&interiorEDGEsForOneGraph[j]);
                }
            }
        }
    }
    //determine upper bound of max edges per sheet, can be set manually for more accuracy
    int maxEdgesPerSheet = determineMaxEdgesPerSheet(interiorEDGEsForOneGraph, numInteriorEdges, sheetNum);//3;
    
    std::list<EDGE*>oneSheet;
    std::list<EDGE*> allSheets[200000];
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
int determineMaxEdgesPerSheet(EDGE interiorEdges[], int numInteriorEdges, int sheetNum)
{
    long maxEdgesPerSheet = 1;
    //no point in finding sheetNum if it's the numInteriorEdges
    if(sheetNum == numInteriorEdges)
        return 1;
    
    //this gives us the highest number of edges feasibly possible
    //in a given sheet, assuming intersections don't happen
    //we'll then generate such sheets regardless of intersections
    //and then discard the invalid sheets
    //return numInteriorEdges - sheetNum + 1;
    
    for(int edgeIndex = 0; edgeIndex < numInteriorEdges; edgeIndex++)
    {
        //this is how many edges a given edge Cannot be paired with
        if(interiorEdges[edgeIndex].cooperatingEdges.size() + 1 > maxEdgesPerSheet)
            maxEdgesPerSheet = interiorEdges[edgeIndex].cooperatingEdges.size() + 1;
    }
    return (int)maxEdgesPerSheet;
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

bool checkForIntersectionsBetweenEdgeAndSet(EDGE oneEdge, list<EDGE*> setOfEdges)
{
    for(list<EDGE*>::iterator iter = setOfEdges.begin(),
        end = setOfEdges.end();
        iter != end;
        ++iter)
    {
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

//Adds information to our array of valid sheets, such as number of edges
//Then, outputs all of them to a text file. Useful to ensure that the list
//of sheets you're working with is valid.
void buildSheets(list<EDGE*> validSheets[], int numValidSheets)
{
    SHEET mySheets[1000];
    for(int i = 0; i < numValidSheets; i++)
    {
        mySheets[i].edgesInSheet = validSheets[i];
        mySheets[i].numEdgesInSheet = (int)validSheets[i].size();
        //cout << endl << mySheets[i] << endl;
    }

    ofstream OneSheetOutput ("sheetList.txt");
    for(int i = 0; i < numValidSheets; i++)
    {
        OneSheetOutput << mySheets[i] << endl << endl;
    }
    OneSheetOutput.close();

    //we pass our array of valid sheets to this method to combine them in all possible ways
    crunchEmbeddingCombinations(mySheets, numValidSheets);
    
}

//Looks at the edges in two given sheets and checks if any two are the same edge
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

//Checks a given embedding to ensure sheets don't contain the same edges
bool determineIfValidEmbedding(list<EDGE*> bookRepresentation[], int numSheets)
{
    for(int sheetA = 0; sheetA < numSheets-1; sheetA++)
    {
        for(int sheetB = sheetA+1; sheetB < numSheets; sheetB++)
        {
            if(!determineIfTwoSheetsCooperate(bookRepresentation[sheetA], bookRepresentation[sheetB]))
                return false;
        }
        
    }
    return true;

}


void crunchEmbeddingCombinations(SHEET mySheets[], int numValidSheets)
{
    //We create a linked list of sheets for each possible sheet size
    //This will have to be modified based on the given complete graph
    //being evaluated.
    list<SHEET*>sizeOne;
    list<SHEET*>sizeTwo;
    list<SHEET*>sizeThree;
    list<SHEET*>sizeFour;
    list<SHEET*>sizeFive;
    
    //We create embedding combinations based on how many edges
    //are present in each sheet
    for(int index = 0; index < numSheets; index++)
    {
        switch(mySheets[index].numEdgesInSheet)
        {
            case 1:
                sizeOne.push_back(&mySheets[index]);
                break;
                
            case 2:
                sizeTwo.push_back(&mySheets[index]);
                break;
                
            case 3:
                sizeThree.push_back(&mySheets[index]);
                break;
            case 4:
                sizeFour.push_back(&mySheets[index]);
                break;
            case 5:
                sizeFive.push_back(&mySheets[index]);
                break;
        }
    }
    
    list<EDGE*> bookRepresentation[35];
    ofstream OneSheetOutput;
    
    BOOK oneEmbedding;
    
    string file_name;
    BOOK tempEmbedding;
    
    //Edit the below string to indicate desired output file name
    file_name = "2K_6_#4_embedding_#x321_2_apr_numbered"/* + to_string(embeddingCount)*/;
    OneSheetOutput.open(string(file_name+".txt").c_str());
    
    int embeddingCount = 0;
    SHEET one, two, three, four;
    
    //These loops iterate over given sized lists of edges, below loops will create 3-3-2-1 config
    for(list<SHEET*>::iterator iterA = sizeThree.begin()++++,
        end = sizeThree.end();
        iterA != end;
        ++iterA)
    {
    for(list<SHEET*>::iterator iterB = sizeThree.begin()++++,
        end = sizeThree.end();
        iterB != end;
        ++iterB)
    {
        for(list<SHEET*>::iterator iterC = sizeTwo.begin()++++,
            end = sizeTwo.end();
            iterC != end;
            ++iterC)
        {
            for(list<SHEET*>::iterator iterD = sizeOne.begin(),
                end = sizeOne.end();
                iterD != end;
                ++iterD)
            {
                //if(*iterA != NULL)
                one = **iterA;
                //if(*iterB != NULL)
                two = **iterB;
                //if(*iterC != NULL)
                three = **iterC;
                //if(*iterD != NULL)
                four = **iterD;
                
                //this is where we consolidate the sheets into a book rep so we can test its validity
                bookRepresentation[0] = one.edgesInSheet;
                bookRepresentation[1] = two.edgesInSheet;
                bookRepresentation[2] = three.edgesInSheet;
                bookRepresentation[3] = four.edgesInSheet;
                
                //then, we determine its validity
                if(determineIfValidEmbedding(bookRepresentation, numSheets) == true)
                {
                    //if the rep is valid, then we put in our storage
                    myEmbeddings[embeddingCount].sheetsInBook.push_back(*iterA);
                    myEmbeddings[embeddingCount].sheetsInBook.push_back(*iterB);
                    myEmbeddings[embeddingCount].sheetsInBook.push_back(*iterC);
                    myEmbeddings[embeddingCount].sheetsInBook.push_back(*iterD);
                    myEmbeddings[embeddingCount].sheetNumber = numSheets;

                    
                    allEmbeddings.push_back(&myEmbeddings[embeddingCount]);
                    
                    //and output it to the file
                    OneSheetOutput << one << endl << two << endl << three << endl << four;
                    OneSheetOutput << "===" << endl;
                    
                    embeddingCount++;
                    
                }
             
                
            }
        }
    }
    }
    OneSheetOutput << endl << endl << "total: " << embeddingCount;
    OneSheetOutput.close();
    
  

    cout << endl << "total: " << embeddingCount << " embeddings. " << endl;

}

//Evaluates two embeddings and determines if they have the same edges
//present in the same sheets with the same edge to sheet distribution
bool areEmbeddingsEqual(BOOK embedA, BOOK embedB, int numSheets)
{
    int matches = 0;
    //bool areEqual = false;
    BOOK tempA = embedA;
    BOOK tempB = embedB;
    for(list<SHEET*>::iterator iterA = tempA.sheetsInBook.begin(),
        end = tempA.sheetsInBook.end();
        iterA != end;
        ++iterA)
    {
        
        for(list<SHEET*>::iterator iterB = tempB.sheetsInBook.begin(),
            end = tempB.sheetsInBook.end();
            iterB != end;
            ++iterB)
        {
            if(iterA == iterB)
            {
                tempA.sheetsInBook.erase(iterA);
                tempB.sheetsInBook.erase(iterB);
                matches++;
            }
        }
    }
    
    if(matches == numSheets)
    {
        cout << "duplicate" << endl;
        return true;
    }
    else
        return false;
}


//Determines if the edges from a given sheet can be moved to adjacent sheets
/*bool canEdgesBeMovedToAdjSheets(SHEET currentSheet, SHEET toLeft, SHEET toRight)
{
    char kk;
    
    cout << "TESTINGLOLOL" << endl;
    cout << "L:" << toLeft;
    cout << endl << currentSheet;
    cout << endl << "R:" << toRight;
    for(list<EDGE*>::iterator iterA = currentSheet.edgesInSheet.begin(),
        end = currentSheet.edgesInSheet.end();
        iterA != end;
        ++iterA)
    {
        if(canEdgeBeInGivenSheet(**iterA, toLeft))
        {
            cout << endl << **iterA << " can be moved to " << toLeft;
            return true;
        }
        else if(canEdgeBeInGivenSheet(**iterA, toRight))
        {
            cout << endl << **iterA << " can be moved to " << toRight;
            return true;
        }
        //cin >> kk;
    }//why is this never returning false..
    
    return false;
}
*/


bool canEdgeBeInGivenSheet(EDGE oneEdge, SHEET destinationSheet)
{
    //look through the given sheet at each edge
    for(list<EDGE*>::iterator iterA = destinationSheet.edgesInSheet.begin(),
        end = destinationSheet.edgesInSheet.end();
        iterA != end;
        ++iterA)
    {
        //if an intersection occurs then edge cannot be in said sheet
        if(determineIfTwoEdgesIntersect(oneEdge, **iterA))
        {
            cout << endl << oneEdge << " and " << **iterA << " intersect.";
            return false;
        }
    }
    //no intersections occurred, edge can be in sheet
    return true;
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

//Builds a set of edges required for RTK output format, important distinction
//is that 2:3 and 3:2 are both represented, for example
int generateAllRTKEdges(int numVertices, EDGE allRTKEDGEs[])
{
    int count = 0;
    //origin points for all edges range from 1-n
    for(int i = 0; i < numVertices; i++)
    {
        
        //same for end points, except..
        for(int j = 0; j < numVertices; j++)
        {
            //they never = the origin, so don't add 1-1 or 2-2, for ex
            if(j != i)
            {
                allRTKEDGEs[count].originPoint = i+1;
                allRTKEDGEs[count].endPoint = j+1;
                count++;
            }
        }
    }
    
    return count;
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

//overloading output operator so we can use << operator to output edges
std::ostream& operator << (std::ostream &o, EDGE &a)
{
    o << a.originPoint << "-" << a.endPoint;
    return o;
}
//retrieves file input required to generate an ordered rtk file
//k10 input file is unfinished so k10 rtk files will not be ordered
void getFileInput(int numVertices, int numEdges, int numInteriorEdges, EDGE allEDGEs[], EDGE interiorEDGEs[])
{
	EDGE tempEDGE;
	int edgeCount = 0;
    
    int origin, end;//store current values for origin and end points as we're getting file input
	ifstream input;// ("K5_input.txt");
	
	//determines which input text file to open based on num vertices
	switch(numVertices)
	{
		case 5:
            input.open("K5_input.txt");
            break;
            
		case 6:
            input.open("K6_input.txt");
            break;
            
		case 7:
            input.open("K7_input.txt");
            break;
            
		case 8:
            input.open("K8_input.txt");
            break;
            
		case 9:
            input.open("K9_input.txt");
            break;
            
            //k10 input template file isn't complete
		case 10:
            input.open("K10_input.txt");
            cout << endl << "ERROR:Source file for K10 not ordered..." << endl;
            return;
            
		default:
            cout << endl << "ERROR:Invalid number of vertices. Quitting.." << endl;
            return;
            
	}
	
	//cycle through all edges in set of allEDGEs[]
	for(int i = 0; i < ((numVertices-1)*numVertices); i++)
    {
        
        //cycling through all interior edges
        for(int j = 0; j < numInteriorEdges; j++)
        {
            //our set of interiorEdges does not have repeats(includes 3-5 but not 5-3 as well)
            //so, check each edge from our super set of all edges against each interior edge
            //and see if we can find an interior edge that it is equal to 1-2 = 1-2 and 2-1, for ex.
            if(determineIfTwoEdgesEquivalent(allEDGEs[i], interiorEDGEs[j]))
            {
                //same edge, so copy sheet number
                allEDGEs[i].sheetNumber = interiorEDGEs[j].sheetNumber;
                
                //we found its equivalent edge so stop looking, move on to next edge
                break;
            }
            
        }
        //if an edge doesn't have a sheet number at this point it's a non-interior edge
        //so, we set it to 1
        if(!allEDGEs[i].sheetNumber)
            allEDGEs[i].sheetNumber = 1;
    }
    
	//string for our stringstream
    string s;
    
    while(getline(input, s)) // fetch each line
    {
		cout << endl;
        stringstream line(s);
        //as long as current line has integer values..
        while(line >> origin)
		{
			//store 2nd as endpoint
			line >> end;
			
			tempEDGE.originPoint = origin;
			tempEDGE.endPoint = end;
			
			//cout << origin << "-" << end << " ";
			//cycle trhough all interior edges to find right one, store it in intersections for current edge
			for(int j = 0; j < numInteriorEdges; j++)
			{
				if(determineIfTwoEdgesEquivalent(tempEDGE, interiorEDGEs[j]))
				{
					allEDGEs[edgeCount].intersectingEdges.push_back(&interiorEDGEs[j]);
				}
			}
            
		}
		//every time we go to a new line we are counting another edge
		edgeCount++;
    }
	//close input stream
	input.close();
}
//Permutation methods, used for generating permutations of sheet distribution partitions
/*
void swap(int *v, const int i, const int j)
{
    int t;
    t = v[i];
    v[i] = v[j];
    v[j] = t;
}

void print(const int *v, const int size)
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
    print(v, n);
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
} // permute
*/