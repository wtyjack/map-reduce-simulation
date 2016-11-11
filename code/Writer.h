#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "Queue.h"

using namespace std;

class Writer
{
private:
    ofstream ofs;
public:
	// construct with output file name to be written by this writer
    Writer(){}
	Writer(string);
	// destructor
	~Writer();
    void setPath(string);
    pair<string, int> getPair(concurrent_queue< pair<string,int> >*);
	// write <word,count> tuple in write queue into output file
	void writeFile(pair<string, int>);	
};
