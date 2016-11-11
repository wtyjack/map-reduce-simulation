#ifndef READER_H
#define READER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include "Queue.h"

using namespace std;

class Reader
{
private:
	int tid;
	int idx;
	vector<string> fileList;
public:
	// read queue stored at each reader
	// myQueue<string> localReadQ;
	// construct with thread id, loop index and list of files to be read by this reader
	// Reader(int threadId, int index, vector<string> filesToRead);
    Reader(){}
	Reader(vector<string> filesToRead);
    Reader(int idx) {this->idx = idx;}
	// desctructor
	~Reader(){}
	// feed reader with thread id, loop index and list of files to be read by this reader
	void feedReader(int threadId, int index, vector<string> filesToRead);
	// feedReadQ() feeds the read queue
	void feedReadQ(concurrent_queue<string>*);
    int feedReadQ(concurrent_queue<string>*, string);
	// finish(readFinishFlag) set read complete flag 
	void finish(bool* readFinshFlag);
};

#endif
