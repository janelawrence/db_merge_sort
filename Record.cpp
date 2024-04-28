#include "Record.h"
#include "defs.h"
// #include <string>
#include <random>
#include <cstring>

int KEY_SIZE = 8;

// Constructor
Record::Record(int s,
            const std::string &bytes) : 
            size(s),
            key(bytes.substr(0, KEY_SIZE)),
            content(bytes.substr(KEY_SIZE)){}

// Copy constructor definition
Record::Record(const Record &other) : 
            size(other.size), 
            slot(other.slot), 
            key(other.key), 
            content(other.content){}

// Destructor
Record::~Record()
{

}


void Record::printRecord()
{
    printf("Record: key %s, slot: %d, size: %d, content size: %d\n",
           key.data(), slot, size, content.size());
}
// Getters
int Record::getSize() const { return size; }

int Record::getSlot() const { return slot; }

const char *Record::getKey() const { return key.data(); }

const char *Record::getContent() const { return content.data(); }

// Setters

void Record::setSize(int s) { size = s; }

void Record::setSlot(int s) { slot = s; }
