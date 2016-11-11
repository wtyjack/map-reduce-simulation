#include "Reader.h"

using namespace std;

Reader::Reader(vector<string> filesToRead) {
 	fileList = filesToRead;
}

void Reader::feedReader(int threadId, int index, vector<string> filesToRead) {
	tid = threadId;
	idx = index;
	fileList = filesToRead;
}

int Reader::feedReadQ(concurrent_queue<string>* readQ, string fileName) {
    int count = 0;
    ifstream infile (fileName.c_str());
    string line;
    string buf;
    if (infile.is_open()) {
        while (getline(infile, line)) {
            replace_if(line.begin(), line.end(), ptr_fun<int, int>(&std::ispunct), ' ');
            stringstream ss(line);
            while (ss >> buf) {
                readQ->push(buf);
                count++;
            }
        }
    }
    else {
        printf("Cannot open file %s \n", fileName.c_str());
    }
    return count;
}

void Reader::feedReadQ(concurrent_queue<string>* readQ) {
    int count = 0;
    for (string fileName: fileList) {
        ifstream infile (fileName.c_str());
        string line;
        string buf;
        if (infile.is_open()) {
            while (getline(infile, line)) {
                replace_if(line.begin(), line.end(), ptr_fun<int, int>(&std::ispunct), ' ');
                stringstream ss(line);
                // push word into local read queue stored within reader
                // #pragma omp critical
                while (ss >> buf) {
                    readQ->push(buf);
                    count++;
                }
            }
        }
        else {
            printf("Cannot open file %s \n", fileName.c_str());
        }
    }
    cout << "Total push" << count << endl;
}

void Reader::finish(bool* readFinshFlag) {
    readFinshFlag[idx] = false;
}
