// two_phase_termination.cpp

#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>


namespace cmdp {

using std::chrono::milliseconds;
using std::cout;
using std::endl;
using std::lock_guard;
using std::mutex;


class Countup {

  private:
    long __counter_;
    bool __shutdownrequested_;
    mutex __mtx_;

  public:
    Countup() : __counter_(0), __shutdownrequested_(false) {}
    void shutdown() {
        lock_guard<mutex> lk(__mtx_);
        __shutdownrequested_ = true;
    }
    void run() {
        while (!__shutdownrequested_) {
            ++__counter_;
            cout << "\tWorking: count = " << __counter_ << "\n";
            std::this_thread::sleep_for(milliseconds(500));
        }
        cout << "\tShutdown: count = " << __counter_ << "\n";
    }

};


} // end of namespace cmdp


int main() {
    using std::cout;
    using std::endl;
    using std::thread;
    using namespace cmdp;

    cout << "main: BEGIN\n";

    Countup c;
    thread t(&Countup::run, &c);

    std::this_thread::sleep_for(milliseconds(10000));

    cout << "main: shutdown requested\n";
    c.shutdown();

    t.join();
    cout << "main: END\n";

    cout << endl << "Bye Two-Phase Termination pattern..." << endl;
    return 0;
}

