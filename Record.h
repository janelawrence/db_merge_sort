#ifndef RECORD_H
#define RECORD_H
#include "defs.h"


extern int KEY_SIZE;

class Record {
    private:
        int size;
        int slot = -1;
        char* key;
        char* content;
        // Record* prev;
        // Record* next;


    public:
        // Constructor
        Record(int s, const char* bytes);
        
        // Copy constructor
        Record(const Record& other);


        // Serialize record
        char * serialize() const;
        static Record * deserialize(const char* serializedData, int dataSize);
        
        // Print record
        void printRecord();

        // Destructor
        ~Record();


        // Getters
        int getSize() const;
        int getSlot() const;
        const char* getKey() const;
        const char* getContent() const;
        // Record* getPrev() const;
        // Record* getNext() const;

    // Setter functions
        // void setPrev(Record* prevRecord);
        // void setNext(Record* nextRecord);

        void setSize(int s);
        void setSlot(int s);
        void setKey(const char* k);
        void setContent(const char* data);

    };

#endif // RECORD_H
