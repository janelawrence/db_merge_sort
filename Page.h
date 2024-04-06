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
        int idx;
        int size;
    public:
        Page(int i, int s);

        void addRecord(Record *);
        void removeFisrtRecord();
        void clear();
        void print(bool listRecord = true) const;
        Page* clone();

        // Getters
        Record* getFirst();
        std::list<Record*> getRecords() const;
        int getNumRecords() const;
        bool isEmpty() const;
        Page* getPrev() const;
        Page* getNext() const;

        // Setter functions
        void setPrev(Page* prevPage);
        void setNext(Page* nextPage);

};


#endif //PAGE_H

