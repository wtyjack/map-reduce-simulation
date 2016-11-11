#include "Writer.h"

using namespace std;

Writer::Writer(string outPath) {
    ofs.open(outPath, ofstream::out);
}

Writer::~Writer() {
    ofs.close();
}

void Writer::setPath(string outPath) {
    if (ofs.is_open()) {
        ofs.close();
    }
    ofs.open(outPath, ofstream::out | ofstream::app);
}

pair<string, int> Writer::getPair(concurrent_queue< pair<string,int> > *Q) {
    std::pair<std::string, int> p("", -1);
    if (!Q->empty()) {
        p = Q->pop();
    }
    return p;
}

void Writer::writeFile(pair<string, int> p) {
    if (ofs.is_open()) {
        ofs << p.first << ", " << p.second << endl;
    }
    else {
        cout << "Writer: File open failed." << endl;
    }
}
