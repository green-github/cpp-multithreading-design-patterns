// producer_consumer.cpp

#include <cstdlib>

#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>


namespace cmdp {


using std::chrono::milliseconds;
using std::condition_variable;
using std::cout;
using std::endl;
using std::lock_guard;
using std::mutex;
using std::ostringstream;
using std::string;
using std::unique_lock;


string this_thread_id() {
    ostringstream oss;
    oss << std::this_thread::get_id();
    return string(oss.str());
}


class Table {

  private:
    string* __buffer_;
    const unsigned __capacity_;
    unsigned __tail_;
    unsigned __head_;
    unsigned __count_;
    mutex __mtx_;
    condition_variable __cv_;

  public:
    Table(int capacity)
        : __capacity_(capacity), __tail_(0), __head_(0), __count_(0) {
        __buffer_ = new string[__capacity_];
    }
    void put(string cake, const char* name) {
        unique_lock<mutex> lk(__mtx_);
        cout << name << " puts " + cake + "\n";
        __cv_.wait(lk, [&] { return __count_ < __capacity_; });
        __buffer_[__tail_] = cake;
        __tail_ = (__tail_ + 1) % __capacity_;
        ++__count_;
        __cv_.notify_all();
    }
    string take(const char* name) {
        unique_lock<mutex> lk(__mtx_);
        __cv_.wait(lk, [&] { return __count_ > 0; });
        string cake(__buffer_[__head_]);
        __head_ = (__head_ + 1) % __capacity_;
        --__count_;
        __cv_.notify_all();
        cout << name << " takes " + cake + "\n";
        return cake;
    }

};


class Maker {

  private:
    string __name_;
    static mutex __smtx__;
    static unsigned __id__;
    static unsigned __nextid() {
        lock_guard<mutex> lk(__smtx__);
        return ++__id__;
    }

  public:
    Maker(string name) : __name_(name) {}
    void run(Table& table) {
        while (true) {
            std::this_thread::sleep_for(milliseconds(rand()%1000));
            string cake;
            cake += "[Cake No.";
            cake += std::to_string(__nextid());
            cake += " by ";
            cake += __name_;
            cake += "]";
            table.put(cake, __name_.c_str());
        }
    }

};

unsigned Maker::__id__ = 0;
mutex Maker::__smtx__;


class Eater {

  private:
    string __name_;

  public:
    Eater(string name) : __name_(name) {}
    void run(Table& table) {
        while (true) {
            string cake = table.take(__name_.c_str());
            std::this_thread::sleep_for(milliseconds(rand()%3000));
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

    Table table(3);

    Maker m1("Maker-1");
    Maker m2("Maker-2");
    Maker m3("Maker-3");
    Eater e1("Eater-1");
    Eater e2("Eater-2");
    Eater e3("Eater-3");

    thread mt1(&Maker::run, &m1, std::ref(table));
    thread mt2(&Maker::run, &m2, std::ref(table));
    thread mt3(&Maker::run, &m3, std::ref(table));
    thread et1(&Eater::run, &e1, std::ref(table));
    thread et2(&Eater::run, &e2, std::ref(table));
    thread et3(&Eater::run, &e3, std::ref(table));

    mt1.join();
    mt2.join();
    mt3.join();
    et1.join();
    et2.join();
    et3.join();

    cout << endl << "Bye Producer-Consumer pattern..." << endl;
    return 0;
}

