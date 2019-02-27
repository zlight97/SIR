#ifndef SIR_H
#define SIR_H
#include <WMtk.h>

//This is the data-type for the given Save-Ignore-Repeat values
enum SIR
{
    S, I, R, NOTHING
};

//This holds all of the state data that is used to determine what is currently happening
struct state
{
    SIR sir;//What type it was
    int value;//It's value
    int saved;//The value of the last saved
    bool success;//Did we correctly get the saved variable
    float tested;//Used to handle the random number generator
};

//This is the data-type for what is stored in the Chunks of wmtk
struct sir_chunk
{
    SIR sir;
    int value;
};

// This creates the state for a new trial
void generateTrial(state& current_state);
void resetSuccess(state& current_state);

// User-defined function prototypes
double user_reward_function(WorkingMemory& wm);
void user_state_function(FeatureVector& fv, WorkingMemory& wm);
void user_chunk_function(FeatureVector& fv, Chunk& chk, WorkingMemory& wm);
void user_delete_function(Chunk& chk);

// The main function for the program. This is the function that actually
// performs the simulation.
void RunSimulation(bool verbose=false, bool end=false);

#endif