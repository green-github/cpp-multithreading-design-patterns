// active_object.cpp

#include <cstdlib>
#include <cstring>

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <future>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>


namespace cmdp {


using std::chrono::milliseconds;
using std::condition_variable;
using std::cout;
using std::endl;
using std::future;
using std::lock_guard;
using std::mutex;
using std::string;
using std::thread;
using std::unique_lock;


class Result {

  private:
    char* __v_;

  public:
    Result(int count_, char filler_) {
        __v_ = new char[count_+1];
        std::memset(__v_, filler_, count_);
        __v_[count_] = '\0';
    }
    ~Result() {
        delete[] __v_;
    }
    const char* get_value() { return __v_; }

};


class Active_Object_IF {

  public:
    virtual Result* make_string(int count_, char filler_) = 0;
    virtual void display_string(string string_) = 0;

};


class Servant : public Active_Object_IF {

  public:
    Result* make_string(int count_, char filler_) {
        return new Result(count_, filler_);
    }
    void display_string(string string_) {
        cout << string_ + "\n";
    }

};


class Method_Request {

  protected:
    Servant* const __servant_;

  public:
    Method_Request(Servant* servant_) : __servant_(servant_) {}
    virtual ~Method_Request() {}
    virtual void execute() = 0;

};


class Make_String_Request : public Method_Request {

  private:
    int const __count_;
    char const __filler_;
    Result* __result_;
    mutex __mtx_;
    condition_variable __cv_;

  public:
    Make_String_Request(Servant* servant_, int count_, char filler_)
        : Method_Request(servant_), __result_(nullptr), __count_(count_), __filler_(filler_) {}
    virtual ~Make_String_Request() {}
    Result* get_result() {
        unique_lock<mutex> lk_(__mtx_);
        __cv_.wait(lk_, [this] { return __result_ != nullptr; });
        return __result_;
    }
    void execute() {
        lock_guard<mutex> lk_(__mtx_);
        __result_ = __servant_->make_string(__count_, __filler_);
        __cv_.notify_one();
    }

};


class Display_String_Request : public Method_Request {

  private:
    string __message_;

  public:
    Display_String_Request(Servant* servant_, string message_)
        : Method_Request(servant_), __message_(message_) {}
    virtual ~Display_String_Request() {}
    void execute() {
        __servant_->display_string(__message_);
    }

};


class Activation_Queue {

  private:
    // data member of static int can be initialized within class definition
    static int const __MAX_METHOD_REQUEST__ = 100;
    Method_Request** const __requests_;
    size_t __count_;
    size_t __tail_;
    size_t __head_;
    mutex __mtx_;
    condition_variable __cv_;

  public:
    Activation_Queue()
        :__requests_(new Method_Request*[__MAX_METHOD_REQUEST__]), __tail_(0), __head_(0), __count_(0) {}
    ~Activation_Queue() {
        for (size_t i_ = 0; i_ < __count_; ++i_)
            delete __requests_[i_];
        delete[] __requests_;
    }
    void put_request(Method_Request* request_) {
        unique_lock<mutex> lk_(__mtx_);
        __cv_.wait(lk_, [this] { return __count_ < __MAX_METHOD_REQUEST__; });
        if (__requests_[__tail_])                                 // avoid Segmentation fault: 11
            delete __requests_[__tail_];
        __requests_[__tail_] = request_;
        __tail_ = (__tail_ + 1) % __MAX_METHOD_REQUEST__;
        ++__count_;
        __cv_.notify_all();
    }
    Method_Request* take_request() {
        unique_lock<mutex> lk_(__mtx_);
        __cv_.wait(lk_, [this] { return __count_ > 0; });
        Method_Request* request_ = __requests_[__head_];
        __requests_[__head_] = nullptr;                           // avoid memory leak
        __head_ = (__head_ + 1) % __MAX_METHOD_REQUEST__;
        --__count_;
        __cv_.notify_all();
        return request_;
    }

};
//int const Activation_Queue::__MAX_METHOD_REQUEST__ = 100;


class Scheduler {

  private:
    Activation_Queue* const __aq_;

  public:
    Scheduler() : __aq_(new Activation_Queue()) {}
    ~Scheduler() {
        delete __aq_;
    }
    Result* invoke(Method_Request* request_) {
        __aq_->put_request(request_);
        Result* result_;
        Make_String_Request* msr_;
        if ((msr_ = dynamic_cast<Make_String_Request*>(request_))) {
            result_ = msr_->get_result();
        } else {
            result_ = nullptr;
        }
        return result_;
    }
    void run() {
        while (true) {
            Method_Request* request_ = __aq_->take_request();
            request_->execute();
        }
    }

};


class Proxy : public Active_Object_IF {

  private:
    Servant __servant_;
    Scheduler __scheduler_;
    thread* __runner_;

  public:
    Proxy() {
        __runner_ = new thread(&Scheduler::run, &__scheduler_);
    }
    ~Proxy() {
        __runner_->join();
    }
    Result* make_string(int count_, char filler_) {
        future<Result*> f_ = std::async(std::launch::async, &Scheduler::invoke, &__scheduler_,
            new Make_String_Request(&__servant_, count_, filler_));
        return f_.get();
    }
    void display_string(string message_) {
        __scheduler_.invoke(new Display_String_Request(&__servant_, message_));
    }

};


class Maker_Client {

  private:
    string const __name_;
    Active_Object_IF* const __ao_;

  public:
    Maker_Client(string name_, Active_Object_IF* ao_) : __name_(name_), __ao_(ao_) {}
    void run() {
        for (int i_ = 1; true; ++i_) {
            Result* r_ = __ao_->make_string(i_ % 80, __name_[0]);
            cout << __name_ + ": " + r_->get_value() + "\n";
            delete r_;
            std::this_thread::sleep_for(milliseconds(rand() % 100));
        }
    }

};


class Display_Client {

  private:
    string const __name_;
    Active_Object_IF* const __ao_;

  public:
    Display_Client(string name_, Active_Object_IF* ao_) : __name_(name_), __ao_(ao_) {}
    void run() {
        for (int i_ = 1; true; ++i_) {
            string s(__name_ + ": " + std::to_string(i_));
            __ao_->display_string(s);
            std::this_thread::sleep_for(milliseconds(rand() % 200));
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

    Active_Object_IF* ao_ = new Proxy();

    Maker_Client alice("Alice", ao_);
    Maker_Client bobby("Bobby", ao_);
    Display_Client chris("Chris", ao_);

    thread t1(&Maker_Client::run, alice);
    thread t2(&Maker_Client::run, bobby);
    thread t3(&Display_Client::run, chris);

    t1.join();
    t2.join();
    t3.join();

    cout << endl << "Bye Active Object pattern..." << endl;
    return 0;
}

