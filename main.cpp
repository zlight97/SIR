#include <iostream>
#include "SIR.h"

using namespace std;

int main(int argc, char* argv[])
{
    cout<<"Starting Simulation:\n";
    char val = '0';
    if(argc>1)
        val = *argv[1];
    bool debug = val=='1'||val=='3';
    bool dontEnd = val=='2'||val=='3';
    RunSimulation(debug,dontEnd);
    return 0;
}