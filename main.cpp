#include<iostream>
#include<iomanip>
#include<cstring>
#include <vector>
#include <set>
#include <algorithm>
#include <cassert>

using std::setw;
using namespace std;

enum __algo_t {
    fifo,
    second_chance,
    WSet,
    WSetImp,
    WIP
};
typedef enum __algo_t algo_t;

vector<string> split(string str) {
    vector<string> splitted;

    string word = "";
    for (auto x: str) {
        if (x == ' ') {
            splitted.push_back(word);
            word = "";
        } else {
            word = word + x;
        }
    }
    splitted.push_back(word);

    return splitted;
}

// If page found, updates the second chance bit to true
static bool findAndUpdate_sc(int x, int arr[], bool second_chance[], int frames) {
    int i;
    for (i = 0; i < frames; i++) {
        if (arr[i] == x) {
            // Mark that the page deserves a second chance
            second_chance[i] = true;
            // Return 'true', that is there was a hit
            // and so there's no need to replace any page
            return true;
        }
    }
    // Return 'false' so that a page for replacement is selected
    // as he reuested page doesn't exist in memory
    return false;
}

// If page found return true, else false.
static bool findAndUpdate_fifo(int x, int arr[], int frames) {
    int i;
    for (i = 0; i < frames; i++) {
        if (arr[i] == x) {
            // Return 'true', that is there was a hit
            // and so there's no need to replace any page
            return true;
        }
    }
    // Return 'false' so that a page for replacement is selected
    // as he reuested page doesn't exist in memory
    return false;
}


// Updates the page in memory and returns the pointer
static int replaceAndUpdate_sc(int x, int arr[], bool second_chance[], int frames, int pointer) {
    while (true) {
        // We found the page to replace
        if (!second_chance[pointer]) {
            // Replace with new page
            arr[pointer] = x;

            // Return updated pointer
            return (pointer + 1) % frames;
        }

        // Mark it 'false' as it got one chance
        // and will be replaced next time unless accessed again
        second_chance[pointer++] = false;

        //Pointer is updated in round robin manner
        pointer %= frames;
    }
}

// Updates the page in memory and returns the pointer as second chanc algo, but degenerate to fifo
static int replaceAndUpdate_fifo(int x, int arr[], int frames, int pointer) {
    //we will ALWAYS pdate the first inserted frame
    arr[pointer] = x;
    // Return updated pointer
    return (pointer + 1) % frames;
}

static void printHitsAndFaults_sc(string reference_string, int frames) {
    int pointer, i = 0, x, pf;
    //initially we consider frame 0 is to be replaced
    pointer = 0;

    //number of page faults
    pf = 0;

    // Create a array to hold page numbers
    int arr[frames];

    // No pages initially in frame,
    // which is indicated by -1
    memset(arr, -1, sizeof(arr));

    // Create second chance array.
    // Can also be a byte array for optimizing memory
    bool second_chance[frames];
    for (int i = 0; i < frames; i++)
        second_chance[i] = false;

    //tokenize the string
    vector<string> splitted = split(reference_string);

    //do second chance algo
    for (auto str: splitted) {
        i++;
        bool lpf = false;
        x = stoi(str);
        // Finds if there exists a need to replace
        // any page at all
        if (!findAndUpdate_sc(x, arr, second_chance, frames)) {
            // Selects and updates a victim page
            pointer = replaceAndUpdate_sc(x, arr, second_chance, frames, pointer);
            // Update page faults
            pf++;
            lpf = true;
        }

        //print memory at SC(RAM,PSI(R_alpha,i)) and residual pointer
        cout << "[i:" << setw(2) << i << "]\tPSI(R_alpha,i) :: <" << setw(2) << x << ">\t|";
        for (int k = 0; k < frames; k++)
            cout << setw(2) << arr[k] << " | ";
        cout << (lpf ? "\tPF " : "\t") << "\tptr:" << pointer << "\n";
    }
    cout << "Total page faults were " << pf << "\n";
}

void printHitsAndFaults_fifo(string reference_string, int frames) {
    int pointer, i = 0, x, pf;
    //initially we consider frame 0 is to be replaced
    pointer = 0;
    //number of page faults
    pf = 0;
    // Create a array to hold page numbers
    int arr[frames];

    // No pages initially in frame,
    // which is indicated by -1
    memset(arr, -1, sizeof(arr));
    //tokenize the string
    vector<string> splitted = split(reference_string);

    //do fifo algo
    for (auto str: splitted) {
        bool lpf = false;
        i++;
        x = stoi(str);

        // Finds if there exists a need to replace
        // any page at all
        if (!findAndUpdate_fifo(x, arr, frames)) {
            // Selects and updates a victim page
            pointer = replaceAndUpdate_fifo(x, arr, frames, pointer);
            // Update page faults
            pf++;
            lpf = true;
        }

        //print memory at SC(RAM,PSI(R_alpha,i)) and residual pointer
        cout << "[i:" << setw(2) << i << "]\tPSI(R_alpha,i) :: <" << setw(2) << x << ">\t|";
        for (int k = 0; k < frames; k++)
            cout << setw(2) << arr[k] << " | ";
        cout << (lpf ? "\tPF " : "\t") << "\tptr:" << pointer << "\n";
    }
    cout << "Total page faults were " << pf << "\n";
}

void printWSFlow(string reference_string, int frames) {
    //page fault counter
    int pf = 0;
    //splitted reference string
    vector<string> splitted = split(reference_string);
    //moment counter
    int i = 0;
    //old set to see if some page need to swap/make a page fault
    set<string> oldWSSet;

    for (auto rf: splitted) {
        set<string> WSSet;  //actual WS
        for (int k = 0; k < frames; k++) {  //make new WS
            WSSet.insert(i - k < 0 ? splitted.at(0) : splitted.at(i - k));
        }

        //print WS
        cout << "[i:" << setw(2) << i + 1 << "]\tPSI(R_alpha,i) :: <" << setw(2) << rf << ">\t| ";
        int limit = 0;
        set<string>::iterator ws_el;
        for (ws_el = WSSet.begin(); ws_el != WSSet.end(); ws_el++) {
            cout << *ws_el << " | ";
            limit++;
        }

        for (; limit < frames; limit++) {
            cout << "\t";
        }

        //calculate if there is apage fault
        if (oldWSSet.empty() || oldWSSet != WSSet) {
            if (oldWSSet.size() <= WSSet.size()) {
                cout << "\tPF";
                pf++;
            }
        }
        oldWSSet = WSSet;
        i++;
        cout << "\n";
    }
    cout << "Total page faults were " << pf << "\n";
}

bool replaceAndUpdate_WSimproved(int arr[], int frames, set<int> WSet, set<int> oldWSet) {
    bool find[frames];
    set<int> diff;

    for (int i = 0; i < frames; i++)
        find[i] = false;

    //find if there's that page in i-position
    for (auto page: WSet) {
        for (int i = 0; i < frames; i++) {
            if (arr[i] == page) {
                find[i] = true;
            }
        }
    }

    //if there is not a refereced frame in the WS, swap-out it.
    for (int i = 0; i < frames; i++) {
        if (!find[i]) {
            arr[i] = -1;
            break;
        }
    }

    //get the difference from pages at moment (i-1) and (i);
    set_difference(WSet.begin(), WSet.end(), oldWSet.begin(),
                   oldWSet.end(), std::inserter(diff, diff.begin()));

    assert(diff.size() == 0 || diff.size() == 1);       //assert: we have always al least 1 page in diff
    if (!diff.empty()) {
        int npage = *diff.begin();                          //get  the page

        //replace the page
        for (int i = 0; i < frames; i++) {
            if (arr[i] == -1) {
                arr[i] = npage;
                return true;
            }
        }
    }
    return false;
}

void printHitsAndFaults_WSimproved(string reference_string, int frames) {
    //page fault counter
    int pf = 0;
    //splitted reference string
    vector<string> splitted = split(reference_string);
    //moment counter
    int i = 0;
    //old set to see if some page need to swap/make a page fault
    set<int> oldWSSet;
    //memory set
    int arr[frames];
    //init all memory to -1 (no page in memory)
    memset(arr, -1, sizeof(arr));

    for (auto rf: splitted) {
        set<int> WSSet;  //actual WS
        bool lpf = false;   //have a PF is false for invariant;
        bool ntd = true;    //we assume haven't to do anything...

        if (!findAndUpdate_fifo(stoi(rf), arr, frames)) {
            //calculating WS(i,frames)
            for (int k = 0; k < frames; k++) {
                WSSet.insert(stoi(i - k < 0 ? splitted.at(0) : splitted.at(i - k)));
            }
            //update the ram
            lpf = replaceAndUpdate_WSimproved(arr, frames, WSSet, oldWSSet);
            //save the old WS
            oldWSSet = WSSet;
            if (lpf) {
                pf++;
            }
        } else {    //calculate if the WS is decreased...
            set<int> WSSet;
            for (int k = 0; k < frames; k++) {
                WSSet.insert(stoi(i - k < 0 ? splitted.at(0) : splitted.at(i - k)));
            }
            if (WSSet.size() - oldWSSet.size() > 0)
                ntd = false;
        }
        i++;

        //print memory at SC(RAM,PSI(R_alpha,i))
        cout << "[i:" << setw(2) << i << "]\tPSI(R_alpha,i) :: <" << setw(2) << rf << ">\t|";
        for (int k = 0; k < frames; k++) {
            cout << setw(2) << arr[k] << " | ";
        }
        cout << (lpf ? "\tPF " : "");
        cout << (!ntd ? "\tWS decreased " : "");
        cout << "\n";
    }
    cout << "Total page faults were " << pf << "\n";
    cout << "\n";
}


bool replaceAndUpdate_WS(int arr[], int frames, set<int> WSet, set<int> oldWSet) {
    bool find[frames];
    set<int> diff;

    for (int i = 0; i < frames; i++)
        find[i] = false;

    //find if there's that page in i-position
    for (auto page: WSet) {
        for (int i = 0; i < frames; i++) {
            if (arr[i] == page) {
                find[i] = true;
            }
        }
    }

    //if there is not a refereced frame in the WS, swap-out it.
    for (int i = 0; i < frames; i++) {
        if (!find[i]) {
            arr[i] = -1;
        }
    }

    //get the difference from pages at moment (i-1) and (i);
    set_difference(WSet.begin(), WSet.end(), oldWSet.begin(),
                   oldWSet.end(), std::inserter(diff, diff.begin()));

    assert(diff.size() == 0 || diff.size() == 1);       //assert: we have always al least 1 page in diff
    if (!diff.empty()) {
        int npage = *diff.begin();                          //get  the page

        //replace the page
        for (int i = 0; i < frames; i++) {
            if (arr[i] == -1) {
                arr[i] = npage;
                return true;
            }
        }
    }
    return false;
}

void printHitsAndFaults_WS(string reference_string, int frames) {
    //page fault counter
    int pf = 0;
    //splitted reference string
    vector<string> splitted = split(reference_string);
    //moment counter
    int i = 0;
    //old set to see if some page need to swap/make a page fault
    set<int> oldWSSet;
    //memory set
    int arr[frames];
    //init all memory to -1 (no page in memory)
    memset(arr, -1, sizeof(arr));

    for (auto rf: splitted) {
        set<int> WSSet;  //actual WS
        bool lpf = false;   //have a PF is false for invariant;

        //calculating WS(i,frames)
        for (int k = 0; k < frames; k++) {
            WSSet.insert(stoi(i - k < 0 ? splitted.at(0) : splitted.at(i - k)));
        }
        //update the ram
        lpf = replaceAndUpdate_WS(arr, frames, WSSet, oldWSSet);
        //save the old WS
        oldWSSet = WSSet;
        if (lpf) {
            pf++;
        }

        i++;

        //print memory at SC(RAM,PSI(R_alpha,i))
        cout << "[i:" << setw(2) << i << "]\tPSI(R_alpha,i) :: <" << setw(2) << rf << ">\t|";
        for (int k = 0; k < frames; k++) {
            cout << setw(2) << arr[k] << " | ";
        }
        cout << (lpf ? "\tPF " : "");
        cout << "\n";
    }
    cout << "Total page faults were " << pf << "\n";
    cout << "\n";
}

int main(int argc, char *argv[]) {

    algo_t algo = fifo;
    string reference_string = "1 2 3 4 2 1 5 6 2 1 2 3 7 6 3 2 1 2 3 6";
    int frames = 4;

    if (argc > 1) {

        if (strcmp(argv[1], "--help") == 0) {
            cout << "Usage:\n"
                    "\t - arg1: algorithm to applicate: {fifo,second_chance, WS, WS_imp}\n"
                    "\t - arg2: number of frames\n"
                    "\t - arg3: reference string\n\n"
                    "example: ./memory_algo second_chance 4 \"1 2 3 4 2 1 5 6 2 1 2 3 7 6 3 2 1 2 3 6\"\n";
            return 0;
        }

        if (strcmp(argv[1], "fifo") == 0)
            algo = fifo;
        else if (strcmp(argv[1], "second_chance") == 0)
            algo = second_chance;
        else if (strcmp(argv[1], "WS") == 0)
            algo = WSet;
        else if (strcmp(argv[1], "WS_imp") == 0)
            algo = WSetImp;
        else
            algo = WIP;

        frames = stoi(argv[2]);
        reference_string = argv[3];
    }

    if (algo == second_chance)
        printHitsAndFaults_sc(reference_string, frames);
    if (algo == fifo)
        printHitsAndFaults_fifo(reference_string, frames);
    if (algo == WSet)
        printHitsAndFaults_WS(reference_string, frames);
    if (algo == WSetImp)
        printHitsAndFaults_WSimproved(reference_string, frames);

    if (algo == WIP)
        cout << "WIP, not implemented yet!\n";


    return 0;
}