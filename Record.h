#ifndef RECORD_H
#define RECORD_H
#include "defs.h"

extern int KEY_SIZE;

class Record
{
private:
    // int size;
    // int slot = -1;
    // std::string key;
    // std::string content;
    // Record* prev;
    // Record* next;

public:
    int size;
    int slot = -1;
    std::string key;
    std::string content;

    // Constructor
    Record(int s, const std::string &bytes);

    // Copy constructor
    Record(const Record &other);

    // Serialize record
    // char *serialize() const;
    // static Record *deserialize(const char *serializedData, int dataSize);

    // Print record
    void printRecord();

    // Destructor
    ~Record();

    // Getters
    int getSize() const;
    int getSlot() const;
    const char *getKey() const;
    // std::string getKey() const;

    const char *getContent() const;
    // Record* getPrev() const;
    // Record* getNext() const;

    // Setter functions
    // void setPrev(Record* prevRecord);
    // void setNext(Record* nextRecord);

    void setSize(int s);
    void setSlot(int s);
    // void setKey(const char *k);
    // void setContent(const char *data);
};

#endif // RECORD_H
