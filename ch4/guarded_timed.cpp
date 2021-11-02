// guarded_timed.cpp


#include <chrono>
#include <condition_variable>
#include <exception>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>


namespace cmdp {


using std::chrono::duration;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::chrono::time_point;
using std::condition_variable;
using std::cout;
using std::endl;
using std::lock_guard;
using std::mutex;
using std::ostringstream;
using std::runtime_error;
using std::string;
using std::unique_lock;



class Timeout_Exception : public runtime_error {

  private:
    long __timeout;
    long __passedtime;

  public:
    Timeout_Exception(long to, long pt)
        : runtime_error("Timeout exception happened."), __timeout(to), __passedtime(pt) {}
    virtual const char* what() const noexcept(true) {
        ostringstream oss;
        oss << "\nNow - start = " << __passedtime
            << " microseconds, timeout = " << __timeout << " microseconds.";
        string msg;
        msg += runtime_error::what();
        msg += oss.str();
        return msg.c_str();
    }

};


class Host {

  private:
    const long __timeout;
    bool __ready;
    mutex __mtx;
    condition_variable __cv;

    void __do_execute() {
        ostringstream oss;
        oss << std::this_thread::get_id() << " calls do_execute.\n";
        cout << oss.str();
    }

  public:
    Host(long to) : __timeout(to), __ready(false) {}
    void set_executable(bool on) {
        lock_guard<mutex> lk(__mtx);
        __ready = on;
        __cv.notify_all();
    }
    void execute() {
        unique_lock<mutex> lk(__mtx);
        time_point<steady_clock> start = steady_clock::now();
        while (!__ready) {
            time_point<steady_clock> now = steady_clock::now();
            long passedtime = std::chrono::duration_cast<microseconds>(now - start).count();
            long rest = __timeout - passedtime;
            if (rest <= 0)
                throw Timeout_Exception(__timeout, passedtime);
            __cv.wait_for(lk, microseconds(rest));
        }
        __do_execute();
    }

};


} // end of namespace cmdp


int main() {
    using std::cout;
    using std::endl;
    using std::thread;
    using namespace cmdp;

    Host host(1000000);
    cout << "Begin executing..." << endl;
    try {
        host.execute();
    } catch (Timeout_Exception& te) {
        cout << te.what() << endl;
    }

    cout << endl << "Bye Guarded Timeout..." << endl;
    return 0;
}

