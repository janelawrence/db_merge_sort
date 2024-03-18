#include "Record.h"
#include <iostream>
#include <string>
#include <random>

// Constructor
Record::Record(int s, const std::string& k) : size(s) {
        if (k.empty()) {
        // Generate random key if the provided key is empty
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis('a', 'z');

        // Generating a random string of length 10
        for (int i = 0; i < 10; ++i) {
            key += static_cast<char>(dis(gen));
        }
    } else {
        // Use the provided key
        key = k;
    }
}

// Getters
int Record::getSize() const {
        return size;
}


std::string Record::getKey() const {
    return key;
}

// Setters
void Record::setSize(int s) {
    size = s;
}

void Record::setKey(const std::string& k) {
    key = k;
}

// Display method
void Record::display() const {
    std::cout << "Key: " << key << ", Size: " << size << std::endl;
}

int main() {
    // Creating a Record object
    Record record(10, "");

    // Displaying the initial record attributes
    std::cout << "Initial Record:" << std::endl;
    record.display();

    // Modifying record attributes
    record.setSize(20);
    record.setKey("sdf");

    // Displaying the modified record attributes
    std::cout << "\nModified Record:" << std::endl;
    record.display();

    return 0;
}