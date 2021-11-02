// balking.cpp


#include <cstdlib>

#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>


namespace cmdp {


using std::chrono::milliseconds;
using std::cout;
using std::endl;
using std::lock_guard;
using std::mutex;
using std::ofstream;
using std::string;


class Data {

  private:
    string __filename_;
    string __content_;
    bool __changed_;
    mutex __mtx_;

    void __do_save() {
        cout << std::this_thread::get_id()
             << " calls __do_save, contents = " + __content_ + "\n";
        ofstream ofs(__filename_);
        ofs << __content_;
        ofs.close();
    }

  public:
    Data(string filename, string content)
        : __filename_(filename), __content_(content), __changed_(true) {}
    void change(string newcontent) {
        lock_guard<mutex> lk(__mtx_);
        __content_ = newcontent;
        __changed_ = true;
    }
    void save() {
        lock_guard<mutex> lk(__mtx_);
        if (!__changed_)
            return;
        __do_save();
        __changed_ = false;
    }

};


class Saver {

  private:
    Data* __data_;

  public:
    Saver(Data* data) : __data_(data) {}
    void run() {
        cout << "Saver: " << std::this_thread::get_id() << endl;
        while (true) {
            std::this_thread::sleep_for(milliseconds(rand()%1000));
            __data_->save();
        }
    }

};


class Changer {

  private:
    Data* __data_;

  public:
    Changer(Data* data) : __data_(data) {}
    void run() {
        std::this_thread::sleep_for(milliseconds(100));
        cout << "Changer: " << std::this_thread::get_id() << endl;
        for (int i = 0; true; ++i) {
            string s("No.");
            s += std::to_string(i);
            __data_->change(s);
            std::this_thread::sleep_for(milliseconds(rand()%1000));
            __data_->save();
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

    Data data("data.txt", "(empty)");
    Changer c(&data);
    Saver   s(&data);

    thread t1(&Changer::run, &c);
    thread t2(&Saver::run, &s);

    t1.join();
    t2.join();

    cout << endl << "Bye Balking pattern..." << endl;
    return 0;
}

