#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

class IntList {
    struct Node {
        int value;
        std::unique_ptr<Node> next;
    };

public:
    IntList() = default;
    IntList(const IntList&) = delete; // keep the exercise focused: move-only
    IntList& operator=(const IntList&) = delete;
    IntList(IntList&&) = default;
    IntList& operator=(IntList&&) = default;

    // Iterative destruction: unlink nodes one by one so each node's destructor
    // has an empty `next` and no deep recursion happens.
    ~IntList() {
        while (head_) head_ = std::move(head_->next);
    }

    void push_front(int v) {
        head_ = std::make_unique<Node>(Node{v, std::move(head_)});
        ++size_;
    }

    int pop_front() {
        if (!head_) throw std::out_of_range("pop_front on empty list");
        int v = head_->value;
        head_ = std::move(head_->next); // old head is destroyed here
        --size_;
        return v;
    }

    void reverse() {
        std::unique_ptr<Node> prev;
        while (head_) {
            auto next = std::move(head_->next); // detach the rest
            head_->next = std::move(prev);      // point current back
            prev = std::move(head_);            // advance prev
            head_ = std::move(next);            // advance head
        }
        head_ = std::move(prev);
    }

    std::size_t size() const { return size_; }

    friend std::ostream& operator<<(std::ostream& os, const IntList& l) {
        os << '[';
        for (const Node* n = l.head_.get(); n; n = n->next.get()) { // raw pointers to OBSERVE
            os << n->value << (n->next ? ", " : "");
        }
        return os << ']';
    }

private:
    std::unique_ptr<Node> head_;
    std::size_t size_ = 0;
};

int main() {
    IntList list;
    for (int i = 1; i <= 5; ++i) list.push_front(i);
    std::cout << list << " size=" << list.size() << '\n';

    list.reverse();
    std::cout << "reversed: " << list << '\n';

    std::cout << "pop_front -> " << list.pop_front() << ", now " << list << '\n';

    IntList big;
    for (int i = 0; i < 1'000'000; ++i) big.push_front(i);
    std::cout << "built a list of " << big.size() << " nodes; destroying it iteratively...\n";
} // with the default destructor this could crash with a stack overflow
