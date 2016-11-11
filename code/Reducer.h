#ifndef REDUCER_H
#define REDUCER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include "Queue.h"

class Reducer
{
private:
	int tid;
	int idx;
    std::unordered_map<std::string, int> wordMap;
public:
    Reducer(){}
    Reducer(int);
	Reducer(int, int);
    ~Reducer();
    // getPair(): get a word pair from reducer work queue
    std::pair<std::string, int> getPair(concurrent_queue< std::pair<std::string,int> >*);
    // reduce(): merge the pair into its wordMap, and update the value
    void reduce(std::pair<std::string, int>);
    // pushToWriteQ(): push the reduced pairs to writeQ after finishing
    void dumpPair(concurrent_queue<std::pair< std::string, int> >*);
    // finish(): set the corresponding flag when all jobs are done
    void finish(bool*);
    void showAll();
    int reduceSize() {return wordMap.size();}
};

#endif
