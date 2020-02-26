/*
 * Copyright 2017-2020
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *   Pascal Felber <pascal.felber@unine.ch>
 *
 * This work is published under the MIT license. See LICENSE.txt
 */
#ifndef _TM_LINKED_LIST_FAT_QUEUE_BY_REF_H_
#define _TM_LINKED_LIST_FAT_QUEUE_BY_REF_H_

#include <stdexcept>


/**
 * <h1> A Linked List queue with fat nodes using a PTM </h1>
 */
template<typename T, typename TM, template <typename> class TMTYPE>
class TMLinkedListFatQueueByRef : public TM::tmbase {

    static const uint64_t NUM_ITEMS = 16-3; // 16 is faster than 32 and faster than 8

private:
    struct Node : public TM::tmbase {
        TMTYPE<uint64_t> it {1};
        TMTYPE<uint64_t> ih {0};
        TMTYPE<T*>       item[NUM_ITEMS];
        TMTYPE<Node*>    next {nullptr};
        Node(T* userItem) {
            item[0] = userItem;
        }
    };

    TMTYPE<Node*>  head {nullptr};
    TMTYPE<Node*>  tail {nullptr};


public:
    TMLinkedListFatQueueByRef() {
        TM::template updateTx([&] () {
            Node* sentinelNode = TM::template tmNew<Node>(nullptr);
            sentinelNode->it = NUM_ITEMS;
            sentinelNode->ih = NUM_ITEMS;
            head = sentinelNode;
            tail = sentinelNode;
        });
    }


    ~TMLinkedListFatQueueByRef() {
        TM::template updateTx([&] () {
            while (dequeue() != nullptr); // Drain the queue
            Node* lhead = head;
            TM::tmDelete(lhead);
        });
    }


    static std::string className() { return TM::className() + "-LinkedListFatQueue"; }


    /*
     * Always returns true
     */
    bool enqueue(T* item) {
        TM::template updateTx([&] () {
            Node* ltail = tail;
            if (ltail->it == NUM_ITEMS) {
                Node* newNode = TM::template tmNew<Node>(item);
                tail->next = newNode;
                tail = newNode;
            } else {
                ltail->item[ltail->it] = item;
                ltail->it++;
            }
        });
        return true;
    }


    T* dequeue() {
        T* item = nullptr;
        TM::template updateTx([&] () {
            Node* lhead = head;
            if (lhead == tail) return;
            if (lhead->ih == NUM_ITEMS) {
                head = lhead->next;
                TM::tmDelete(lhead);
                lhead = head;
            }
            item = lhead->item[lhead->ih];
            lhead->ih++;
        });
        return item;
    }
};

#endif /* _TM_LINKED_LIST_FAT_QUEUE_BY_REF_H_ */
