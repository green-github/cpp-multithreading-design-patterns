// immutable.cpp

#include <cstring>

#include <iostream>
#include <thread>


namespace cmdp {


class Person {

  private:
    const char* __name_;
    const char* __address_;

  public:
    Person(const char* name, const char* address)
        : __name_(strdup(name)), __address_(strdup(address)) {}
    const char* get_name() {
        return __name_;
    }
    const char* get_address() {
        return __address_;
    }
    friend std::ostream& operator<<(std::ostream& os, const Person& person) {
        os << "[ Person: name = " << person.__name_ << ", address = " << person.__address_ << " ]";
        return os;
    }

};


class PrintPerson {

 public:
    void operator()(const Person& person) {
        while (true)
            std::cout << std::this_thread::get_id() << " prints " << person << std::endl;
    }

};


} // end of namespace cmdp


int main() {
    using std::cout;
    using std::endl;
    using std::thread;
    using namespace cmdp;

    Person alice("Alice", "Alaska");

    thread a(PrintPerson(), std::ref(alice));
    thread b(PrintPerson(), std::ref(alice));
    thread c(PrintPerson(), std::ref(alice));

    a.join();
    b.join();
    c.join();

    cout << endl << "Bye Immutable pattern..." << endl;
    return 0;
}

