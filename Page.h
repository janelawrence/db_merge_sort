#ifndef PAGE_H
#define PAGE_H
#include "Record.h"
// #include <memory>
#include <list>
#include <string>

const std::string FROM_SSD = "FROM_SSD";
const std::string FROM_HDD = "FROM_HDD";

class Page
{
private:
    std::list<Record *> records;
    int MAX_RECORDS;
    int size;
    int bytes;
    int idx;

    std::string pageSource;

public:
    Page *prev;
    Page *next;

    Page(int i, int s, int pageSize);
    ~Page();

    void addRecord(Record *);
    void removeFisrtRecord();
    void clear();
    void print(bool listRecord = true) const;
    Page *clone();

    // Getters
    Record *getFirstRecord();
    std::list<Record *> getRecords() const;
    int getNumRecords() const;
    bool isEmpty() const;
    bool isFull() const;
    Page *getPrev() const;
    Page *getNext() const;
    int getIdx() const;
    int getBytes() const;
    int getSize() const;
    int getMaxRecords() const;
    std::string getSource() const;

    // Setter functions
    void setPrev(Page *prevPage);
    void setNext(Page *nextPage);
    void setIdx(int newIdx);
    void setBytes(int newBytes);
    void setMaxRecords(int maxRec);
    void setSource(const std::string &newSource);
};

#endif // PAGE_H
