#ifndef _PERSISTENT_SEQUENTIAL_LINKED_LIST_QUEUE_H_
#define _PERSISTENT_SEQUENTIAL_LINKED_LIST_QUEUE_H_

#include <string>


/**
 * <h1> A Linked List queue (memory unbounded) for usage with STMs and PTMs </h1>
 *
 */
template<typename T, typename PTM, template <typename> class TMTYPE>
class PSeqLinkedListQueue {

private:
    struct Node {
        TMTYPE<T>    item;
        TMTYPE<Node*> next {nullptr};
        Node(T userItem) : item{userItem} { }
    };

    TMTYPE<Node*>  head {nullptr};
    TMTYPE<Node*>  tail {nullptr};


public:
    T EMPTY {};

    PSeqLinkedListQueue() {
		Node* sentinelNode = PTM::template tmNew<Node>(EMPTY);
		head = sentinelNode;
		tail = sentinelNode;
    }


    ~PSeqLinkedListQueue() {
		while (dequeue() != EMPTY); // Drain the queue
		Node* lhead = head;
		PTM::tmDelete(lhead);
    }


    static std::string className() { return PTM::className() + "-LinkedListQueue"; }


    bool enqueue(T item, const int tid=0) {
        PTM::updateTxSeq([&] () {
            Node* newNode = PTM::template tmNew<Node>(item);
            tail->next = newNode;
            tail = newNode;
        });
        return true;
    }


    T dequeue(const int tid=0) {
        T item = EMPTY;
        PTM::updateTxSeq([&] () {
            Node* lhead = head;
            if (lhead == tail) return;
            head = lhead->next;
            PTM::tmDelete(lhead);
            item = head->item;
        });
        return item;
    }
};

#endif /* _PERSISTENT_SEQUENTIAL_LINKED_LIST_QUEUE_H_ */
