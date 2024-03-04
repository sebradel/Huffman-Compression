//Sebastian Radel
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
Global arrays
    bits: used to store binary codes during tree traversal
    code: used to store binary encoding for each character in input file
    codeLen: used to store the length of each character's binary encoding
    frequency: used to store the frequency of each character in the input file
*/
unsigned bits[32], code[256], codeLen[256], frequency[256];

//Used for bitshift operations
unsigned zero = 0;
unsigned one = 1;

//Singly-linked list used to store tree nodes
struct listNode {
    struct treeNode* node;
    struct listNode* next;
};

//Struct for Huffman tree
struct treeNode {
    char value;
    int sum;
    struct treeNode* left;
    struct treeNode* right;
};

//Pointer to head of linked list of tree nodes
struct listNode* head;

//Pointer to root of Huffman tree
struct treeNode* root;

//Sorts the linked-list of tree nodes
void sortNodeList(int nodeCount) {
    struct listNode *currentNode, *nextNode, *previousNode, *tempNode;
    int swap = 0, firstIter = 0;
    currentNode = head;

    //Simple bubble sort algorithm to sort the list
    for(int i=0; i<=nodeCount; i++) {
        swap = 0;
        firstIter = 0;
        currentNode = head;
        while(currentNode->next != NULL) {

            nextNode = currentNode->next;
            if((currentNode->node)->sum > (nextNode->node)->sum) {
                currentNode->next = nextNode->next;
                nextNode->next = currentNode;
                if(currentNode!=head) {
                    previousNode->next = nextNode;
                } else {
                    head = nextNode;
                }
                tempNode = currentNode;
                currentNode = nextNode;
                nextNode = tempNode;
                swap = 1;
            }
            previousNode = currentNode;
            currentNode = currentNode->next;
        }
        //If no swaps are made during an iteration, the list is sorted,
        //and loop can break
        if(swap == 0) {
            break;
        }
    } 
}

//Traverses Huffman tree recursively and stores the encoding for each leaf
void traverseTree(struct treeNode *input, int index) {

    //Recursive calls if current node is not a leaf. Updates the bits array
    if(input->left!=NULL) {
        bits[index] = 0;
        traverseTree(input->left, index+1);
    }
    if(input->right!=NULL) {
        bits[index] = 1;
        traverseTree(input->right, index+1);
    }
    //Storing the current encoding in the bits array in the code array
    if(input->left==NULL && input->right==NULL) {
        codeLen[input->value] = index;
        unsigned temp = 0;

        for(int i=0; i<index; i++) {
            if(bits[i]==0) {
                temp = temp << 1;
            } else {
                temp = temp << 1;
                temp = temp + one;
            }
            
        }
        code[input->value] = temp;
    }
    return;
}

//Creates the Huffman tree
struct treeNode* createTree() {
    int nodeCount = 0;
    struct treeNode *tempPtr;
    struct listNode *newestNode, *previousNode, *smallerListNode, *largerListNode;

    //Creates leaf nodes for each character, and stores them in the linked list
    for(int i=0; i<256; i++) {
        if(frequency[i] != 0) {
            tempPtr = malloc(sizeof(struct treeNode));
            tempPtr->value = i;
            tempPtr->sum = frequency[i];
            tempPtr->left = NULL;
            tempPtr->right = NULL;
            newestNode = malloc(sizeof(struct listNode));
            newestNode->node = tempPtr;
            newestNode->next = NULL;
            if (nodeCount == 0) {
                head = newestNode;
            } else {
                previousNode->next = newestNode;
            }
            previousNode = newestNode;
            nodeCount++;
        }
    }
    sortNodeList(nodeCount);
    
    //Creates the rest of the nodes of the tree
    while(nodeCount>1) {
        tempPtr = malloc(sizeof(struct treeNode));
        newestNode = malloc(sizeof(struct listNode));
        smallerListNode = head;
        largerListNode = head->next;
        tempPtr->sum = (int)((smallerListNode->node)->sum) + (int)((largerListNode->node)->sum);
        tempPtr->left = largerListNode->node;
        tempPtr->right = smallerListNode->node;
        newestNode->node = tempPtr;
        newestNode->next = largerListNode->next;

        head = newestNode;
        nodeCount--;
        sortNodeList(nodeCount);
    }
    root = head->node;
    return root;
}

//Scans input file, and prints each character's binary encoding to
//the output file
void printCodes(FILE *inputFile, FILE *outputFile) {
    unsigned outputBuffer[1];
    char inputBuffer[1];
    unsigned bitTester = 2147483648;
    int bitsRemaining = 8;
    while(fread(inputBuffer, 1, 1, inputFile)) {
        unsigned shiftedInput = code[inputBuffer[0]] << (32-codeLen[inputBuffer[0]]);

        //Prints encoding for current character to file or stores encoding
        //in outputBuffer
        for(int i=0; i<codeLen[inputBuffer[0]]; i++) {

            //If outputBuffer is full, prints it to file, and resets it
            if(bitsRemaining == 0) {
                fwrite(outputBuffer, 1, 1, outputFile);
                outputBuffer[0] = 0;
                bitsRemaining = 8;
            }
            outputBuffer[0] = outputBuffer[0] << 1;
            if((bitTester&shiftedInput) == bitTester) {
                outputBuffer[0] = outputBuffer[0] + 1;
            }
            shiftedInput = shiftedInput << 1;
            bitsRemaining--;
        }
        
        //If outputBuffer was filled in last iteration of for loop, prints
        //outputBuffer and resets it
        if(bitsRemaining == 0) {
            fwrite(outputBuffer, 1, 1, outputFile);
            outputBuffer[0] = 0;
            bitsRemaining = 8;
        }
    }

    //If there are bits leftover in outputBuffer after input stream ends,
    //shifts these bits to the left and prints the final byte
    if(outputBuffer[0]!=0) {
        outputBuffer[0] = outputBuffer[0] << bitsRemaining;
        fwrite(outputBuffer, 1, 1, outputFile);
        
    }
}

//Main function
int main(int argc, char *argv[]) {
    char buffer[1];         //input buffer
    char* inputFileName = NULL;         
    char* outputFileName = NULL;
    FILE *inputFilePtr;
    FILE *outputFilePtr;
    char paramLabel;
    while((paramLabel = getopt(argc, argv, "i:o:"))!=-1) {
        switch (paramLabel) {
            case 'i':
                inputFileName = optarg;
                break;
            case 'o':
                outputFileName = optarg;
                break;
        }
    }

    //Default case if no parameters were given with executable
    if ((inputFileName==NULL) && (outputFileName==NULL)) {
        inputFileName = "completeShakespeare.txt";
        outputFileName = "huffman.out";
    }
    inputFilePtr = fopen(inputFileName, "r");
    
    //Read input file and populate frequency array
    while(fread(buffer, 1, 1, inputFilePtr)) {
        frequency[buffer[0]] = frequency[buffer[0]] + 1;
    }
    fclose(inputFilePtr);

    //Open input file again, and create the Huffman tree
    inputFilePtr = fopen(inputFileName, "r");
    struct treeNode *root = createTree();

    //Traverse the tree and save codes for each character
    traverseTree(root, 0);
    
    //Open the output file and print codes to it
    outputFilePtr = fopen(outputFileName, "w");
    printCodes(inputFilePtr, outputFilePtr);

    //Close files and return
    fclose(inputFilePtr);
    fclose(outputFilePtr);
    return 0;
}