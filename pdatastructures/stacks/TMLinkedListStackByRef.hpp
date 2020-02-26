#ifndef _TM_LINKED_LIST_STACK_BY_REF_H_
#define _TM_LINKED_LIST_STACK_BY_REF_H_

#include <string>


/**
 * <h1> A Linked List stack (memory unbounded) for usage with STMs and PTMs </h1>
 *
 */
template<typename T, typename TM, template <typename> class TMTYPE>
class TMLinkedListStackByRef : public TM::tmbase {

private:
    struct Node : public TM::tmbase {
        TMTYPE<T*>    item;
        TMTYPE<Node*> next {nullptr};
        Node(T* userItem) : item{userItem} { }
    };

    TMTYPE<Node*>  head {nullptr};


public:
    TMLinkedListStackByRef() {
        TM::template updateTx([&] () {
		    head = TM::template tmNew<Node>(nullptr);
        });
    }


    ~TMLinkedListStackByRef() {
        TM::template updateTx([&] () {
            while (pop() != nullptr); // Drain the stack
        });
    }


    static std::string className() { return TM::className() + "-LinkedListStack"; }


    // Always returns true
    bool push(T* item) {
        TM::template updateTx([&] () {
            Node* newNode = TM::template tmNew<Node>(item);
            newNode->next = head;
            head = newNode;
        });
        return true;
    }


    T* pop() {
        T* retval = nullptr;
        TM::template updateTx([&] () {
            Node* lhead = head;
            if (lhead == nullptr) return;
            retval = lhead->item;
            head = lhead->next;
            TM::tmDelete(lhead);
        });
        return retval;
    }
};

#endif /* _TM_LINKED_LIST_STACK_BY_REF_H_ */
