#include <string>

struct CircularBufferNode {
    CircularBufferNode* next;
    CircularBufferNode* prev;
    int id;
    std::string filepath;
    bool allocated;

    CircularBufferNode(CircularBufferNode* next, CircularBufferNode* prev, int id, std::string filepath, bool allocated=false) :
        next(next), prev(prev), id(id), filepath(filepath), allocated(allocated) {}

    CircularBufferNode* circularNext() {
        return next;
    }

    CircularBufferNode* circularPrev() {
        return prev;
    }

    CircularBufferNode* operator++() {
        return circularNext();
    }

    CircularBufferNode* operator--() {
        return circularPrev();
    }

    CircularBufferNode* operator++(int) {
        CircularBufferNode* tmp = circularNext();
        return tmp;
    }

    CircularBufferNode* operator--(int) {
        CircularBufferNode* tmp = circularPrev();
        return tmp;
    }

    operator bool() {
          return allocated;
    }
};

struct CircularBuffer {
    CircularBufferNode* head;

    CircularBuffer() {
        head = nullptr;
    }

    void init(int size, std::string folder) {
        head = new CircularBufferNode(nullptr, nullptr, 0, folder + "0.bak");
        CircularBufferNode* current = head;
        for(int i = 1; i < size; i++) {
            current->next = new CircularBufferNode(nullptr, current, i, folder + std::to_string(i) + ".bak");
            current = current->next;
        }
        current->next = head;
        head->prev = current;
    }

    void free() {
        if (!head) return;
        CircularBufferNode* current = head;
        do {
            CircularBufferNode* next = current->next;
            delete current;
            current = next;
        } while(current != head);
        head = nullptr;
    }

    ~CircularBuffer() {
        free();
    }

    void clear() {
        CircularBufferNode* current = head;
        do {
            current->allocated = false;
            current = current->next;
        } while(current != head);
    }

    // prefix operator checks for allocation
    CircularBufferNode* operator++() {
        if (head->next->allocated) {
            head = head->next;
        }
        return head;
    }

    // prefix operator checks for allocation
    CircularBufferNode* operator--() {
        if (head->prev->allocated) {
            head = head->prev;
        }
        return head;
    }

    // postfix operator doesnt check for allocation
    CircularBufferNode* operator++(int) {
        CircularBufferNode* tmp = head;
        head = head->next;
        return tmp;
    }

    // postfix operator doesnt check for allocation
    CircularBufferNode* operator--(int) {
        CircularBufferNode* tmp = head;
        head = head->prev;
        return tmp;
    }

    CircularBufferNode* operator[](int id) { // get the element with id
        CircularBufferNode* current = head;
        do {
            if(current->id == id) {
                return current;
            }
            current = current->next;
        } while(current != head);
        return nullptr;
    }

    // checks if any is allocated
    bool any() { 
        CircularBufferNode* current = head;
        do {
            if(current->allocated) {
                return true;
            }
            current = current->next;
        } while(current != head);
        return false;
    }

    // checks if all are allocated
    bool all() { 
        CircularBufferNode* current = head;
        do {
            if(!current->allocated) {
                return false;
            }
            current = current->next;
        } while(current != head);
        return true;
    }
};



