// read_write_lock.cpp


#include <cstdlib>
#include <cstring>

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>


namespace cmdp {


using std::chrono::milliseconds;
using std::condition_variable;
using std::cout;
using std::endl;
using std::lock_guard;
using std::mutex;
using std::pair;
using std::string;
using std::unique_lock;


class Read_Write_Lock {

  private:
    unsigned __readingreaders_;
    unsigned __waitingwriters_;
    unsigned __writingwriters_;
    bool __preferwriter_;
    mutex __mtx_;
    condition_variable __cv_;

  public:
    Read_Write_Lock() :
        __readingreaders_(0), __waitingwriters_(0),
        __writingwriters_(0), __preferwriter_(true) {}
    void read_lock() {
        unique_lock<mutex> lk(__mtx_);
        __cv_.wait(lk, [this] {
            return !(__writingwriters_ > 0 || (__preferwriter_ && __waitingwriters_ > 0));
        });
        ++__readingreaders_;
    }
    void read_unlock() {
        lock_guard<mutex> lk(__mtx_);
        --__readingreaders_;
        __preferwriter_ = true;
        __cv_.notify_all();
    }
    void write_lock() {
        unique_lock<mutex> lk(__mtx_);
        ++__waitingwriters_;
        __cv_.wait(lk, [this] {
            return !(__readingreaders_ > 0 || __writingwriters_ > 0);
        });
        --__waitingwriters_;
        ++__writingwriters_;
    }
    void write_unlock() {
        lock_guard<mutex> lk(__mtx_);
        --__writingwriters_;
        __preferwriter_ = false;
        __cv_.notify_all();
    }

};


class Data {

  private:
    char* __buffer_;
    size_t __capacity_;
    Read_Write_Lock __lock_;

    pair<char*, size_t> __do_read() {
        char* newbuffer = new char[__capacity_];
        std::memcpy(newbuffer, __buffer_, __capacity_);
        return std::make_pair(newbuffer, __capacity_);
    }
    void __do_write(char c) {
        for (size_t s = 0; s < __capacity_; ++s) {
            __buffer_[s] = c;
            std::this_thread::sleep_for(milliseconds(50));
        }
    }

  public:
    Data(size_t capacity) : __capacity_(capacity) {
        __buffer_ = new char[__capacity_];
        for (size_t s = 0; s < __capacity_; ++s)
            __buffer_[s] = '*';
    }
    pair<char*, size_t> read() {
        __lock_.read_lock();
        pair<char*, size_t> newbuffer = __do_read();
        __lock_.read_unlock();
        return newbuffer;
    }
    void write(char c) {
        __lock_.write_lock();
        __do_write(c);
        __lock_.write_unlock();
    }

};


class Writer {

  private:
    string __name_;
    Data* __data_;
    string __filler_;
    size_t __index_;

    char __nextchar() {
        char c = __filler_[__index_];
        ++__index_;
        if (__index_ >= __filler_.size())
            __index_ = 0;
        return c;
    }

  public:
    Writer(string name, Data* data, string filler) :
        __name_(name), __data_(data), __filler_(filler), __index_(0) {}
    void run() {
        while (true) {
            char c = __nextchar();
            __data_->write(c);
            std::this_thread::sleep_for(milliseconds(rand() % 30));
        }
    }

};


class Reader {

  private:
    string __name_;
    Data* __data_;

  public:
    Reader(string name, Data* data) : __name_(name), __data_(data) {}
    void run() {
        while (true) {
            char* buf = __data_->read().first;
            cout << __name_ + " reads " + buf + "\n";
            delete[] buf;
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

    Data data(10);

    Reader r1("Reader-1", &data);
    Reader r2("Reader-2", &data);
    Reader r3("Reader-3", &data);
    Reader r4("Reader-4", &data);
    Reader r5("Reader-5", &data);
    Reader r6("Reader-6", &data);
    Writer w1("Writer-1", &data, "abcdefghijklmnopqrstuvwxyz");
    Writer w2("Writer-2", &data, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    thread wt1(&Writer::run, &w1);
    thread wt2(&Writer::run, &w2);
    thread rt1(&Reader::run, &r1);
    thread rt2(&Reader::run, &r2);
    thread rt3(&Reader::run, &r3);
    thread rt4(&Reader::run, &r4);
    thread rt5(&Reader::run, &r5);
    thread rt6(&Reader::run, &r6);

    wt1.join();
    wt2.join();
    rt1.join();
    rt2.join();
    rt3.join();
    rt4.join();
    rt5.join();
    rt6.join();

    cout << endl << "Bye Read-Write Lock pattern..." << endl;
    return 0;
}

