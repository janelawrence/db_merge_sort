#include "Record.h"
#include "defs.h"
// #include <string>
#include <random>

// Constructor
Record::Record(int s, const char* k) : size(s) {
    if (k[0] == '\0') {
        // Generate random key if the provided key is empty
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis('a', 'z');

        key = new char[KEY_SIZE + 1];

        // Generating a random string of length 10
        for (int i = 0; i < KEY_SIZE; ++i) {
            key[i] = static_cast<char>(dis(gen));
        }
    } else {
        // Use the provided key
        key = new char[strlen(k) + 1]; // Allocate memory for the provided key
        strcpy(key, k);
    }
    int contentsize = size - 2 * sizeof(int) - (strlen(key) + 1);
    printf("contentsize: %d\n", contentsize);
    generateRandomBytes();

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
}

void Record::generateRandomBytes() {
    int contentSize = size - sizeof(int) - (strlen(key) + 1);
    
    // Generate random data
    char* buffer = new char[contentSize];

    for (int i = 0; i < contentSize - 1; ++i) {
        buffer[i] = rand() % 256; // Random byte value
    }

    setContent(buffer);
}

char * Record::serialize() const {
    int contentSize = size - 2 * sizeof(int) - (strlen(key) + 1);
    char * serializedData = new char[size];
    memcpy(serializedData, &size, sizeof(int));
    memcpy(serializedData, &slot, sizeof(int));
    memcpy(serializedData + sizeof(int), key, strlen(key) + 1);
    // Copy 'content' into the serialized data after 'key'
    memcpy(serializedData + sizeof(int) + strlen(key) + 1, content, contentSize);

    return serializedData;
}

Record* Record::deserialize(const char* serializedData, int dataSize) {
    // first sizeof(int) bytes is size
    // second sizeof(int) bytes is slot index
    int contentSize = dataSize - 2 * sizeof(int) - (KEY_SIZE + 1);
    int s;
    int slotIdx;
    char* k = new char[(KEY_SIZE + 1)];
    printf("size: %d, ContentSize: %d\n", dataSize , contentSize);
    char* cont = new char[contentSize];
    memcpy(&s, serializedData, sizeof(int));
    memcpy(&slotIdx, serializedData, sizeof(int));
    memcpy(k, serializedData + sizeof(int), (KEY_SIZE + 1));
    memcpy(cont, serializedData + sizeof(int) + (KEY_SIZE + 1), contentSize);
    Record* record = new Record(dataSize, k);
    record->setContent(cont);
    record->setSlot(slotIdx);
    return record;
}

void Record::printRecord(){
    printf("Record: key %s, slot: %d, size: %d\n", key, slot, size);
};

// Getters
int Record::getSize() const {
    return size;
}

int Record::getSlot() const {
    return slot;
}

const char* Record::getKey() const {
    return key;
}

const char* Record::getContent() const {
    return content;
}




// Setters
void Record::setSize(int s) {
    size = s;
}

void Record::setSlot(int s) {
    slot = s;
}

void Record::setKey(const char* k) {
    delete[] key; // Deallocate memory for the current key
    key = new char[strlen(k) + 1]; // Allocate memory for the new key
    strcpy(key, k); // Copy the new key into key
}

void Record::setContent(const char* data) {
    if(data != nullptr) {
        delete[] content; // Deallocate memory for the current data
        content = new char[strlen(data) + 1]; // Allocate memory for the new content
        strcpy(content, data); // Copy the new content into content
    }
}

// Display method
void Record::display() const {
    // std::cout << "Key: " << key << ", Size: " << size << std::endl;
	printf("key: %s, Size: %d\n", key, size);
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