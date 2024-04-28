#include "Run.h"
#include "defs.h"

Run::Run(int pSizeInRun): 
    PAGE_SIZE_IN_RUN(pSizeInRun)
{
    pageHead = new Page(-1, 0, 0);
    pageTail = pageHead;
    bytes = 0;
    numPage = 0;
}

Run::~Run()
{
    Page *temp = pageHead;
    while (temp != nullptr)
    {
        Page *temptemp = temp;
        temp = temp->getNext();
        delete temptemp;
    }
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
        Page *ptr = pageHead;
        delete ptr;
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
        if (temp == nullptr)
        {
            break;
        }
        temp = temp->getNext();
        numPage++;
        bytes += pageTail->getBytes();
    }
}

Run *Run::clone()
{
    Run *clonedRun = new Run(DRAM_PAGE_SIZE);
    Page *current = pageHead;
    while (current)
    {
        clonedRun->appendPage(current->clone());
        current = current->getNext();
    }
    return clonedRun;
}

void Run::removeFirstPage(int firstPageOriginalBytes)
{
    if (numPage == 0 || pageHead == nullptr)
    {
        printf("Can't remove, run is empty");
        return;
    }
    numPage--;
    Page *temp = pageHead;
    if (firstPageOriginalBytes > 0 && pageHead->getBytes() == 0)
    {
        bytes -= firstPageOriginalBytes; // HANDLE CORNER CASE
    }
    else
    {
        bytes -= pageHead->getBytes();
    }
    pageHead = pageHead->getNext();
    // delete temp;
}

/*Add a record to the last page, add a page if needed */
// Only used when Page size in run is DRAM size
void Run::addRecord(Record *record)
{
    if (pageHead->getIdx() == -1)
    {
        Page *temp = pageHead;
        pageHead = new Page(0, DRAM_PAGE_SIZE / recordSize, DRAM_PAGE_SIZE);
        pageTail = pageHead;
        delete temp;
        numPage++;
    }
    if (pageTail->isFull() || pageTail->getSize() - pageTail->getBytes() < record->getSize())
    {
        Page *newPage = new Page(pageTail->getIdx() + 1, DRAM_PAGE_SIZE / recordSize, DRAM_PAGE_SIZE);
        newPage->addRecord(record);
        appendPage(newPage);
    }
    else
    {
        pageTail->addRecord(record);
        bytes += recordSize;
    }
}

Record *Run::popFirstRecord()
{
    if (!isEmpty() && !pageHead->isEmpty())
    {
        Record *record = pageHead->getFirstRecord();
        pageHead->removeFisrtRecord();
        bytes -= recordSize;
        //  if this is the last record of the page
        if (pageHead->isEmpty())
        {
            numPage--;
            pageHead = pageHead->getNext();
        }
        return record;
    }
    return nullptr;
}

void Run::clear()
{
    // std::list<Page*> newpages;
    // pages.swap(newpages);
    numPage = 0;
    bytes = 0;
    while (pageHead != nullptr)
    {
        Page *temp = pageHead;
        pageHead = pageHead->getNext(); // Assuming 'next' is a pointer to the next Page
        delete temp;
    }
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
