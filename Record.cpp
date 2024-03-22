#include "Record.h"
// #include <string>
#include <random>

// Constructor
Record::Record(int s, const char* k) : size(s) {
        if (k[0] == '\0') {
	        printf("\nStart generating random key\n");

        // Generate random key if the provided key is empty
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis('a', 'z');

        key = new char[11];

        // Generating a random string of length 10
        for (int i = 0; i < 10; ++i) {
            key[i] = static_cast<char>(dis(gen));
        }
    } else {
        // Use the provided key
        key = new char[strlen(k) + 1]; // Allocate memory for the provided key
        strcpy(key, k);
    }
}

// Destructor
Record::~Record() {
    delete[] key; // Deallocate memory for key
}

// Getters
int Record::getSize() const {
        return size;
}


const char* Record::getKey() const {
    return key;
}

// Setters
void Record::setSize(int s) {
    size = s;
}

void Record::setKey(const char* k) {
    delete[] key; // Deallocate memory for the current key
    key = new char[strlen(k) + 1]; // Allocate memory for the new key
    strcpy(key, k); // Copy the new key into key
}

// Display method
void Record::display() const {
    // std::cout << "Key: " << key << ", Size: " << size << std::endl;
	printf("key: %s, Size: %d\n", key, size);

    
}

// int main() {
//     // Creating a Record object
//     Record record(10, "");

//     // Displaying the initial record attributes
//     // std::cout << "Initial Record:" << std::endl;
// 	printf("Initial Record:\n");
//     record.display();

//     // Modifying record attributes
//     record.setSize(20);
//     record.setKey("sdf");

//     // Displaying the modified record attributes
//     // std::cout << "\nModified Record:" << std::endl;
// 	printf("\nModified Record:\n");
//     record.display();

//     return 0;
// }
