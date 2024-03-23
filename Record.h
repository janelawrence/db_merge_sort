#ifndef RECORD_H
#define RECORD_H


class Record {
private:
    int size;
    char* key;
    char* content;
    // std::string key;


public:
    // Constructor
    Record(int s, const char* k);

    // Wrtie random bytes to file
    void generateRandomBytes();

    // Serialize record
    char * serialize() const;
    static Record * deserialize(const char* serializedData, int dataSize);


    // Destructor
    ~Record();


    // Getters
    int getSize() const;
    const char* getKey() const;
    const char* getContent() const;


    // Setters
    void setSize(int s);
    void setKey(const char* k);
    void setContent(const char* data);


    // Display method
    void display() const;
};

#endif // RECORD_H
