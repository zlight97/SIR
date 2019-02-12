#include <iostream>
#include <thread>
#include <ctime>
#include "game.h"

#define ch c_str()
using namespace std;

string generateChoice()
{
    int c1 = rand()%3;
    int c2 = rand()%3;
    string ret = "";
    switch(c1)
    {
        case 0:
        ret = "S";
        break;
        case 1:
        ret = "I";
        break;
        default:
        return "R";
    }
    switch(c2)
    {
        case 0:
        ret = ret +" 1";
        break;
        case 1:
        ret = ret +" 2";
        break;
        default:
        ret = ret +" 3";
        break;
    }
    return ret;
}

int main(int argc, char* argv[])
{
    int runtime = 10000;
    int reward = 0;
    int total = 0;
    double running  = 0.;
    time_t seed = time(NULL);
    srand(seed);
    srand48(seed);
    cout<<"Seed is: " <<seed<<endl;
    string currentChoice = "S 1";
    string lastSaved = "1";
    if(currentChoice.ch[0]=='S')
        lastSaved = currentChoice.ch[2];
    for(int i = 0; i<runtime; i++)
    {
        double reward = 0;
        string input;
        cout<<currentChoice<<endl;
        // cin>>input;
        // if(currentChoice.ch[0]!='R'&&input!="P")
        // {

        // }
        // else
        if(currentChoice.ch[0]=='R')
        {
            cin>>input;
            if(atoi(input.ch)==atoi(lastSaved.ch))
                reward = 1;
        }
        cout<<"Reward is: "<<reward<<endl; 
        currentChoice=generateChoice();
        if(currentChoice.ch[0]=='S')
            lastSaved = currentChoice.ch[2];
    }
    return 0;
}