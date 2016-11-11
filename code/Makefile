CXX = mpic++
CPPFLAGS = -std=c++11
OMPFLAGS = -fopenmp

WordCount: main.o Mapper.o Reducer.o Reader.o Writer.o
	$(CXX) $(CPPFLAGS) $(OMPFLAGS) -o WordCount main.o Mapper.o Reducer.o Reader.o Writer.o
main.o: main.cpp Mapper.h Reducer.h Reader.h Writer.h Mypair.h
	$(CXX) $(CPPFLAGS) $(OMPFLAGS) -c main.cpp
Mapper.o: Mapper.h Mapper.cpp Queue.h Mypair.h
	$(CXX) $(CPPFLAGS) -c Mapper.cpp 
Reducer.o: Reducer.h Reducer.cpp 
	$(CXX) $(CPPFLAGS) -c Reducer.cpp
Reader.o: Reader.h Reader.cpp Queue.h 
	$(CXX) $(CPPFLAGS) -c Reader.cpp
Writer.o: Writer.h Writer.cpp 
	$(CXX) $(CPPFLAGS) -c Writer.cpp

clean:
	rm *.o *.h.gch
