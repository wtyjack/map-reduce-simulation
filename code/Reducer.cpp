
#include "Reducer.h"

Reducer::Reducer(int idx) 
{
    this->idx = idx;
}

Reducer::Reducer(int tid, int idx) 
{
    this->tid = tid;
    this->idx = idx;
}

Reducer::~Reducer(){}

std::pair<std::string, int> Reducer::getPair(concurrent_queue< std::pair<std::string,int> >*Q) 
{
    std::pair<std::string, int> p("", -1);
    if (!Q->empty()) {
        p = Q->pop();
    }
    return p;
}

void Reducer::reduce(std::pair<std::string, int> p) 
{
    std::unordered_map<std::string, int>::iterator it = wordMap.find(p.first);
    if (it != wordMap.end()) {
        // If key exists, update the value
        it->second += p.second;
    }
    else {
        // If key doesn't exist, insert a new pair
        wordMap.insert(p);
    }
    
}

void Reducer::dumpPair(concurrent_queue< std::pair<std::string,int> >*Q) 
{
    for (auto it = wordMap.begin(); it != wordMap.end(); ++it) {
        Q->push((*it));
    }
}

void Reducer::finish(bool *flag)
{
    flag[idx] = false;
}

void Reducer::showAll()
{
    for (auto it = wordMap.begin(); it != wordMap.end(); ++it) {
        std::cout << "( " << it->first << ", " << it->second << " )" << std::endl;
    }
}
