#ifndef PAGE_H
#define PAGE_H
#include "Record.h"
#include <list>


class Page {
    private:
        std::list<Record*> records; // linkedlist
        Page* prev;
        Page* next;
        // Record* recordHead;
        // Record* record;
        int MAX_RECORDS;
        int size;
        int bytes;
        int idx;
    public:
        Page(int i, int s, int pageSize);

        void addRecord(Record *);
        void removeFisrtRecord();
        void clear();
        void print(bool listRecord = true) const;
        Page* clone();

        // Getters
        Record* getFirstRecord();
        std::list<Record*> getRecords() const;
        int getNumRecords() const;
        bool isEmpty() const;
        bool isFull() const;
        Page* getPrev() const;
        Page* getNext() const;
        int getIdx() const;
        int getBytes() const;


        // Setter functions
        void setPrev(Page* prevPage);
        void setNext(Page* nextPage);
        void setIdx(int newIdx);
        void setBytes(int newBytes);


};


#endif //PAGE_H

