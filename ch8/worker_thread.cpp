// worker_thread.cpp

#include <cstdlib>

#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>


namespace cmdp {


using std::chrono::milliseconds;
using std::condition_variable;
using std::cout;
using std::endl;
using std::mutex;
using std::ostringstream;
using std::pair;
using std::string;
using std::thread;
using std::unique_lock;


string this_thread_id() {
    ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}


class Request {

  private:
    string __consignor_;
    int __no_;

  public:
    Request(string consignor, int no) : __consignor_(consignor), __no_(no) {}
    void execute() {
        string msg;
        msg += this_thread_id();
        msg += " executes [Request from ";
        msg += __consignor_;
        msg += " No. ";
        msg += std::to_string(__no_);
        msg += " ]\n";
        cout << msg;
        std::this_thread::sleep_for(milliseconds(rand() % 1000));
    }

};


class Channel {

  private:
    static const size_t __MAX_REQUEST_NO__;
    Request** __requestqueue_;
    size_t __tail_;
    size_t __head_;
    size_t __count_;
    thread** __threadpool_;
    size_t __poolsize_;
    mutex __mtx_;
    condition_variable __cv_;

  public:
    Channel(int poolsize) :
        __poolsize_(poolsize), __tail_(0), __head_(0), __count_(0) {
        __requestqueue_ = new Request*[__MAX_REQUEST_NO__];
        __threadpool_ = new thread*[__poolsize_];
    }
    ~Channel() {
        for (size_t i = 0; i < __count_; ++i)
            delete __requestqueue_[i];
        for (size_t i = 0; i < __poolsize_; ++i)
            delete __threadpool_[i];
        delete[] __requestqueue_;
        delete[] __threadpool_;
    }
    pair<thread**, size_t> start_working();
    void put_request(Request* req) {
        unique_lock<mutex> lk(__mtx_);
        __cv_.wait(lk, [this] { return __count_ < __MAX_REQUEST_NO__; });
        __requestqueue_[__tail_] = req;
        __tail_ = (__tail_ + 1) % __MAX_REQUEST_NO__;
        ++__count_;
        __cv_.notify_all();
    }
    Request* take_request() {
        unique_lock<mutex> lk(__mtx_);
        __cv_.wait(lk, [this] { return __count_ > 0; });
        Request* req = __requestqueue_[__head_];
        __head_ = (__head_ + 1) % __MAX_REQUEST_NO__;
        --__count_;
        __cv_.notify_all();
        return req;
    }

};

size_t const Channel::__MAX_REQUEST_NO__ = 100;


class Worker {

  private:
    string __name_;
    Channel* __channel_;

  public:
    Worker(string name, Channel* channel) : __name_(name), __channel_(channel) {}
    void run() {
        string msg;
        msg += __name_;
        msg += "'s id: ";
        msg += this_thread_id();
        cout << msg << endl;
        while (true) {
            Request* req = __channel_->take_request();
            req->execute();
            delete req;
        }
    }

};


class Client {

  private:
    string __name_;
    Channel* __channel_;

  public:
    Client(string name, Channel* channel) : __name_(name), __channel_(channel) {}
    void run() {
        string msg;
        msg += __name_;
        msg += "'s id: ";
        msg += this_thread_id();
        cout << msg << endl;
        for (int i = 1; true; ++i) {
            Request* req = new Request(__name_, i);
            __channel_->put_request(req);
            std::this_thread::sleep_for(milliseconds(rand() % 1000));
        }
    }

};


pair<thread**, size_t> Channel::start_working() {
    for (size_t i = 0; i < __poolsize_; ++i) {
        string name("Worker-");
        name += std::to_string(i);
        __threadpool_[i] = new thread(&Worker::run, new Worker(name, this));
    }
    return std::make_pair(__threadpool_, __poolsize_);
}


} // end of namespace cmdp


int main() {
    using std::cout;
    using std::endl;
    using std::thread;
    using namespace cmdp;

    srand(time(0));

    Channel channel(5);
    pair<thread**, size_t> workers = channel.start_working();
    thread ct1(&Client::run, new Client("Alice", &channel));
    thread ct2(&Client::run, new Client("Bobby", &channel));
    thread ct3(&Client::run, new Client("Chris", &channel));

    std::this_thread::sleep_for(milliseconds(60));

    for (int i = 0; i < workers.second; ++i)
        workers.first[i]->join();
    ct1.join();
    ct2.join();
    ct3.join();

    cout << endl << "Bye Worker Thread pattern..." << endl;
    return 0;
}

