#ifndef _PERSISTENT_SEQUENTIAL_LINKED_LIST_SETBYREF_H_
#define _PERSISTENT_SEQUENTIAL_LINKED_LIST_SETBYREF_H_

#include <string>


/**
 * <h1> A Linked List Set meant to be used with PTMs </h1>
 */
template<typename K, typename PTM, template <typename> class TMTYPE>
class PSeqLinkedListSet {

private:
    struct Node {
        TMTYPE<K>     key;
        TMTYPE<Node*> next {nullptr};
        Node(const K& key) : key{key} { }
        Node(){ }
    };

    TMTYPE<Node*>  head {nullptr};
    TMTYPE<Node*>  tail {nullptr};


public:
    PSeqLinkedListSet() {
        PTM::updateTxSeq([&] () {
            Node* lhead = PTM::template tmNew<Node>();
            Node* ltail = PTM::template tmNew<Node>();
            head = lhead;
            head->next = ltail;
            tail = ltail;
        });
    }

    ~PSeqLinkedListSet() {
        PTM::updateTxSeq([&] () {
            // Delete all the nodes in the list
            Node* prev = head;
            Node* node = prev->next;
            while (node != tail) {
                PTM::tmDelete(prev);
                prev = node;
                node = node->next;
            }
            PTM::tmDelete(prev);
            PTM::tmDelete(tail.pload());
        });
    }

    static std::string className() { return PTM::className() + "-LinkedListSet"; }

    /*
     * Adds a node with a key, returns false if the key is already in the set
     */
    bool add(K key) {
        bool retval = false;
        PTM::updateTxSeq([&] () {
            Node *prev, *node;
            find(key, prev, node);
            retval = !(node != tail && key == node->key);
            if (!retval) return;
            Node* newNode = PTM::template tmNew<Node>(key);
            prev->next = newNode;
            newNode->next = node;
        });
        return retval;
    }

    /*
     * Removes a node with an key, returns false if the key is not in the set
     */
    bool remove(K key) {
        bool retval = false;
        PTM::updateTxSeq([&] () {
            Node *prev, *node;
            find(key, prev, node);
            retval = (node != tail && key == node->key);
            if (!retval) return;
            prev->next = node->next;
            PTM::tmDelete(node);
        });
        return retval;
    }

    /*
     * Returns true if it finds a node with a matching key
     */
    bool contains(K key) {
        bool retval = false;
        PTM::readTxSeq([&] () {
            Node *prev, *node;
            find(key, prev, node);
            retval = (node != tail && key == node->key);
        });
        return retval;
    }

    void find(const K& lkey, Node*& prev, Node*& node) {
        Node* ltail = tail;
        for (prev = head; (node = prev->next) != ltail; prev = node) {
            if ( !(node->key < lkey) ) break;
        }
    }

    // Used only for benchmarks
    bool addAll(K** keys, const int size) {
        for (int i = 0; i < size; i++) add(*keys[i]);
        return true;
    }
};

#endif /* _PERSISTENT_SEQUENTIAL_LINKED_LIST_SETBYREF_H_ */
