#include "game.h"

App::App()
{
    responded = false;
    response = NONE;
    
}

void App::operator()()
{
    
}

void App::play()
{
    double finished_percentage = .98;

    int number_of_trials = 10000;

    double goodness = 0.;
    int goodness_index = 0;
    int wm_size = 3;//see if it can learn to only keep one
    int state_feature_vector_size = 6;
    int chunk_feature_vector_size = 6;

    double lrate = .01;
    double lambda = .7;
    double ngamma = .99;
    double exploration_percentage = .05;




    time_t seed = time(NULL);
    srand(seed);
    srand48(seed);
    cout<< "Random Seed: "<<seed;

}

bool App::setResponse()
{

}

App::~App()
{

}