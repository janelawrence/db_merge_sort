#include "Record.h"
#include "defs.h"
// #include <string>
#include <random>


int KEY_SIZE = 8;

// Constructor
Record::Record(int s, const char* bytes) : size(s) {
    key = new char[KEY_SIZE + 1];
    std::strncpy(key, bytes, KEY_SIZE);
    key[KEY_SIZE] = '\0';

    content = new char[size - KEY_SIZE + 1];
    // Copy the rest of the bytes to content
    std::strncpy(content, bytes + KEY_SIZE, size - KEY_SIZE);
    content[size - KEY_SIZE] = '\0'; // Ensure null termination

}

// Copy constructor definition
Record::Record(const Record& other) : size(other.size), slot(other.slot) {
    key = new char[strlen(other.key) + 1];
    content = new char[strlen(other.content) + 1];
    strcpy(key, other.key);
    strcpy(content, other.content);
}


// Destructor
Record::~Record() {
    delete[] key; // Deallocate memory for key
    delete[] content;
}


char * Record::serialize() const {
    int contentSize = strlen(content);
    char * serializedData = new char[size];
    memcpy(serializedData, key, strlen(key));
    // Copy 'content' into the serialized data after 'key'
    memcpy(serializedData + KEY_SIZE, content, contentSize);

    return serializedData;
}

Record* Record::deserialize(const char* serializedData, int dataSize) {
    Record* record = new Record(dataSize, serializedData);
    record->setSlot(-1);
    return record;
}

void Record::printRecord(){
    printf("Record: key %s, slot: %d, size: %d, content size: %d\n", key, slot, size, strlen(content));
};

// Getters
int Record::getSize() const { return size; }

int Record::getSlot() const { return slot; }

const char* Record::getKey() const { return key; }

const char* Record::getContent() const {return content; }

// Setters

void Record::setSize(int s) { size = s; }

void Record::setSlot(int s) { slot = s; }

void Record::setKey(const char* k) {
    delete[] key; // Deallocate memory for the current key
    key = new char[strlen(k) + 1]; // Allocate memory for the new key
    strcpy(key, k); // Copy the new key into key
}

void Record::setContent(const char* data) {
    delete[] content; // Deallocate memory for the current data
    content = new char[strlen(data) + 1]; // Allocate memory for the new content
    strcpy(content, data); // Copy the new content into content
}




// int main() {

//     Record * r1 = new Record(20, "");
//     Record * r2 = new Record(20, "");
//     Record * r3 = new Record(20, "");
//     Record * r4 = new Record(20, "");
//     Record * r5 = new Record(20, "");

//     r1->setSlot(1);
//     r2->setSlot(2);
//     r3->setSlot(3);
//     r4->setSlot(4);
//     r5->setSlot(5);

//     r1->printRecord();
//     r2->printRecord();
//     r3->printRecord();
//     r4->printRecord();
//     r5->printRecord();
// }