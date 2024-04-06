#ifndef RUN_H
#define RUN_H
#include "Page.h"
#include <list>


class Run {
    private:
        Page * pageHead;
        Page * pageTail;
        int numPage;
    public:
        Run();

        void appendPage(Page *);
        void removeFisrtPage();
        void clear();
        void print(bool listPage = true) const;
        Run* clone();
        // Getters
        Page* getFirstPage();
        Page* getLastPage();
        int getNumPages() const;
        bool isEmpty() const;

};


#endif //RUN_H

