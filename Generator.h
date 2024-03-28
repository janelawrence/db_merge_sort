#ifndef GENERATOR_H
#define GENERATOR_H

class Generator {

public:
    // Constructor
    Generator(int recordSize, int numRecords);

    static void generateRecords();
};


#endif // GENERATOR_H