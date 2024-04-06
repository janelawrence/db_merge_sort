#include "Run.h"
#include "defs.h"





Run::Run(){
    pageHead = new Page(-1, 0);
    pageTail = pageHead;
}



void Run::appendPage(Page * page) {
    pageTail->setNext(page);
    page->setPrev(pageTail);
    pageTail = page;
    numPage++;
}

Run* Run::clone(){
    Run* clonedRun = new Run();
    Page * current = pageHead;
    while(current) {
        clonedRun->appendPage(current->clone());
        current = current->getNext();
    }

    return clonedRun;
}

void Run::removeFisrtPage() {
    if(numPage == 0 || pageHead == nullptr) {
        printf("Can't remove, run is empty");
        return;
    }
    numPage--;
    pageHead = pageHead->getNext();
}


void Run::clear(){
    // std::list<Page*> newpages;
    // pages.swap(newpages);
    numPage = 0;
    pageHead = nullptr;
    pageTail = nullptr;
}

void Run::print(bool listPage) const {
    printf("Run: has %lu pages \n", numPage);
    if(listPage) {
        Page * current = pageHead->getNext();
        while(current) {
            current->print();
            current = current->getNext();
        }
    }
}

/**Return a shallow copy of the first Record in Run*/
Page* Run::getFirstPage() {
    Page* firstPage;
    if(numPage == 0 || pageHead == nullptr) {
        printf("List is empty, can't get fist from the list");
        return firstPage;
    }
    return pageHead;
}

Page* Run::getLastPage() {
    Page* lastPage;
    if(numPage == 0 || pageHead == nullptr) {
        printf("List is empty, can't get fist from the list");
        return lastPage;
    }
    return pageTail;
}

int Run::getNumPages() const {
    return numPage;
}

bool Run::isEmpty() const {
    return numPage == 0 || pageHead == nullptr;
}

