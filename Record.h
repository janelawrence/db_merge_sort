#ifndef RECORD_H
#define RECORD_H

#include <string>

class Record {
private:
    int size;
    std::string key;

public:
    // Constructor
    Record(int s, const std::string& k);

    // Getters
    int getSize() const;
    std::string getKey() const;

    // Setters
    void setSize(int s);
    void setKey(const std::string& k);

    // Display method
    void display() const;
};

#endif // RECORD_H
