/*
 * Copyright 2017-2020
 *   Andreia Correia <andreia.veiga@unine.ch>
 *   Pedro Ramalhete <pramalhe@gmail.com>
 *   Pascal Felber <pascal.felber@unine.ch>
 *
 * This work is published under the MIT license. See LICENSE.txt
 */
#ifndef _TM_LINKED_LIST_FAT_STACK_BY_REF_H_
#define _TM_LINKED_LIST_FAT_STACK_BY_REF_H_

#include <string>


/**
 * <h1> A Linked List stack (memory unbounded) with fat nodes for usage with STMs and PTMs </h1>
 *
 */
template<typename T, typename TM, template <typename> class TMTYPE>
class TMLinkedListFatStackByRef : public TM::tmbase {

    static const uint64_t NUM_ITEMS = 16-2; // 16 is faster than 32

private:
    struct Node : public TM::tmbase {
        TMTYPE<uint64_t> ih {1};
        TMTYPE<T*>       item[NUM_ITEMS];
        TMTYPE<Node*>    next {nullptr};
        Node(T* userItem) {
            item[0] = userItem;
        }
    };

    TMTYPE<Node*>  head {nullptr};


public:
    TMLinkedListFatStackByRef() { }


    ~TMLinkedListFatStackByRef() {
        TM::template updateTx([&] () {
            while (pop() != nullptr); // Drain the stack
        });
    }


    static std::string className() { return TM::className() + "-LinkedListFatStack"; }


    // Always returns true
    bool push(T* item) {
        TM::template updateTx([&] () {
            Node* lhead = head;
            if (lhead == nullptr || lhead->ih == NUM_ITEMS) {
                // Node is full or non-existant. Create a new node.
                Node* newNode = TM::template tmNew<Node>(item);
                newNode->next = lhead;
                head = newNode;
            } else {
                lhead->item[lhead->ih] = item;
                lhead->ih++;
            }
        });
        return true;
    }


    T* pop() {
        T* item = nullptr;
        TM::template updateTx([&] () {
            Node* lhead = head;
            if (lhead == nullptr) return;
            if (lhead->ih == 0) {
                head = lhead->next;
                TM::tmDelete(lhead);
            }
            lhead = head;
            if (lhead == nullptr) return;
            item = lhead->item[lhead->ih-1];
            lhead->ih--;
        });
        return item;
    }
};

#endif /* _TM_LINKED_LIST_FAT_STACK_BY_REF_H_ */
