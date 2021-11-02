// single_threaded_execution.cpp

#include <cstdlib>
#include <cstring>

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>


namespace cmdp {

using std::chrono::milliseconds;
using std::cout;
using std::endl;
using std::lock_guard;
using std::ostream;
using std::recursive_mutex;

class Gate {

  private:
    int __counter_;
    const char* __name_;
    const char* __address_;
    mutable recursive_mutex __mtx_;
    void check() {
        if (__name_[0] != __address_[0]) {
            cout << "***** BROKEN *****" << *this << endl;
        } else {
            /*
             * When a thread goes into this branch, "cout << *this" will call
             * the friend operator<<() function and then the mutex will be
             * locked for the second time. To handle such case, std::mutex is
             * changed to std::recursive_mutex.
             */
            cout << "*****   OK   *****" << *this << endl;
        }
    }

  public:
    Gate() : __counter_(0), __name_(nullptr), __address_(nullptr) {}
    void pass(const char* name, const char* address) {
        lock_guard<recursive_mutex> guard(__mtx_);
        ++__counter_;
        __name_ = name;
        __address_ = address;
        check();
    }
    friend std::ostream& operator<<(std::ostream& os, const Gate& gate) {
        lock_guard<recursive_mutex> lk(gate.__mtx_);
        os << " No." << gate.__counter_ << ": " << gate.__name_ << ", " << gate.__address_;
        return os;
    }

};


class User {

  private:
    char* __name_;
    char* __address_;

  public:
    User(const char* name, const char* address) {
        __name_ = strdup(name);
        __address_ = strdup(address);
    }
    /*
     * When a functor acts as the initial function of a thread, if the functor
     * operates memory allocation for its member(s), the move constructor and
     * destructor functions should be carefully defined.
     */
    User(User&& user) {
        __name_ = user.__name_;
        __address_ = user.__address_;
        user.__name_ = nullptr;
        user.__address_ = nullptr;
    }
    ~User() {
        delete[] __name_;
        delete[] __address_;
    }
    void operator()(Gate& gate) {
        cout << __name_ << " BEGIN" << endl;
        while (true) {
            gate.pass(__name_, __address_);
            std::this_thread::sleep_for(milliseconds(rand() % 1000));
        }
    }

};


} // end of namespace cmdp


int main() {
    using std::cout;
    using std::endl;
    using std::thread;
    using namespace cmdp;

    srand(time(0));

    Gate gate;

    thread alice(User("Alice", "Alaska"), std::ref(gate));
    thread bobby(User("Bobby", "Brazil"), std::ref(gate));
    thread chris(User("Chris", "Canada"), std::ref(gate));

    alice.join();
    bobby.join();
    chris.join();

    cout << endl << "Bye Single Threaded Execution pattern..." << endl;
    return 0;
}
