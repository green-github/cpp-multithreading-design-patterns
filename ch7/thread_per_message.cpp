// thread_per_message.cpp


#include <chrono>
#include <future>
#include <iostream>
#include <thread>


namespace cmdp {


using std::chrono::milliseconds;
using std::cout;
using std::endl;
using std::future;
using std::launch;
using std::thread;


class Helper {

  public:
    void handle(int count, char c) const {
        cout << "   handle(" << count << ", " << c << ") BEGIN\n";
        for (int i = 0; i < count; ++i) {
            std::this_thread::sleep_for(milliseconds(100));
            cout << c;
        }
        cout << "\n   handle(" << count << ", " << c << ") END\n";
    }

};


class Host {

  private:
    const Helper* __helper_;

  public:
    Host() : __helper_(new Helper()) {}
    ~Host() { delete __helper_; }
    /*
     * -- 1 --
     * transfer the ownership of the thread to the caller.
     */
    thread request(int count, char c) {
        cout << "   request(" << count << ", " << c << ") BEGIN\n";
        thread t([this, count, c] {
            __helper_->handle(count, c);
        });
        cout << "   request(" << count << ", " << c << ") END\n";
        return t;
    }
    /*
     * -- 2 --
     * async task with future
     */
    //future<void> request(int count, char c) {
    //    cout << "   request(" << count << ", " << c << ") BEGIN\n";
    //    future<void> f = std::async(launch::async, [=] {
    //        __helper_->handle(count, c);
    //    });
    //    cout << "   request(" << count << ", " << c << ") END\n";
    //    return f;
    //}

};


} // end of namespace cmdp


int main() {
    using std::cout;
    using std::endl;
    using std::thread;
    using namespace cmdp;

    cout << "main BEGIN\n";
    Host host;

    // -- 1 --
    thread t1 = host.request(30, 'A');
    thread t2 = host.request(40, 'B');
    thread t3 = host.request(50, 'C');
    t1.join();
    t2.join();
    t3.join();

    // -- 2 --
    //future<void> f1 = host.request(30, 'A');
    //future<void> f2 = host.request(40, 'B');
    //future<void> f3 = host.request(50, 'C');
    //f1.get();
    //f2.get();
    //f3.get();

    cout << "main END\n";

    cout << endl << "Bye Thread-Per-Message pattern..." << endl;
    return 0;
}

