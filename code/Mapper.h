#ifndef MAPPER_H
#define MAPPER_H

#include <iostream>
#include <fstream>
#include <vector>
//#include <queue>
#include <string>
#include <cstring>
#include <unordered_map>
#include <functional>
#include "Mypair.h"
#include "Queue.h"

class Mapper
{
private:
	int tid;
	int idx;
	std::unordered_map<std::string, int> wordMap;
public:
	Mapper(){}
    Mapper(int idx) {this->idx = idx;}
	Mapper(int tid, int idx);
	~Mapper();
    // getWord() read in one word from queue;
    std::string getWord(concurrent_queue<std::string>*);
	// addWord() read in one word and update the wordTable
	void addWordPair(std::string, int);
	// pushWord() push <word, count> pair into reduceQ
	void pushWord();
	// finish(mapFinishFlag) set map complete flag 
	void finish(bool *);
	// hash function to determine which reducer queue to feed
    void dumpPair(std::vector<MYPAIR>*, int);
    void dumpPair(concurrent_queue<std::pair<std::string, int>>*);
	int hashQ(std::string, int);
    void showAll();
    int mapSize() {return wordMap.size();}
};

#endif
