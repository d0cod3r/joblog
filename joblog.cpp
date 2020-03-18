/*
joblog - A tool to keep track of your worked hours

No licence for now.
Copyright 2020 Nicolas Essing.
*/


#include <string.h>   // string
#include <vector>     // vector
#include <iostream>   // command line in & out
#include <fstream>    // file in & out
#include <sys/stat.h> // mkdir
#include <exception>  // exceptions

#include "datetime.cpp"

using std::string;
using std::vector;
namespace dt = DateTime;


#include "joblog.h"


const string SAVEPATH = "dat"; //TODO change back to ".joblog";
const int SEARCHDEPTH = 10;

const std::ios_base::openmode FILEMODE = std::fstream::in | std::fstream::out;


// for debugging
void dbglg(string str) {
    std::cout << "> " << str << std::endl;
}


#include "coremethods.cpp"

#include "uimethods.cpp"


int main(int argc, char* argv[]) {
    vector<string> args;
    for (int i=1; i<argc; i++) {
        args.push_back(string(argv[i]));
    }
    return commandLineInterface(args);
}
