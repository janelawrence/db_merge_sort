#ifndef RECORD_H
#define RECORD_H


class Record {
private:
    int size;
    char*  key;
    // std::string key;


public:
    // Constructor
    Record(int s, const char* k);

    // Destructor
    ~Record();

    // Wrtie random bytes to file
    // void writeToFile

    // Getters
    int getSize() const;
    const char* getKey() const;

    // Setters
    void setSize(int s);
    void setKey(const char* k);

    // Display method
    void display() const;
};

#endif // RECORD_H
