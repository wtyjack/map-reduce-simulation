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
#include "Mypair.h"
#include <iostream>
#include <functional>
#include <dirent.h>
#include <unistd.h>
#include <omp.h>
#include <mpi.h>

#define NUM_MAPPERS 1
#define NUM_REDUCERS 1
#define IOERR 1
#define MASTER 0


int main(int argc, char** argv) {
    int rank, pnum;
    // Timing
    double start, end;
    start = omp_get_wtime();

    // Initialized and set up MPI environment
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &pnum);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int notif = 0;
    bool readFlag = true, reduceFlag = true;
    concurrent_queue<std::string> readQ;
    std::vector<MYPAIR> mapQ[pnum];
    concurrent_queue<std::pair<std::string, int>> writeQ[NUM_REDUCERS];
    concurrent_queue<std::pair<std::string, int>> reduceQ[NUM_REDUCERS];


    MPI_Status status;
    MPI_Request send_request,recv_request;

    // Create MYPAIR data type for MPI
    int nitems = 2;
    int nlengths[2] = {50,1};
    MPI_Datatype types[2] = {MPI_CHAR, MPI_INT};
    MPI_Datatype mpi_pair;
    MPI_Aint offsets[2];
    offsets[0] = offsetof(MYPAIR, word);
    offsets[1] = offsetof(MYPAIR, value);
    MPI_Type_create_struct(nitems, nlengths, offsets, types, &mpi_pair);
    MPI_Type_commit(&mpi_pair);

    end = omp_get_wtime();
    std::cout << rank << ": starting time - " << end-start << std::endl;
    
    // MASTER PROCESS
    if (rank == MASTER) {
        DIR *pDir;
        struct dirent *pEnt;
        FILE *pFile;
        std::vector<std::string> rawFileList;
        // Read input directory
        pDir = opendir(argv[1]);
        if (pDir == NULL) {
            std::cout << "Cannot open directory " << argv[1] << std::endl;
            return IOERR;
        }

        while ((pEnt = readdir(pDir)) != NULL) {
            if (pEnt->d_name[0] != '.'){
                //std::cout << "Found: " << pEnt->d_name << std::endl;
                rawFileList.push_back(std::string(argv[1])+'/'+std::string(pEnt->d_name));
                //std::cout << "Found: " << std::string(pEnt->d_name) << std::endl;
            }
        }
        closedir(pDir);
        end = omp_get_wtime();
        std::cout << rank << ": Read filename time - " << end-start << std::endl;
        
        // waiting for request and distribute file name
        int notif = 0;
        int fNum = rawFileList.size();
        int count = 0, end = 0;
        while (true) {
            MPI_Recv(&notif, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            if (count < fNum) {
                std::string filename = rawFileList[count];
                MPI_Ssend((void *)filename.c_str(), filename.length()+1, MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                count++;
            }
            else {
                MPI_Ssend(&notif, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
                //std::cout << rank << ": no files to be sent to " << status.MPI_SOURCE << std::endl;
                end++;
                if (end == pnum-1) {
                    end = omp_get_wtime();
                    //std::cout << rank << ": Send filename time - " << end-start << std::endl;
                    break;
                }
            }
        }

    }
    // WORKER PROCESSES
    else {
        #pragma omp parallel num_threads(NUM_MAPPERS+1)
        {
            int tid = omp_get_thread_num();
            if (tid < NUM_MAPPERS) {
                // Mappers start working
                //std::cout << rank << ": Mapper " << tid << " started!" << std::endl;
                Mapper mapper;
                int mapCount = 0;
                while (true) {
                    std::string word = "";
                    #pragma omp critical
                    if (!readQ.empty()) {
                        word = mapper.getWord(&readQ);
                    }
                    if (word != "") {
                        mapper.addWordPair(word, 1);
                        mapCount++;
                    }
                    else {
                        if (!readFlag) {
                            break;
                        }
                    }
                }
                //std::cout << rank << ": Mapper " << tid << " finished!" << std::endl;
                // Dump the pairs to queue to be sent to reducer 
                if (tid == 0) {
                    end = omp_get_wtime();
                    //std::cout << rank << ": Mapper before dumping time - " << end-start << std::endl;
                }
                #pragma omp critical
                mapper.dumpPair(mapQ, pnum);
            }
            #pragma omp single
            {
                std::string filename;
                int size;
                char buf[50];
                Reader reader;
                while (true) {
                    // Request filename from master
                    MPI_Ssend(&notif, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
                    MPI_Probe(MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    if (status.MPI_TAG == 0) {
                        // Receive filename
                        MPI_Get_count(&status, MPI_CHAR, &size);
                        //char *buf = (char *)malloc(sizeof(char)*size);
                        memset(buf, 0, 50);
                        MPI_Recv(buf, size, MPI_CHAR, MASTER, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        filename = std::string(buf,size-1);
                        //std::cout << rank << ": Reader - " << filename << "recieved!" << std::endl;
                        //#pragma omp task firstprivate(filename)
                        //{
                            // Read file
                            int readCount = reader.feedReadQ(&readQ, filename);
                            end = omp_get_wtime();
                            //std::cout << rank << ": Reader " << omp_get_thread_num() << "- "  << readCount << "-" << end-start << std::endl;
                        //}
                    }
                    else {
                        // No more file (tag == 1)
                        MPI_Recv(&notif, 1, MPI_INT, MASTER, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        //std::cout << rank << ": no files to be received " << std::endl;
                        end = omp_get_wtime();
                        //std::cout << rank << ": Recv all filename time - " << end-start << std::endl;
                        break;
                    }
                }
                //#pragma omp taskwait
                readFlag = false;
            }
        }
    }
    end = omp_get_wtime();
    std::cout << rank << ": After read/map time " << end-start << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);
    //std::cout << "WTF!!!" << std::endl;
    
    // Gather the mapQ array to corresponding node
    //std::cout << rank << ": Gathering started!" << std::endl;
    MYPAIR *rbuf; 
    int bufsize = 0;
    int mapqsize;
    int rcounts[pnum][pnum];
    int displs[pnum][pnum];
    for (int i = 0; i < pnum; i++) {
        mapqsize = mapQ[i].size();
        MPI_Allgather(&mapqsize, 1, MPI_INT, rcounts[i], 1, MPI_INT, MPI_COMM_WORLD);
    }

    for (int i = 0; i < pnum; i++) {
        bufsize += rcounts[rank][i];
        displs[i][0] = 0;
        for (int j = 1; j < pnum; j++) {
            displs[i][j] = displs[i][j-1] + rcounts[i][j-1];
        }
    }
    rbuf = (MYPAIR *)malloc(bufsize*sizeof(MYPAIR)); 
    for (int i = 0; i < pnum; i++) {
        MPI_Gatherv(&mapQ[i][0], mapQ[i].size(), mpi_pair, rbuf, rcounts[i], displs[i], mpi_pair, i, MPI_COMM_WORLD);
    }
    end = omp_get_wtime();
    std::cout << rank << ": After gathering time " << end-start << std::endl;

    // Start Reduce and write result
    #pragma omp parallel num_threads(NUM_REDUCERS + 1)
    {
        int tid = omp_get_thread_num();
        if (tid < NUM_REDUCERS) {
            Reducer reducer;
            int reduceId = tid;
            int reduceCount = 0;
            concurrent_queue<std::pair<std::string, int>> *writeQRef = &writeQ[reduceId];

            //std::cout << rank << ": Reducer " << reduceId << " started!" <<  std::endl;

            while (true) {
                std::pair<std::string,int> p("", 0);
                if (!reduceQ[reduceId].empty()) {
                    p = reducer.getPair(&(reduceQ[reduceId]));
                }
                if (p.second != 0) {
                    reducer.reduce(p);
                    reduceCount++;
                }
                else {
                    if (!reduceFlag) {
                        break;
                    }
                }
            }
            //std::cout << rank << ": Reducer " << reduceId << " finished!" <<  std::endl;
            //std::cout << rank << ": Reducer " << reduceId << "-" << reduceCount << std::endl;
            reducer.dumpPair(writeQRef);
            //std::cout << rank << ": wQ - " << writeQ.size() << std::endl;
            if (tid == 0) {
                end = omp_get_wtime();
                std::cout << rank << ": reduce finished time - " << end-start << std::endl;
            }
            
            // Write the result to file
            std::string outPath = "./output/";
            std::string outFile = "p_result" + std::to_string((long long)rank) + "_" + std::to_string((long long)reduceId);
            Writer writer(outPath+outFile);
            //std::cout << rank << ": wQ - " << writeQ.size() << std::endl;
            //std::cout << rank << ": Writer started!" <<  std::endl;
            //pair<std::string, int> p;
            while (!writeQRef->empty()) {
                writer.writeFile(writeQRef->pop());
            }
            //std::cout << rank << ": Writer finished!" <<  std::endl;
            if (tid == 0) {
                end = omp_get_wtime();
                std::cout << rank << ": writing finished time - " << end-start << std::endl;
            }

        }
        else {
            // push the pairs to coresponding queue
            int index;
            std::hash<char> hash_ch;
            for (int i = 0; i < bufsize; i++) {
                MYPAIR p = rbuf[i];
                std::string word(p.word);
                int value = p.value;
                index = (hash_ch(word[0]) + word.length()) % NUM_REDUCERS;
                std::pair<std::string, int> tmp(word, value);
                reduceQ[index].push(tmp);
            }
            reduceFlag = false;
        }
    }
    end = omp_get_wtime();
    std::cout << rank << ": total time - " << end-start << std::endl;

    MPI_Finalize();
}

