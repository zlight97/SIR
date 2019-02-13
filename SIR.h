#ifndef SIR_H
#define SIR_H
#include <WMtk.h>

enum SIR
{
    S, I, R, NOTHING
};


struct state
{
    SIR sir;
    int value;
    int saved;
};

struct sir_chunk
{
    SIR sir;
    int value;
};

void generateTrial(state& current_state);

// User-defined function prototypes
double user_reward_function(WorkingMemory& wm);
void user_state_function(FeatureVector& fv, WorkingMemory& wm);
void user_chunk_function(FeatureVector& fv, Chunk& chk, WorkingMemory& wm);
void user_delete_function(Chunk& chk);

// The main function for the program. This is the function that actually
// performs the simulation.
void RunSimulation();

#endif