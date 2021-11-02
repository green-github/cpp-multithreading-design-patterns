// thread_specific_storage.cpp
// namespace variables, class static variables and method local variables
// can be qualified for 'thread_local'.

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>


namespace cmdp {


using std::chrono::milliseconds;
using std::cout;
using std::endl;
using std::ofstream;
using std::ostringstream;
using std::string;

// Just to demo the namespace variable can be thread_local
thread_local bool FLAG = false;


string this_thread_id() {
    ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}


class Logger {

  private:
    // Just to demo the class static variable can be thread_local
    static thread_local ofstream __ofs__;

  public:
    void println(const string& message) {
        __ofs__ << &FLAG << ", " << message << endl;
    }
    void close() {
        __ofs__.close();
    }

};

thread_local ofstream Logger::__ofs__(this_thread_id() + ".log");


class Client {

  private:
    string __name_;
    Logger __logger_;

  public:
    Client(string name) : __name_(name) {}
    void run() {
        cout << this_thread_id() << " BEGIN\n";
        for (int i = 0; i < 10; ++i) {
            string msg;
            msg += "i = ";
            msg += std::to_string(i);
            __logger_.println(msg);
        }
        __logger_.close();
        cout << this_thread_id() << " END\n";
    }

};


} // end of namespace cmdp


int main() {
    using std::cout;
    using std::endl;
    using std::thread;
    using namespace cmdp;

    Client alice("Alice");
    Client bobby("Bobby");
    Client chris("Chris");
    thread t1(&Client::run, &alice);
    thread t2(&Client::run, &bobby);
    thread t3(&Client::run, &chris);

    t1.join();
    t2.join();
    t3.join();

    cout << endl << "Bye Thread-Specific Storage pattern..." << endl;
    return 0;
}

