// future.cpp


#include <chrono>
#include <future>
#include <iostream>


namespace cmdp {


using std::chrono::milliseconds;
using std::cout;
using std::endl;
using std::future;
using std::launch;
using std::string;


class Data {

  private:
    string __content_;

  public:
    Data(int count, char c) : __content_(count, c) {}
    string get_content() {
        return __content_;
    }

};


class Host {

  public:
    future<Data> request(int count, char c) {
        cout << "   request(" << count << ", " << c << ") BEGIN\n";
        future<Data> f = std::async(launch::deferred, [=]() -> Data {
            for (int i = 0; i < count; ++i)
                std::this_thread::sleep_for(milliseconds(30));
            return Data(count, c);
        });
        cout << "   request(" << count << ", " << c << ") END\n";
        return f;
    }

};


} // end of namespace cmdp


int main() {
    using std::cout;
    using std::endl;
    using namespace cmdp;

    cout << "main BEGIN\n";
    Host host;

    future<Data> f1 = host.request(30, 'A');
    future<Data> f2 = host.request(40, 'B');
    future<Data> f3 = host.request(50, 'C');
    Data data1 = f1.get();
    Data data2 = f2.get();
    Data data3 = f3.get();

    cout << "data1 = " << data1.get_content() << endl;
    cout << "data2 = " << data2.get_content() << endl;
    cout << "data3 = " << data3.get_content() << endl;
    cout << "main END\n";

    cout << endl << "Bye Future pattern..." << endl;
    return 0;
}

