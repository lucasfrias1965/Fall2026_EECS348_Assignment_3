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
