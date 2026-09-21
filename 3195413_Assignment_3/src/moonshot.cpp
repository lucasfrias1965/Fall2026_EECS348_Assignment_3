
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
using namespace std;

// ---------------- Date ----------------
class Date {
public:
    int month, day, year;
    Date() : month(0), day(0), year(0) {}
    // Build a date from an "MM-DD-YYYY" string
    Date(const string& s) {
        month = stoi(s.substr(0, 2));
        day   = stoi(s.substr(3, 2));
        year  = stoi(s.substr(6, 4));
    }
    // Numeric key so newer dates compare greater
    int key() const { return year * 10000 + month * 100 + day; }
    string str() const {
        char buf[11];
        snprintf(buf, sizeof(buf), "%02d-%02d-%04d", month, day, year);
        return string(buf);
    }
};

// ---------------- Email ----------------
class Email {
public:
    string sender;
    string subject;
    Date date;

    Email() {}

    // Parse one EMAIL line payload: "<sender>, <subject>, <date>"
    Email(const string& payload) {
        size_t c1 = payload.find(',');
        size_t c2 = payload.rfind(',');
        sender  = payload.substr(0, c1);
        subject = payload.substr(c1 + 1, c2 - c1 - 1);
        date    = Date(payload.substr(c2 + 1));
    }

    int priority() const {
        if (sender == "Boss")           return 5;
        if (sender == "Subordinate")    return 4;
        if (sender == "Peer")           return 3;
        if (sender == "ImportantPerson")return 2;
        return 1; // OtherPerson
    }

    // true if THIS email should be read before e
    bool higherThan(const Email& e) const {
        if (priority() != e.priority())
            return priority() > e.priority();
        return date.key() > e.date.key();   // newest first
    }
};

// ---------------- Heap node ----------------
class HeapNode {
public:
    Email data;
    HeapNode *parent, *left, *right;
    explicit HeapNode(const Email& e)
        : data(e), parent(nullptr), left(nullptr), right(nullptr) {}
};

// ---------------- MaxHeap (list-based) ----------------
class MaxHeap {
    HeapNode* root;
    int count;

    void clear(HeapNode* n) {
        if (!n) return;
        clear(n->left);
        clear(n->right);
        delete n;
    }

    // Locate the node at 1-based position n by following the
    // binary representation of n (drop leading 1; 0 = left, 1 = right)
    HeapNode* nodeAt(int n) const {
        HeapNode* cur = root;
        int bits = 31 - __builtin_clz((unsigned)n);   // index of leading bit
        for (int b = bits - 1; b >= 0; --b) {
            cur = ((n >> b) & 1) ? cur->right : cur->left;
            if (!cur) break;
        }
        return cur;
    }

    void siftUp(HeapNode* n) {
        while (n->parent && n->data.higherThan(n->parent->data)) {
            swap(n->data, n->parent->data);
            n = n->parent;
        }
    }

    void siftDown(HeapNode* n) {
        while (n) {
            HeapNode* best = n;
            if (n->left  && n->left->data.higherThan(best->data))  best = n->left;
            if (n->right && n->right->data.higherThan(best->data)) best = n->right;
            if (best == n) break;
            swap(n->data, best->data);
            n = best;
        }
    }

public:
    MaxHeap() : root(nullptr), count(0) {}
    ~MaxHeap() { clear(root); }

    int size() const { return count; }
    bool empty() const { return count == 0; }

    void insert(const Email& e) {
        HeapNode* node = new HeapNode(e);
        ++count;
        if (!root) { root = node; return; }
        // Attach at position 'count' (the next complete-tree slot)
        HeapNode* pos = nodeAt(count / 2);          // parent of new slot
        if (count % 2 == 0) pos->left = node; else pos->right = node;
        node->parent = pos;
        siftUp(node);
    }

    const Email& peekMax() const { return root->data; }

    Email extractMax() {
        Email top = root->data;
        HeapNode* last = nodeAt(count);             // last node in tree
        --count;
        if (last == root) {
            delete root;
            root = nullptr;
            return top;
        }
        root->data = last->data;                    // move last data to root
        if (last->parent->left == last) last->parent->left = nullptr;
        else last->parent->right = nullptr;
        delete last;
        siftDown(root);
        return top;
    }
};

// ---------------- main ----------------
int main(int argc, char* argv[]) {
    string filename;
    if (argc > 1) filename = argv[1];
    else { cout << "Enter input file name: "; cin >> filename; }

    ifstream in(filename);
    if (!in) { cerr << "Cannot open file: " << filename << endl; return 1; }

    MaxHeap inbox;
    string line;
    while (getline(in, line)) {
        // trim trailing whitespace / CR
        while (!line.empty() && isspace((unsigned char)line.back())) line.pop_back();
        if (line.empty()) continue;

        if (line.rfind("EMAIL ", 0) == 0) {
            inbox.insert(Email(line.substr(6)));
        } else if (line == "COUNT") {
            cout << "There are " << inbox.size() << " emails to read.\n\n";
        } else if (line == "NEXT") {
            if (inbox.empty()) {
                cout << "No emails to read.\n\n";
            } else {
                const Email& e = inbox.peekMax();
                cout << "Next email: \n"
                     << "Sender: "  << e.sender  << "\n"
                     << "Subject: " << e.subject << "\n"
                     << "Date: "    << e.date.str() << "\n\n";
            }
        } else if (line == "READ") {
            if (!inbox.empty()) inbox.extractMax();
        }
    }
    return 0;
}
