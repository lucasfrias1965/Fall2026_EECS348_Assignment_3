# Assignment 3 - EECS 348 (Software Engineering) by Lucas Frias

Pleasure to have you read this assignment, as always. Today's will be difficult for me, as I am not one of the most frequent enjoyers of C++. Even still, I hope that I will be able to provide decent analysis.

On the model frontier, there was some news a couple weeks ago about KIMI3 by a Chinese-based company called Moonshot. Their new frontier model is open source, and very cheap because of that. Anthropic claims that their models were distelled and "stolen" by KIMI3. It is interesting, because we are seeing the fight between private and public development of LLMs and their future. I decided to use Moonshot's KIMI3.

Before the grader becomes apprehensive of me comparing like AI, I believe that Anthropic is not being fully transparent with their claims, and even if the claims are true, KIMI is different enough from just a pure copy paste model in the design choices it implemented. These are seperate models.

For Claude, I did the same method as before. Our returner will be Sonnet (on High reasoning). I was able to access this model for signing up for a free account on Claude.ai, and then prompted it the prompt in PROMPT.md by copy and pasting the text. I have gotten a Claude Pro subscription between this assignment and the next, but I do not think it overly impacted the reasoning, as I used the website version (no access to the terminal or other tools not already provided to it by Anthropic). The reasoning depths might have been increased slightly, however.

To access KIMI, I had to use a Google account to sign up for the online portal after denying my U.S. based phone number. After jumping through this hurdle, the website worked about as much as any other chat bot would. I copy and pasted the text into the prompt. Here's the prompt I used for both of them:


```
Make this program in C++. You have the added requirement to code using objects, not functions.

•	The program will prioritize emails for a busy company CEO.  
•	You will use a MaxHeap as a means of implementing a priority queue. A priority queue is a queue where emails can shift towards the front of the queue based on a priority status. 
•	You must implement a MaxHeap using a list-based implementation. Then use that MaxHeap to handle all your email prioritizing for the CEO. 
•	You must create functions from scratch. Do not include pre-existing heap modules. 


Here's the file format:

-------------
EMAIL <sender category>, <subject line>, <date> - The emails in the CEO’s Inbox should be placed in queue based on their sender category and date. The sender categories and priority to be read are as follows:  
     •	Boss – read first   
     •	Subordinate – read next  
     •	Peer – read next   
     •	ImportantPerson – read next   
     •	OtherPerson – read last   
     If there is more than one from a sender, then the newest email (not the oldest) should be read first. I discovered this trick while a manager at Sprint.      EMAIL is followed by space. The rest of the fields are delimited.   Assume <sender category> is one of the five strings listed above.   Assume <subject line> is a string which may contain spaces, but not commas   Assume <date> is in the format: MM-DD-YYYY
--------------
NEXT - Next email for the CEO to read. Display the information on the terminal in the following format: 
     Sender: 
     Subject: 
     Date: 
------------
READ - CEO has read with the email and has dealt with it
-----------
COUNT - display current count of untracked files


Here's an example file:
EMAIL Peer,Can you help me on this?,12-01-2024 
EMAIL OtherPerson,Try our product,12-19-2024 
EMAIL Boss,Important,12-20-2024 
EMAIL Subordinate,How do I handle this?,12-25-2024 
EMAIL ImportantPerson,Health Insurance Enrollment,12-31-2024 
EMAIL Boss,Never Mind,01-03-2025 
COUNT 
NEXT 
READ 
NEXT 
READ 
COUNT 
Here is the output that the program should give
There are 6 emails to read. 
 
Next email: 
Sender: Boss 
Subject: Never Mind 
Date: 01-03-2025 
 
Next email: 
Sender: Boss 
Subject: Important 
Date: 12-20-2024 
 
There are 4 emails to read. 

Good luck!
```

Formatting with backticks was removed in this copy and paste, but otherwise this is the identical prompt. 

# Output
## Anthropic's Sonnet's Output

```c
// ceo_inbox.cpp
// prioritizes emails for a ceo using a hand-built max heap.
// build: g++ -std=c++11 -Wall -Wextra -o ceo_inbox ceo_inbox.cpp
// run:   ./ceo_inbox inbox.txt

#include <fstream>
#include <iostream>
#include <string>

// ---------------------------------------------------------------
// email: one message. knows how to rank itself.
// ---------------------------------------------------------------
class Email {
private:
    std::string sender;
    std::string subject;
    std::string date;       // MM-DD-YYYY
    long long   sequence;   // arrival order, breaks exact ties (earlier wins)

    // higher rank = read sooner
    static int RankOf(const std::string& category) {
        if (category == "Boss")            return 5;
        if (category == "Subordinate")     return 4;
        if (category == "Peer")            return 3;
        if (category == "ImportantPerson") return 2;
        if (category == "OtherPerson")     return 1;
        return 0;   // unknown
    }

    // MM-DD-YYYY -> YYYYMMDD so a bigger number means a newer date
    static long DateKeyOf(const std::string& d) {
        if (d.size() != 10 || d[2] != '-' || d[5] != '-') return 0;
        int month = std::stoi(d.substr(0, 2));
        int day   = std::stoi(d.substr(3, 2));
        int year  = std::stoi(d.substr(6, 4));
        return static_cast<long>(year) * 10000L + month * 100L + day;
    }

public:
    Email() : sender(""), subject(""), date(""), sequence(0) {}

    Email(const std::string& s, const std::string& subj,
          const std::string& d, long long seq)
        : sender(s), subject(subj), date(d), sequence(seq) {}

    const std::string& GetSender()  const { return sender; }
    const std::string& GetSubject() const { return subject; }
    const std::string& GetDate()    const { return date; }

    static bool IsValidCategory(const std::string& category) {
        return RankOf(category) > 0;
    }

    static bool IsValidDate(const std::string& d) {
        if (d.size() != 10 || d[2] != '-' || d[5] != '-') return false;
        for (std::size_t i = 0; i < d.size(); ++i) {
            if (i == 2 || i == 5) continue;
            if (d[i] < '0' || d[i] > '9') return false;
        }
        return true;
    }

    // true when this email must be read before the other one
    bool HasHigherPriorityThan(const Email& other) const {
        int mine   = RankOf(sender);
        int theirs = RankOf(other.sender);
        if (mine != theirs) return mine > theirs;

        long myDate    = DateKeyOf(date);
        long theirDate = DateKeyOf(other.date);
        if (myDate != theirDate) return myDate > theirDate;   // newest first

        return sequence < other.sequence;                     // earlier arrival first
    }
};

// ---------------------------------------------------------------
// maxheap: list-based (array) binary heap. built from scratch.
// parent of i is (i-1)/2. children of i are 2i+1 and 2i+2.
// ---------------------------------------------------------------
class MaxHeap {
private:
    Email*      items;
    std::size_t count;
    std::size_t capacity;

    void Grow() {
        std::size_t newCapacity = capacity * 2;
        Email* bigger = new Email[newCapacity];
        for (std::size_t i = 0; i < count; ++i) bigger[i] = items[i];
        delete[] items;
        items    = bigger;
        capacity = newCapacity;
    }

    void Swap(std::size_t a, std::size_t b) {
        Email temp = items[a];
        items[a]   = items[b];
        items[b]   = temp;
    }

    void SiftUp(std::size_t index) {
        while (index > 0) {
            std::size_t parent = (index - 1) / 2;
            if (items[index].HasHigherPriorityThan(items[parent])) {
                Swap(index, parent);
                index = parent;
            } else {
                break;
            }
        }
    }

    void SiftDown(std::size_t index) {
        while (true) {
            std::size_t left    = 2 * index + 1;
            std::size_t right   = 2 * index + 2;
            std::size_t largest = index;

            if (left < count && items[left].HasHigherPriorityThan(items[largest]))
                largest = left;
            if (right < count && items[right].HasHigherPriorityThan(items[largest]))
                largest = right;

            if (largest == index) break;
            Swap(index, largest);
            index = largest;
        }
    }

public:
    MaxHeap() : items(new Email[8]), count(0), capacity(8) {}
    ~MaxHeap() { delete[] items; }

    // a raw owned array makes copying unsafe. forbid it.
    MaxHeap(const MaxHeap&)            = delete;
    MaxHeap& operator=(const MaxHeap&) = delete;

    bool        IsEmpty() const { return count == 0; }
    std::size_t Size()    const { return count; }

    void Insert(const Email& email) {
        if (count == capacity) Grow();
        items[count] = email;
        SiftUp(count);
        ++count;
    }

    // caller must check IsEmpty() first
    const Email& Peek() const { return items[0]; }

    // caller must check IsEmpty() first
    Email ExtractMax() {
        Email top = items[0];
        --count;
        if (count > 0) {
            items[0] = items[count];
            SiftDown(0);
        }
        return top;
    }
};

// ---------------------------------------------------------------
// inboxprocessor: owns the heap, reads the command file, prints results.
// ---------------------------------------------------------------
class InboxProcessor {
private:
    MaxHeap   heap;
    long long nextSequence;

    static std::string Trim(const std::string& text) {
        const std::string whitespace = " \t\r\n";
        std::size_t first = text.find_first_not_of(whitespace);
        if (first == std::string::npos) return "";
        std::size_t last = text.find_last_not_of(whitespace);
        return text.substr(first, last - first + 1);
    }

    void HandleEmail(const std::string& fields) {
        std::size_t firstComma  = fields.find(',');
        std::size_t secondComma = (firstComma == std::string::npos)
                                      ? std::string::npos
                                      : fields.find(',', firstComma + 1);
        if (secondComma == std::string::npos) {
            std::cerr << "skipped bad EMAIL line (need 3 fields): " << fields << "\n";
            return;
        }

        std::string sender  = Trim(fields.substr(0, firstComma));
        std::string subject = Trim(fields.substr(firstComma + 1,
                                                 secondComma - firstComma - 1));
        std::string date    = Trim(fields.substr(secondComma + 1));

        if (!Email::IsValidCategory(sender)) {
            std::cerr << "skipped EMAIL with unknown sender: " << sender << "\n";
            return;
        }
        if (!Email::IsValidDate(date)) {
            std::cerr << "skipped EMAIL with bad date: " << date << "\n";
            return;
        }

        heap.Insert(Email(sender, subject, date, nextSequence++));
    }

    void HandleCount() const {
        std::cout << "There are " << heap.Size() << " emails to read.\n\n";
    }

    void HandleNext() const {
        if (heap.IsEmpty()) {
            std::cout << "No emails to read.\n\n";
            return;
        }
        const Email& top = heap.Peek();
        std::cout << "Next email:\n"
                  << "Sender: "  << top.GetSender()  << "\n"
                  << "Subject: " << top.GetSubject() << "\n"
                  << "Date: "    << top.GetDate()    << "\n\n";
    }

    void HandleRead() {
        if (heap.IsEmpty()) {
            std::cout << "No emails to read.\n\n";
            return;
        }
        heap.ExtractMax();
    }

public:
    InboxProcessor() : heap(), nextSequence(0) {}

    bool ProcessFile(const std::string& path) {
        std::ifstream input(path.c_str());
        if (!input.is_open()) {
            std::cerr << "cannot open file: " << path << "\n";
            return false;
        }

        std::string line;
        while (std::getline(input, line)) {
            line = Trim(line);
            if (line.empty()) continue;

            std::size_t space   = line.find(' ');
            std::string command = (space == std::string::npos) ? line
                                                               : line.substr(0, space);
            std::string rest    = (space == std::string::npos) ? ""
                                                               : line.substr(space + 1);

            if (command == "EMAIL")      HandleEmail(rest);
            else if (command == "COUNT") HandleCount();
            else if (command == "NEXT")  HandleNext();
            else if (command == "READ")  HandleRead();
            else std::cerr << "skipped unknown command: " << line << "\n";
        }
        return true;
    }
};

// ---------------------------------------------------------------
// application: entry point wrapper so main holds no logic.
// ---------------------------------------------------------------
class Application {
public:
    int Run(int argc, char* argv[]) {
        std::string path;
        if (argc > 1) {
            path = argv[1];
        } else {
            std::cout << "input file: ";
            std::getline(std::cin, path);
        }

        InboxProcessor processor;
        return processor.ProcessFile(path) ? 0 : 1;
    }
};

int main(int argc, char* argv[]) {
    Application app;
    return app.Run(argc, argv);
}
```

## Kimi 3's Output:
```c
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
```
# Correctness
I was reasonably impressed on a first inspection with the output of the two in terms of correctness. In fact, they output the exact same output. However, interally they function differently. We will discuss this later. Just to be said that both are "correct". 

As per the main requirement, they use objects and methods within the objects instead of functions. They also implement their own "list" based implementation, as the rubric given on Canvas states:

```
•	You will use a MaxHeap as a means of implementing a priority queue. A priority queue is a queue where emails can shift towards the front of the queue based on a priority status. 
•	You must implement a MaxHeap using a list-based implementation. Then use that MaxHeap to handle all your email prioritizing for the CEO. 
```

But both of them did this tremendously differently. While Sonnet implemented a similar method to what it had in C (using a heap allocated array which it dynamically grows) KIMI used a linked list!! This was shocking to me. I mentioned it as a possibility in Assignment 2 previously as a slightly impractical but interesting solution, and the reason why was the cost of lookup not being O(1), but is O(log(N)). We'll discuss whether this was a decent choice or not in the next section.

One last comment: as much as I have distaste in C++, I have to concede that the C++ implementation is more legible and better when implementing complex data structures. Reading takes much less mental effort.

Given test inputs generated by me and my test file generator in Python, I found that the programs were functional identitcally. However, under the hood there's a real distance. Let's discuss that.



