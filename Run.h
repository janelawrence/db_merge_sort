#ifndef RUN_H
#define RUN_H
#include "Record.h"
#include <list>


class Run {
    private:
        std::list<Record*> records;
    public:
        Run();

        void add(Record *);
        void removeFisrt();
        void clear();
        void print(bool listRecord = true) const;
        Run* clone();
        // Getters
        std::list<Record*> getRecords() const;
        int getSize() const;
        bool isEmpty() const;

};


#endif //RUN_H

