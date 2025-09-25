#include <iostream>
#include <thread>
#include <string>
#include <chrono>

using namespace std;

void marquee(const string& msg, bool& done) {
    size_t pos = 0;
    string lastMsg;
    string marqMsg;
    while (true) {
        //reset marquee string and position
        if (msg != lastMsg) {
            lastMsg = msg;
            marqMsg = msg + "   "; 
            pos = 0;
        }
        if (1 < marqMsg.size()) {
            string marquee = marqMsg.substr(pos) + marqMsg.substr(0, pos);
            cout << "\r" << string(100, ' ') << "\r";
            cout << marquee << flush;
            pos = (pos + 1) % marqMsg.size();
        }
        if (done) break;
        this_thread::sleep_for(chrono::milliseconds(500));
    }
    cout << endl;
}

void input(string& msg, bool& done) {
    string input;
    while (true) {
        getline(cin, input);
        cout << "\033[F";
        msg = input;
        if (input == "exit") {
            done = true;
            break;
        }
    }
}

int main() {
    string msg;
    bool done = false;
    thread t1(marquee, ref(msg), ref(done));
    thread t2(input, ref(msg), ref(done));
    t2.join();
    t1.join();
    return 0;
}