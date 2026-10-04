#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <vector>

struct Job {
    int priority;
    std::string name;
    bool operator<(const Job& o) const { return priority < o.priority; } // max-heap by priority
};

bool balanced(const std::string& s) {
    std::stack<char> st;
    for (char c : s) {
        if (c == '(' || c == '[' || c == '{') {
            st.push(c);
        } else if (c == ')' || c == ']' || c == '}') {
            if (st.empty()) return false;
            char open = st.top();
            st.pop();
            if ((c == ')' && open != '(') || (c == ']' && open != '[') || (c == '}' && open != '{')) return false;
        }
    }
    return st.empty();
}

int main() {
    for (std::string s : {"([]{})", "([)]", "((", "f(a[1]) { }"})
        std::cout << s << " balanced? " << std::boolalpha << balanced(s) << '\n';

    std::queue<std::string> line;
    line.push("Ada");
    line.push("Bjarne");
    line.push("Grace");
    while (!line.empty()) {
        std::cout << "serving " << line.front() << '\n';
        line.pop();
    }

    std::priority_queue<Job> jobs;
    jobs.push({1, "send newsletter"});
    jobs.push({5, "fix production bug"});
    jobs.push({3, "code review"});
    while (!jobs.empty()) {
        std::cout << "[" << jobs.top().priority << "] " << jobs.top().name << '\n';
        jobs.pop();
    }

    // min-heap of ints
    std::priority_queue<int, std::vector<int>, std::greater<int>> min_heap;
    for (int x : {5, 1, 8, 3}) min_heap.push(x);
    std::cout << "smallest: " << min_heap.top() << '\n';
}
