#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <algorithm>
#ifdef _WIN32
  #include <windows.h>
#else
  #include <sys/ioctl.h>
  #include <unistd.h>
#endif

// =====================
// Shared state (globals)
// =====================
std::string marquee_text = "Hello, Marquee!";     // Default marquee text
std::atomic<int> marquee_speed{150};              // Refresh speed in milliseconds
std::atomic<bool> marquee_running{false};         // Controls start/stop state
std::atomic<bool> marquee_thread_alive{false};    // Indicates if marquee thread is alive
std::mutex text_mutex;                            // Protects access to shared marquee_text
std::condition_variable cv;                       // Used to notify marquee thread of changes
std::thread marquee_thread;                       // The thread that runs the marquee



// Get the width of the terminal window; fallback = 80
int get_terminal_width() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE || !GetConsoleScreenBufferInfo(h, &csbi))
        return 80;
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) return 80;
    return w.ws_col > 0 ? w.ws_col : 80;
#endif
}

// Enable ANSI escape sequences on Windows (for colors and cursor control)
void enable_ansi_on_windows() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}


// Command list (help)
void print_help() {
    std::cout << "Available commands:\n"
              << "  help                 : displays the commands and its description\n"
              << "  start_marquee        : starts the marquee “animation”\n"
              << "  stop_marquee         : stops the marquee “animation”\n"
              << "  set_text <text>      : accepts a text input and displays it as a marquee\n"
              << "  set_speed <ms>       : sets the marquee animation refresh in milliseconds\n"
              << "  exit                 : terminates the console\n";
}


// Marquee thread logic
void marquee_loop() {
    marquee_thread_alive = true;
    std::unique_lock<std::mutex> lk(text_mutex, std::defer_lock);

    while (marquee_running) {
        // Lock once to safely read current text and speed
        lk.lock();
        std::string text = marquee_text;
        int speed = marquee_speed.load();
        lk.unlock();

        if (text.empty()) text = " "; // Prevent blank crashes

        int width = get_terminal_width();
        std::string pad(width, ' ');

        // Create a long buffer for smooth wrapping (text + padding + text + padding)
        std::string longbuf = text + pad + text + pad;
        int pos = 0;
        int maxPos = static_cast<int>(longbuf.size());

        // Scroll across the buffer
        for (; marquee_running && pos < maxPos; ++pos) {
            if (!marquee_running) break;

            std::string slice;
            if (width <= 0) width = 80;  // Default width
            if (pos + width <= maxPos) {
                slice = longbuf.substr(pos, width);
            } else {
                // Wrap-around slice
                int part1 = maxPos - pos;
                slice = longbuf.substr(pos, part1) + longbuf.substr(0, width - part1);
            }

            // Print current slice, overwrite same line using \r
            std::cout << "\r" << slice << std::flush;

            // Wait for given time or until a change happens
            std::unique_lock<std::mutex> wait_lk(text_mutex);
            cv.wait_for(wait_lk, std::chrono::milliseconds(speed), []{ return !marquee_running.load(); });
            wait_lk.unlock();

            if (!marquee_running) break;
        }
    }

    // Clear the line when marquee stops
    std::cout << "\r" << std::string(get_terminal_width(), ' ') << "\r" << std::flush;
    marquee_thread_alive = false;
}


void start_marquee_cmd() {
    if (marquee_running) {
        std::cout << "Marquee is already running.\n";
        return;
    }
    marquee_running = true;
    cv.notify_all();

    // Create new thread if not already alive
    if (!marquee_thread.joinable() || !marquee_thread_alive.load()) {
        marquee_thread = std::thread([]{
            enable_ansi_on_windows();
            marquee_loop();
        });
    }
    std::cout << "Marquee started.\n";
}

void stop_marquee_cmd() {
    if (!marquee_running) {
        std::cout << "Marquee is not running.\n";
        return;
    }
    marquee_running = false;
    cv.notify_all();
    if (marquee_thread.joinable()) {
        marquee_thread.join(); // Wait for graceful shutdown
    }
    std::cout << "Marquee stopped.\n";
}


int main() {
    enable_ansi_on_windows();

    // Display group banner & ASCII art
    std::cout << "Welcome to CSOPESY!" << std::endl;
    std::cout << "Group Developers:" << std::endl;
    std::cout << "Aquino, Mark Daniels" << std::endl;
    std::cout << "Corpuz, Hannah Francesca" << std::endl;
    std::cout << "Sena, Sophia Pauline" << std::endl;
    std::cout << "Suizo, Danica Marie" << std::endl;
    std::cout << "Version Date: September 30, 2025" << std::endl << std::endl;

    std::cout << "   _____  _____  ____  _____  ______  _______     __" << std::endl;
    std::cout << "  / ____|/ ____|/ __ \\|  __ \\|  ____|/ ____\\ \\   / /" << std::endl;
    std::cout << " | |    | (___ | |  | | |__) | |__  | (___  \\ \\_/ / " << std::endl;
    std::cout << " | |     \\___ \\| |  | |  ___/|  __|  \\___ \\  \\   /  " << std::endl;
    std::cout << " | |____ ____) | |__| | |    | |____ ____) |  | |   " << std::endl;
    std::cout << "  \\_____|_____/ \\____/|_|    |______|_____/   |_|   " << std::endl;
    std::cout << std::endl;

    // Print initial help menu
    print_help();

    std::string line;
    while (true) {
        std::cout << "\n> " << std::flush;
        if (!std::getline(std::cin, line)) break; // Exit if input ends (Ctrl+D)

        // Trim whitespace from input
        auto ltrim = [](std::string &s){
            s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                [](unsigned char ch){ return !std::isspace(ch); }));
        };
        auto rtrim = [](std::string &s){
            s.erase(std::find_if(s.rbegin(), s.rend(),
                [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
        };
        ltrim(line); rtrim(line);
        if (line.empty()) continue;

        // Split into command + arguments
        std::string cmd, rest;
        size_t spacePos = line.find(' ');
        if (spacePos == std::string::npos) {
            cmd = line;
        } else {
            cmd = line.substr(0, spacePos);
            rest = line.substr(spacePos + 1);
            ltrim(rest); rtrim(rest);
        }

        // Handle commands
        if (cmd == "help") {
            print_help();
        } else if (cmd == "start_marquee") {
            start_marquee_cmd();
        } else if (cmd == "stop_marquee") {
            stop_marquee_cmd();
        } else if (cmd == "set_text") {
            // If no inline argument, prompt interactively
            if (rest.empty()) {
                std::cout << "Enter new marquee text: ";
                std::getline(std::cin, rest);
            }
            if (!rest.empty()) {
                {
                    std::lock_guard<std::mutex> lk(text_mutex);
                    marquee_text = rest;
                }
                cv.notify_all();
                std::cout << "Marquee text set to: \"" << rest << "\"\n";
            } else {
                std::cout << "No text provided.\n";
            }
        } else if (cmd == "set_speed") {
            // If no inline argument, prompt interactively
            if (rest.empty()) {
                std::cout << "Enter speed in milliseconds: ";
                std::getline(std::cin, rest);
            }
            try {
                int ms = std::stoi(rest);
                if (ms < 10) ms = 10; // Minimum speed limit
                marquee_speed = ms;
                cv.notify_all();
                std::cout << "Marquee speed set to " << ms << " ms\n";
            } catch (...) {
                std::cout << "Invalid speed value.\n";
            }
        } else if (cmd == "exit") {
            break; // Exit loop
        } else {
            std::cout << "Unknown command. Type 'help' to list commands.\n";
        }
    }

    // Cleanup before exit
    if (marquee_running) {
        marquee_running = false;
        cv.notify_all();
    }
    if (marquee_thread.joinable()) {
        marquee_thread.join();
    }

    std::cout << "\nProgram exited.\n";
    return 0;
}