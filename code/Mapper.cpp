/* 
subject : ECE-563-Large-Project-OpenMP-version
author  : Tianye Wang, Yan Xia
date    : 03/23/2016 
*/

#include "Mapper.h"


// Mapper member functions
Mapper::Mapper(int tid, int idx)
{
    this->tid = tid;
    this->idx = idx;
}

Mapper::~Mapper()
{
}

std::string Mapper::getWord(concurrent_queue<std::string>* Q)
{
    std::string s = "";
    if (!Q->empty()) {
        //s = Q->front();
        s = Q->pop();
    }
    return s;
}

void Mapper::addWordPair(std::string word, int value)
{
    std::unordered_map<std::string, int>::iterator it = wordMap.find(word);
    if (it != wordMap.end()) {
        // If key exists, update the value
        it->second += value;
    }
    else {
        // If key doesn't exist, insert a new pair
        std::pair<std::string, int> p(word, value);
        wordMap.insert(p);
    }
}

void Mapper::finish(bool *flag)
{
    flag[idx] = false;
}

void Mapper::dumpPair(std::vector<MYPAIR>* Q, int size) 
{
    std::hash<std::string> hash_fn;
    for (auto it = wordMap.begin(); it != wordMap.end(); ++it) {
        size_t hash = hash_fn(it->first);
        int i = hash % size;
        MYPAIR p;
        strcpy(p.word, it->first.c_str());
        p.value = it->second;
        //std::cout << i << "---"<< it->first << "," << it->second << std::endl;
        Q[i].push_back(p);
    }
}

void Mapper::dumpPair(concurrent_queue<std::pair< std::string, int> >* Q) 
{
    for (auto it = wordMap.begin(); it != wordMap.end(); ++it) {
        Q->push(*it);
    }
}

void Mapper::showAll()
{
    for (auto it = wordMap.begin(); it != wordMap.end(); ++it) {
        std::cout << "( " << it->first << ", " << it->second << " )" << std::endl;
    }
}


