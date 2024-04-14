#include "Run.h"
#include "defs.h"

Run::Run()
{
    pageHead = new Page(-1, 0, 0);
    pageTail = pageHead;
}

/**
 * Append page in the end,
 * page can have next page, so need to adjust pageTail
 * and numPage
 *
 */
void Run::appendPage(Page *page)
{
    if (pageHead->getIdx() == -1)
    {
        pageHead = page;
        // page->print();
        // pageHead->print();
    }
    else
    {
        pageTail->setNext(page);
        page->setPrev(pageTail);
    }
    Page *temp = page;
    while (temp)
    {
        pageTail = temp;
        temp = temp->getNext();
        numPage++;
        bytes += pageTail->getBytes();
    }
}

Run *Run::clone()
{
    Run *clonedRun = new Run();
    Page *current = pageHead;
    while (current)
    {
        clonedRun->appendPage(current->clone());
        current = current->getNext();
    }
    return clonedRun;
}

void Run::removeFisrtPage()
{
    if (numPage == 0 || pageHead == nullptr)
    {
        printf("Can't remove, run is empty");
        return;
    }
    numPage--;
    bytes -= pageHead->getBytes();
    pageHead = pageHead->getNext();
}

void Run::addRecord(Record *record)
{
    if (pageTail->isFull() || pageTail->getSize() - pageTail->getBytes() < record->getSize())
    {
        Page *newPage = new Page(pageTail->getIdx() + 1, PAGE_SIZE / recordSize, PAGE_SIZE);
        newPage->addRecord(record);
        appendPage(newPage);
    }
    else
    {
        pageTail->addRecord(record);
        bytes += recordSize;
    }
}

void Run::clear()
{
    // std::list<Page*> newpages;
    // pages.swap(newpages);
    numPage = 0;
    bytes = 0;
    pageHead = new Page(-1, 0, 0);
    pageTail = pageHead;
}

void Run::print(bool listPage) const
{
    printf("Run: has %lu pages, total bytes %llu \n", numPage, bytes);
    int i = 0;
    if (listPage)
    {
        Page *current = pageHead;

        if (pageHead == nullptr || pageHead->getIdx() == -1)
        {
            return;
        }
        while (current)
        {
            current->setIdx(i++);
            current->print();
            current = current->getNext();
        }
    }
}

/// <summary>
/// Return a shallow copy of the first Record in Run
/// </summary>
/// <returns>Return a shallow copy of the first Record in Run</returns>
Page *Run::getFirstPage()
{
    Page *firstPage;
    if (numPage == 0 || pageHead == nullptr)
    {
        printf("List is empty, can't get fist from the list");
        return firstPage;
    }
    return pageHead;
}

Page *Run::getLastPage()
{
    Page *lastPage;
    if (numPage == 0 || pageHead == nullptr)
    {
        printf("List is empty, can't get fist from the list");
        return lastPage;
    }
    return pageTail;
}

int Run::getNumPages() const { return numPage; }

unsigned long long Run::getBytes() const { return bytes; }

bool Run::isEmpty() const { return numPage == 0 || pageHead == nullptr; }

/**
 * Set a new list of pages
 * to be the pages stored in Run
 */
// void Run::setPageHead(Page * page) {
//     pageHead = page;
//     Page * temp = page;
//     //  Find new pageTail
//     while(temp) {
//         pageTail = temp;
//         temp = temp->getNext();
//     }
// }
