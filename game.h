#ifndef GAME_H
#define GAME_H

#include <ctime>
#include <stdlib.h>
enum type
{
    S,
    I,
    R
};

enum value
{
    NONE,
    END,
    A,
    B,
    C
};

class App
{
public:
    App();
    ~App();
    void play();
    bool setResponse();
    void operator()();
private:
    bool responded;
    value response;
    void giveNextInput();
    void takeInput();
};

#endif