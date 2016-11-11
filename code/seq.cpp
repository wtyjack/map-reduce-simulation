/**********************************************************************
ECE 563 Large Project
Counting words by map-reduce framework
Author: Tianye Wang, Yan Xia
Creation Date: 4/15/2016
**********************************************************************/

#include "Reader.h"
#include "Writer.h"
#include "Mapper.h"
#include "Reducer.h"
#include <iostream>
#include <functional>
#include <dirent.h>
#include <unistd.h>
#include <omp.h>

#define IOERR 1

int main(int argc, char** argv) {

    concurrent_queue<std::string> readQ;
    concurrent_queue<std::pair<std::string, int>> reduceQ, writeQ;
    double start, end;

    DIR *pDir;
    struct dirent *pEnt;
    FILE *pFile;
    std::vector<std::string> rawFileList;
    // Read input directory
    start = omp_get_wtime(); 

    pDir = opendir(argv[1]);
    if (pDir == NULL) {
        std::cout << "Cannot open directory " << argv[1] << std::endl;
        return IOERR;
    }

    while ((pEnt = readdir(pDir)) != NULL) {
        if (pEnt->d_name[0] != '.'){
            //std::cout << "Found: " << pEnt->d_name << std::endl;
            rawFileList.push_back(std::string(argv[1])+'/'+std::string(pEnt->d_name));
            std::cout << "Found: " << std::string(pEnt->d_name) << std::endl;
        }
    }
    closedir(pDir);

    // Read files
    Reader reader(rawFileList);
    reader.feedReadQ(&readQ);
    end = omp_get_wtime(); 
    std::cout << "Reader done time: " << end-start << std::endl;


    // Mapping
    Mapper mapper;
    std::string word;
    int mapCount = 0;
    while (!readQ.empty()) {
        word = mapper.getWord(&readQ);
        mapper.addWordPair(word, 1);
        mapCount++;
    }
    std::cout << "Map finished! -- " << mapCount << std::endl;
    mapper.dumpPair(&reduceQ);

    end = omp_get_wtime(); 
    std::cout << "Mapper done time: " << end-start << std::endl;

    // Reduce
    Reducer reducer;
    while (!reduceQ.empty()) {
        std::pair<std::string, int> p  = reducer.getPair(&reduceQ);
        reducer.reduce(p);
    }
    reducer.dumpPair(&writeQ);


    // Write the result to file
    std::string outPath = "./output/";
    std::string outFile = "result_seq";
    int writeCount = 0;
    Writer writer(outPath+outFile);
    while (!writeQ.empty()) {
        pair<std::string, int> p = writeQ.pop();
        if (p.first == "lively") {
            std::cout << "WriteQ Find!!!!" << std::endl;
        }
        writer.writeFile(p);
        writeCount++;
    }
    std::cout << "Writing finished! -- " << writeCount << std::endl;
    end = omp_get_wtime(); 
    std::cout << "Total time: " << end-start << std::endl;
}



