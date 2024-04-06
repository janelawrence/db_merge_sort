/*
    Generate records with random keys 
*/

#ifndef GENERATOR_H
#define GENERATOR_H

class Generator {
    private:
        int recordSize;
        int numRecords;

    public:

        // Constructor
        Generator(int recordSize, int numRecords);

        void generateRecords(const char* fileName);
    };


#endif // GENERATOR_H