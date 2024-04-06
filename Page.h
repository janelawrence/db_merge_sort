#ifndef PAGE_H
#define PAGE_H
#include "Record.h"
#include <list>


class Page {
    private:
        std::list<Record*> records; // linkedlist
        Page* prev;
        Page* next;
    public:
        Page();

        void add(Record *);
        void removeFisrt();
        void clear();
        void print(bool listRecord = true) const;
        Page* clone();

        // Getters
        Record* getFirst();
        std::list<Record*> getRecords() const;
        int getSize() const;
        bool isEmpty() const;
        Page* getPrev() const;
        Page* getNext() const;

        // Setter functions
        void setPrev(Page* prevPage);
        void setNext(Page* nextPage);

};


#endif //PAGE_H

