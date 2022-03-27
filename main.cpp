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
    WSEval,
    WSet,
    WSetImp,
    OPT,
    WIP
};
typedef enum __algo_t algo_t;

vector<string> split(const string &str) {
    vector<string> splitted;

    string word = "";
    for (auto x: str) {
        if (x == ' ') {
            splitted.push_back(word);
            word = "";
        } else {
            word += x;
        }
    }
    splitted.push_back(word);

    return splitted;
}

/////////////////////////// FIFO
/**
 * Check if a page already is in memory
 * @param x the requested frame
 * @param arr the memory space
 * @param frames frames in memory
 * @return true if page found, else false.
 */
static bool findAndUpdate_fifo(int x, const int arr[], int frames) {
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

/**
 * update the page in memory and return the next pointer as fifo prescrition
 * @param x the new requested page
 * @param arr the memory space
 * @param frames numbers of frames in memoty
 * @param pointer the actual victim pointer
 * @return return the next page victim
 */
static int replaceAndUpdate_fifo(int x, int arr[], int frames, int pointer) {
    //we will ALWAYS pdate the first inserted frame
    arr[pointer] = x;
    // Return updated pointer
    return (pointer + 1) % frames;
}

void printHitsAndFaults_fifo(const string &reference_string, int frames) {
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
    for (const auto &str: splitted) {
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

////////////////////////  SECOND CHANCE
/**
 * If page found, updates the second chance bit to true
 * @param x the page
 * @param arr the memory space
 * @param second_chance the second_chance reference bits array
 * @param frames number of frames
 * @return true if found x in arr, false otherwise
 */
static bool findAndUpdate_sc(int x, const int arr[], bool second_chance[], int frames) {
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

/**
 * Updates the page in memory and returns the pointer
 * @param x req frame
 * @param arr memory space
 * @param second_chance the second_chance reference bits array
 * @param frames max frames in memory
 * @param pointer the actual pointer
 * @return next memory pointer
 */
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

/**
 * do the second_chance algorithm
 * @param reference_string
 * @param frames
 */
static void printHitsAndFaults_sc(const string &reference_string, int frames) {
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
    memset(second_chance, false, sizeof(second_chance));

    //tokenize the string
    vector<string> splitted = split(reference_string);

    //do second chance algo
    for (const auto &str: splitted) {
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

//////////////////////////// Working Set
/**
 * show the working set evolution in time
 * @param reference_string
 * @param frames
 */
void printWSFlow(const string &reference_string, int frames) {
    //page fault counter
    int pf = 0;
    //splitted reference string
    vector<string> splitted = split(reference_string);
    //moment counter
    int i = 0;
    //old set to see if some page need to swap/make a page fault
    set<string> oldWSSet;

    for (const auto &rf: splitted) {
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

/**
 * WSet base algo, the only difference is whe do a swapout only if stricted requested.
 * @param arr the memory space
 * @param frames frames in memory
 * @param WSet next WS
 * @param oldWSet actual WS
 * @return true if we swapout a page, false otherwise
 */
bool replaceAndUpdate_WSimproved(int arr[], int frames, const set<int> &WSet, const set<int> &oldWSet) {
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

    assert(diff.empty() || diff.size() == 1);       //assert: we have always al least 1 page in diff
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

/**
 * WS base algo, the diff is we swap-out a page only if necessary
 * @param reference_string
 * @param frames
 */
void printHitsAndFaults_WSimproved(const string &reference_string, int frames) {
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

    for (const auto &rf: splitted) {
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

/**
 * WSet base algo
 * @param arr the memory space
 * @param frames frames in memory
 * @param WSet next WS
 * @param oldWSet actual WS
 * @return true if we swapout a page, false otherwise
 */
bool replaceAndUpdate_WS(int arr[], int frames, const set<int> &WSet, const set<int> &oldWSet) {
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

    assert(diff.empty() || diff.size() == 1);       //assert: we have always al least 1 page in diff
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

/**
 * Working Set base algo
 * @param reference_string
 * @param frames
 */
void printHitsAndFaults_WS(const string &reference_string, int frames) {
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

    for (const auto &rf: splitted) {
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

//////////////////////// OPTimal
/**
 * OPTimal algo to swapping out pages from memory
 * @param reference_string
 * @param frames
 */
void printHitsAndFaults_OPT(const string &reference_string, int frames) {
    int pf = 0;
    int arr[frames];
    memset(arr, -1, sizeof(arr));
    vector<string> splitted = split(reference_string);

    for (int k = 0; k < splitted.size(); k++) {
        int x = stoi(splitted.at(k));
        bool lpf = false;
        bool tonextreq = false;     //flag if i need to go to next req

        if (!findAndUpdate_fifo(x, arr, frames)) {
            pf++;
            lpf = true;
            for (int i = 0; i < frames; i++) {      //looking for first free page
                if (arr[i] == -1) {     //frame is free, so i can swapin the page
                    arr[i] = x;
                    tonextreq = true;
                    break;
                }
            }
            if (!tonextreq) {     //no one of frames are free, so need to check who swapout...
                set<pair<int, int>> checking;       //save pair of (first future req for pagen, pagen),
                // pagen are the page actually in memory
                for (auto fim: arr) {
                    bool pfind = false;             //flag to see if pagen is scheduled in future or not
                    for (int i = k + 1; i < splitted.size() && !pfind; i++) {
                        if (fim == stoi(splitted.at(i))) {
                            pair<int, int> loc(i, fim);
                            checking.insert(loc);
                            pfind = true;
                        }
                    }
                    if (!pfind) {       //fim(pagen) is not in next reqs, so push at the end
                        pair<int, int> loc(splitted.size(), fim);
                        checking.insert(loc);
                    }
                }

                //delete the last occurence of frames in memory
                for (int i = 0; i < frames; i++) {
                    if (arr[i] == checking.rbegin()->second) {
                        arr[i] = x;
                        break;
                    }
                }
            }
        }

        //print memory at SC(RAM,PSI(R_alpha,i))
        cout << "[i:" << setw(2) << k << "]\tPSI(R_alpha,i) :: <" << setw(2) << x << ">\t|";
        for (int k = 0; k < frames; k++) {
            cout << setw(2) << arr[k] << " | ";
        }
        cout << (lpf ? "\tPF " : "");
        cout << "\n";
    }
    cout << "Total page faults were " << pf << "\n";
    cout << "\n";
}

///////////////////// MAIN
int main(int argc, char *argv[]) {

    algo_t algo = fifo;
    string reference_string = "1 2 3 4 2 1 5 6 2 1 2 3 7 6 3 2 1 2 3 6";
    int frames = 4;

    if (argc > 1) {

        if (strcmp(argv[1], "--help") == 0) {
            cout << "Usage:\n"
                    "\t - arg1: algorithm to applicate: {fifo, second_chance, WS, WSEval, WS_imp, OPT}\n"
                    "\t - arg2: number of frames\n"
                    "\t - arg3: reference string\n\n"
                    "The \"WSEval\" show only how the WS change in time. We assume WS(t,Delta) with Delta= number of frames.\n\n"
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
        else if (strcmp(argv[1], "WSEval") == 0)
            algo = WSEval;
        else if (strcmp(argv[1], "OPT") == 0)
            algo = OPT;
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
    if (algo == WSEval)
        printWSFlow(reference_string, frames);
    if (algo == OPT)
        printHitsAndFaults_OPT(reference_string, frames);

    if (algo == WIP)
        cout << "WIP, not implemented yet!\n";


    return 0;
}