#include <iostream>
#include <cstdio>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>

using namespace std;

atomic<char> input_char();
atomic<bool> is_running{ true };

void display() {
    string s;
    while (true) {

        //Display Welcome Message + Creator namespace

        //Display command area

        //Display Marquee

        cout << "Display Here" << endl;
    }
}

void marquee() {
    string s;
    while (true) {
        //Marquee animation and text here

        cout << "Marquee Here" << endl;


    }
}

void input() {
    string s;
    while (true) {

        /*
        Command Interpreter:
            help -              Display commands and its description
            start_marquee -     Starts marquee animation
            stop_marquee -      Stops marquee animation
            set_text -          Accepts a text input and displays it as a marquee
            set_speed -         Sets the marquee animation refresh in miliseconds
            exit -              Terminates the console
        */

        cout << "Input Here" << endl;

    }

}

int main()
{

    thread marqueeLogic(marquee);
    thread inputLogic(input);
    thread toDisplay(display);

    //Keyboard handler to be put here


    inputLogic.join();
    marqueeLogic.join();
    toDisplay.join();
    //Ahh so it doesn't matter which one is called first, all of them display at the same time
    //This is why it's not garuantee as to how it will display in the console


    cout << "Hello World";

    return 0;
}