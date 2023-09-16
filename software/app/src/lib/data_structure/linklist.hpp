#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// #define NOINLINE __NO_INLINE__
#define NOINLINE

#define LINKLIST_DEBUG
#define LINKLIST_TEST

template <typename T>
class LinkList;

template <typename T>
class LinkNode {
    friend class LinkList<T>;

private:
    LinkNode<T>* pNext;
    LinkNode<T>* pPrev;

public:
    T data;

    LinkNode(T data, LinkNode<T>* prev = nullptr, LinkNode<T>* next = nullptr)
    {
        this->pNext = next;
        this->pPrev = prev;
        this->data  = data;
    }

    NOINLINE void Log(uint8_t mode = 1)
    {
        if (mode & 0x01) {
            printf("%d", data);
        }

        if (mode & 0x02) {
            printf("  %p,%p", pPrev, pNext);
        }

        if (mode) {
            printf("\n");
        }
    }
};

template <typename T>
class LinkList {
private:
    LinkNode<T>* pFront;
    LinkNode<T>* pBack;

public:
    LinkList(void) : pFront(nullptr), pBack(nullptr) {}

    ~LinkList()
    {
        LinkNode<T>*pNode = pFront, *t;
        while (pNode) {
            t = pNode->pNext;
            delete pNode;
            pNode = t;
        }
    }

    bool is_empty(void) { return nullptr == pFront || nullptr == pBack; }

    LinkNode<T>* front(void) { return pFront; }

    LinkNode<T>* back(void) { return pBack; }

    LinkNode<T>* push_front(T data)
    {
        LinkNode<T>* pNode = new LinkNode<T>(data, nullptr, pFront);
        return pFront = (pFront == nullptr ? pBack : pFront->pPrev) = pNode;
    }

    LinkNode<T>* push_back(T data)
    {
        LinkNode<T>* pNode = new LinkNode<T>(data, pBack, nullptr);
        return pBack = (pBack == nullptr ? pFront : pBack->pNext) = pNode;
    }

    void pop_front(void)
    {
        if (nullptr == pFront) return;
        LinkNode<T>* pNode = pFront->pNext;
        delete pFront;
        (pFront = pNode) ? (pNode->pPrev = nullptr) : (pBack = nullptr);
    }

    void pop_back(void)
    {
        if (nullptr == pBack) return;
        LinkNode<T>* pNode = pBack->pPrev;
        delete pBack;
        (pBack = pNode) ? (pNode->pNext = nullptr) : (pFront = nullptr);
    }

    NOINLINE LinkNode<T>* insert_before(LinkNode<T>* pNodeRef, T data)
    {
        if (nullptr == pNodeRef) return nullptr;

        LinkNode<T>* pNode = new LinkNode<T>(data, pNodeRef->pPrev, pNodeRef);

        if (pFront == pNodeRef) pFront = pNode;
        if (pNodeRef->pPrev) pNodeRef->pPrev->pNext = pNode;

        return pNodeRef->pPrev = pNode;
    }

    NOINLINE LinkNode<T>* insert_after(LinkNode<T>* pNodeRef, T data)
    {
        if (nullptr == pNodeRef) return nullptr;

        LinkNode<T>* pNode = new LinkNode<T>(data, pNodeRef, pNodeRef->pNext);

        if (pBack == pNodeRef) pBack = pNode;
        if (pNodeRef->pNext) pNodeRef->pNext->pPrev = pNode;

        return pNodeRef->pNext = pNode;
    }

    NOINLINE void remove(LinkNode<T>* pNode)
    {
        if (nullptr == pNode) return;
        if (pNode->pPrev) pNode->pPrev->pNext = pNode->pNext;
        if (pNode->pNext) pNode->pNext->pPrev = pNode->pPrev;
        if (pFront == pNode) pFront = pNode->pNext;
        if (pBack == pNode) pBack = pNode->pPrev;
        delete pNode;
    }

    NOINLINE int length(void)
    {
        int length = 0;

        LinkNode<T>* pNode = pFront;
        if (pNode) {
            do {
                ++length;
            } while (pNode = pNode->pNext);
        }

        return length;
    }

#ifdef LINKLIST_DEBUG

    void log_elems(void)
    {
        if (nullptr == pFront) return;
        LinkNode<T>* pNode = pFront;
        do {
            printf("%d,", pNode->data);
        } while (pNode = pNode->pNext);
        printf("\n");
    }

#endif
};

#if 0

int main()
{
    LinkList<int> list;

    list.push_back(1);
    list.push_back(2);
    list.Front()->Log(3);
    list.Back()->Log(3);

    // 1,2

    list.push_back(3);
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);

    // 3,2,1,1,2,3

    list.log_elems();

    list.pop_front();
    list.pop_front();
    list.pop_back();
    list.pop_back();
    list.pop_front();

    // 1

    list.log_elems();

    list.Front()->Log(1);
    list.Back()->Log(1);

    list.pop_back();

    printf("empty %d", list.is_empty());

    return 0;
}

#endif
