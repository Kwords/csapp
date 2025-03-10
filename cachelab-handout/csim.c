#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
typedef struct Node Node;
typedef struct LRU LRU;
int display;
int hits, misses, evictions;
// 1<<s rows E max lines 1<<b data_size
int s,E,b;
char* fileName;
struct Node{
    unsigned tag;
    Node* next;
    Node* pre;
};
struct LRU {
    int* count;
    Node* start;
    Node* end;
};
LRU* lru;
void parseOption(int argc,char** argv){
    int opt;
    while((opt=getopt(argc,argv,"s:E:b:t:v"))!=-1){
        switch (opt) {
            case 'v':
                display=1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                strcpy(fileName,optarg);
                break;
        }
    }
}
void initCache(){
    int S = 1<<s;

    lru = malloc(S*sizeof(LRU));
    for(int i=0;i<S;i++) {
        lru[i].start = malloc(sizeof(Node));
        lru[i].end = malloc(sizeof(Node));
        lru[i].count = malloc(sizeof(int));
        *lru[i].count = 0;
        lru[i].start->next=lru[i].end;
        lru[i].end->pre=lru[i].start;
    }
}
void deleteNode(Node* node, int isFree) {
    node->next->pre = node->pre;
    node->pre->next = node->next;
    node->next = NULL;
    node->pre = NULL;
    if(isFree) free(node);
}
void addFirst(Node* node, Node* start) {
    node->next = start->next;
    node->pre = start;
    start->next->pre = node;
    start->next = node;
}
void update(unsigned address) {
    unsigned mx = 0xFFFFFFFF;
    // get set index
    unsigned targetSet = (address>>b) & mx>>(32-s);
    unsigned targetTag = (address>>(b+s));
    int hitFlag = 0;
    LRU* tempLru = &lru[targetSet];
//    printf("!%d\n",*tempLru->count);
    Node* tempNode = tempLru->start->next;
    while(tempNode != tempLru->end) {
        if(tempNode->tag == targetTag) {
            hitFlag = 1;
            break;
        }
        tempNode = tempNode->next;
    }
    //printf("!%d\n",hitFlag);
    if(hitFlag) {
        printf(" hit");
        hits++;
        deleteNode(tempNode, 0);
        addFirst(tempNode, tempLru->start);
    } else {
        Node* newNode = malloc(sizeof(Node));
        newNode->tag = targetTag;
        if((*tempLru->count) == E) {
            printf(" miss eviction");
            evictions++;
            misses++;
            deleteNode(tempLru->end->pre, 1);
            addFirst(newNode, tempLru->start);
        } else {
            printf(" miss");
            misses++;

            (*tempLru->count)++;
            addFirst(newNode, tempLru->start);
        }
    }
}
void cacheSimulate(){
    initCache();
    FILE* file = fopen(fileName, "r");
    char op;
    unsigned address;
    int size;
    while(fscanf(file, " %c %x,%d", &op, &address, &size) > 0) {
        if(display) printf("%c %x,%d", op, address, size);
        switch (op) {
            case 'L':
                update(address);
                break;
            case 'S':
                update(address);
                break;
            case 'M':
                update(address);
                update(address);
                break;
        }
        printf("\n");
    }
}
void freeCache(){
    free(lru);
}
int main(int argc,char** argv)
{

    fileName = malloc(100*sizeof(char));
    parseOption(argc,argv);
    cacheSimulate();
    freeCache();
    printSummary(hits, misses, evictions);
    free(fileName);
    return 0;
}
