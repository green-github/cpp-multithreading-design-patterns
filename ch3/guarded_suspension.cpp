// guarded_suspension.cpp

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>


namespace cmdp {


using std::string;
using std::ostream;
using std::queue;
using std::unique_lock;
using std::lock_guard;
using std::condition_variable;
using std::mutex;
using std::chrono::milliseconds;
using std::cout;
using std::endl;


class Request {

  private:
    string __name_;

  public:
    Request(string name) : __name_(name) {}
    string name() const { return __name_; }
    friend ostream& operator<<(ostream& os, const Request& req) {
        os << "REQ-" << req.__name_ << endl;
        return os;
    }

};


class Request_Queue {

  private:
    queue<Request*> __requests_;
    mutex __mtx_;
    condition_variable __cv_;

  public:
    ~Request_Queue() {
        while (!__requests_.empty()) {
            delete __requests_.front();
            __requests_.pop();
        }
    }
    Request* get_request() {
        unique_lock<mutex> lk(__mtx_);
        __cv_.wait(lk, [this] { return !__requests_.empty(); });
        Request* ptr = __requests_.front();
        __requests_.pop();
        return ptr;
    }
    void put_request(Request* ptr) {
        lock_guard<mutex> lk(__mtx_);
        __requests_.push(ptr);
        __cv_.notify_all();
    }

};


class Client {

  private:
    string __name_;
    Request_Queue* __rq_;

  public:
    Client(string nm, Request_Queue* rq) : __name_(nm), __rq_(rq) {}
    void run() {
        for (int i = 0; i < 1000; ++i) {
            Request* req = new Request(string(__name_ + "#" + std::to_string(i)));
            cout << __name_ << " requests: " << *req;
            __rq_->put_request(req);
            std::this_thread::sleep_for(milliseconds(rand()%1000));
        }
    }

};


class Server {

  private:
    string __name_;
    Request_Queue* __rq_;

  public:
    Server(string nm, Request_Queue* rq) : __name_(nm), __rq_(rq) {}
    void run() {
        for (int i = 0; i < 1000; ++i) {
            Request* req = __rq_->get_request();
            cout << __name_ << " handls: " << *req;
            std::this_thread::sleep_for(milliseconds(rand()%1000));
            delete req;
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

    Request_Queue rq;
    Client c1("Alice", &rq);
    Client c2("Bobby", &rq);
    Server s1("Chris", &rq);

    thread ct1(&Client::run, &c1);
    thread ct2(&Client::run, &c2);
    thread st1(&Server::run, &s1);

    st1.join();
    ct1.join();
    ct2.join();

    cout << endl << "Bye Guarded Suspension pattern..." << endl;
    return 0;
}

