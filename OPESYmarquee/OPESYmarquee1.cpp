// commands to run:
// g++ -std=c++11 -pthread OPESYmarquee1.cpp -o OPESYmarquee1
// .\OPESYmarquee1.exe

#include <iostream>
#include <cstdio>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <windows.h>

using namespace std;

// Shared state
atomic<bool> marquee_running{false};   // whether marquee is on/off
atomic<int> marquee_speed{200};        // speed in milliseconds
string marquee_text = "Hello, OPESY!"; // default text for marquee
mutex text_mutex;                      // avoid race conditions

atomic<bool> is_running{true};
atomic<bool> input_active{false};
int PROMPT_ROW;

// helper: move cursor to x,y
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void marquee_display() {
    while (is_running) {
        if (!input_active) { // only update if not typing
            string text;
            {
                lock_guard<mutex> lock(text_mutex);
                text = marquee_running ? marquee_text : "Stopped";
            }
        }

        this_thread::sleep_for(chrono::milliseconds(200));
    }
}

void marquee_logic() {
    while (is_running) {
        if (marquee_running) {
            string temp;
            {
                lock_guard<mutex> lock(text_mutex);
                temp = marquee_text + "   ";
            }
            for (size_t i = 0; i < temp.size() && is_running && marquee_running; i++) {
                string scrolled = temp.substr(i) + temp.substr(0, i);
                {
                    lock_guard<mutex> lock(text_mutex);
                    marquee_text = scrolled;
                }
                this_thread::sleep_for(chrono::milliseconds(marquee_speed));
            }
        } else {
            this_thread::sleep_for(chrono::milliseconds(200));
        }
    }
}

void input() {
    string cmd;
    while (is_running) {
        input_active = true;   // pause marquee updates
        gotoxy(10, PROMPT_ROW); // cursor goes after "Command > "
        getline(cin, cmd);
        input_active = false;  // resume marquee updates

        if(cmd == "help"){
            cout << "Available commands:\n"
                 << " help            - Show this help menu\n"
                 << " start_marquee   - Start marquee animation\n"
                 << " stop_marquee    - Stop marquee animation\n"
                 << " set_text        - Change marquee text\n"
                 << " set_speed       - Change marquee speed (ms)\n"
                 << " exit            - Quit program\n";
        }
        else if (cmd == "start_marquee") {
            marquee_running = true;
            cout << "Marquee started.\n";
        }
        else if (cmd == "stop_marquee") {
            marquee_running = false;
            cout << "Marquee stopped.\n";
        }
        else if (cmd == "set_text") {
            cout << "Enter new marquee text: ";
            string new_text;
            getline(cin, new_text);
            lock_guard<mutex> lock(text_mutex);
            marquee_text = new_text;
            cout << "Text updated!\n";
        }
        else if (cmd == "set_speed") {
            cout << "Enter speed in milliseconds: ";
            int new_speed;
            cin >> new_speed;
            cin.ignore(); // clear buffer
            marquee_speed = new_speed;
            cout << "Speed updated to " << new_speed << " ms.\n";
        }
        else if (cmd == "exit") {
            cout << "Exiting program. Goodbye!\n";
            is_running = false;
            marquee_running = false;
            break;
        }
        else {
            cout << "Unknown command. Type 'help' for list.\n";
        }
    }
}

int getCursorRow() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.dwCursorPosition.Y;
}

int main()
{
    cout << "Welcome to CSOPESY!" << endl;
    cout << "Group Developers:" << endl;
    cout << "Aquino, Mark Daniels" << endl;
    cout << "Corpuz, Hannah Francesca" << endl;
    cout << "Sena, Sophia Pauline" << endl;
    cout << "Suizo, Danica Marie" << endl;
    cout << "Version Date: September 30, 2025" << endl << endl;

    cout << "   _____  _____  ____  _____  ______  _______     __" << endl;
    cout << "  / ____|/ ____|/ __ \\|  __ \\|  ____|/ ____\\ \\   / /" << endl;
    cout << " | |    | (___ | |  | | |__) | |__  | (___  \\ \\_/ / " << endl;
    cout << " | |     \\___ \\| |  | |  ___/|  __|  \\___ \\  \\   /  " << endl;
    cout << " | |____ ____) | |__| | |    | |____ ____) |  | |   " << endl;
    cout << "  \\_____|_____/ \\____/|_|    |______|_____/   |_|   " << endl;
    cout << endl;

    // reserve prompt row (now guaranteed to be below marquee row)
    PROMPT_ROW = getCursorRow();
    cout << "Command > " << endl; 

    // Threads: one updates marquee line, one scrolls text, one reads input
    thread disp(marquee_display);
    thread logic(marquee_logic);
    thread in(input);

    in.join();
    logic.join();
    disp.join();
}