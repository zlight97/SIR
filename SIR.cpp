#include <WMtk.h>
#include "SIR.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

Chunk generateChoice()
{
    int c1 = rand()%3;
    int c2 = rand()%3+1;
    sir_chunk *ret = new sir_chunk;
    switch(c1)
    {
        case 0:
            ret->sir=S;
        break;
        case 1:
            ret->sir=I;
        break;
        case 2:
            ret->sir=R;
        break;
    }
    ret->value = c2;
    Chunk ch;
    ch.setData(ret);
    ch.setType("SIR");
    return ch;
}

void RunSimulation()
{
    double finished_percentage = .99;
    int number_of_trials = 100000;

    int window_size = 20;
    double goodness = 0.;
    int goodness_index = 0;
    int window[window_size];

    int q; //counter for window

    for(q=0;q<window_size;q++) //initilizing window
        window[q]=0;

    int wm_size = 3;
    int state_feature_vector_size = 7;
    int chunk_feature_vector_size = 3;
    double lrate = .01;
    double lambda = .7;
    double ngamma = .99;
    double exploration_percentage = .02;

    time_t random_seed = time(NULL);
    srand(random_seed);
    srand48(random_seed);
    cerr<< "Random Seed is: "<<random_seed<<"\n";
    state current_state;
    bool use_actor = false;
    OR_CODE or_code = NOISY_OR;
    
    WorkingMemory WM(wm_size, state_feature_vector_size,
	chunk_feature_vector_size, &current_state, user_reward_function,
	user_state_function, user_chunk_function, user_delete_function, use_actor,
	or_code);

	// Use learning rate
	WM.getCriticNetwork()->setLearningRate(lrate);

	/// Use lambda
	WM.getCriticNetwork()->setLambda(lambda);

	// Use gamma
	WM.getCriticNetwork()->setGamma(ngamma);

	// Use exploration percentage
	WM.setExplorationPercentage(exploration_percentage);

	// A list of chunk items for consideration for the Working Memory System
	list<Chunk> candidate_chunks;

	// A single chunk
	Chunk chunk;

	// Save the network to a file before beginning the simulation.
	WM.saveNetwork("./starting_network.dat");

    //loop for number of trials
    for(int trial = 0; trial< number_of_trials; trial++)
    {
        generateTrial(current_state);
        WM.newEpisode(true);
        bool flag = false;
        while(((sir_chunk*)((chunk=generateChoice()).getData()))->sir!=R||!flag)
        {
            sir_chunk *ch = (sir_chunk*)chunk.getData();
            current_state.sir = ch->sir;
            current_state.value = ch->value;
            if(current_state.sir==S)
            {
                flag = true;
                current_state.saved=current_state.value;
            }
            candidate_chunks.push_back(chunk);
        }

            WM.tickEpisodeClock(candidate_chunks);
        cout<<trial<< " ";
        		if (current_state.success) {
			window[goodness_index++] = 1;
			for (q = 0; q < window_size; q++)
				goodness += (double) window[q];
			goodness /= (double) window_size;

			cout << "1 " << goodness << endl;
		}
		else {
			window[goodness_index++] = 0;
			for (q = 0; q < window_size; q++)
				goodness += (double) window[q];
			goodness /= (double) window_size;

			cout << "0 " << goodness << endl;
		}

		if (goodness_index == window_size)
			goodness_index = 0;

		// If we are performing as well as we want, then we're finished.
		if (goodness >= finished_percentage) {
			break;
		}
    }
}

void generateTrial(state& current_state)
{
    current_state.sir = NOTHING;
    current_state.value = 0;
    current_state.saved = 0;
    current_state.success=0;
}

double user_reward_function(WorkingMemory& wm)
{
    state* current_state = (state*) wm.getStateDataStructure();
    double reward = 0.;
    //this next line holds the case where R is called before S
    //This also may be where I need to pull a random one from memory
    int number_of_chunks = wm.getNumberOfChunks();
    if(number_of_chunks==0)
        return 0.;
    int x = rand()%number_of_chunks;
    if(((sir_chunk*)(wm.getChunk(x).getData()))->value==current_state->saved)
    {
        reward = 3.;//changing the value of the given reward drastically affects runtime
        current_state->success = 1;
    }
    char letter;
    // if(current_state->sir!=R)
    //     reward = 0;
    switch(((sir_chunk*)(wm.getChunk(x).getData()))->sir)
    {
        case S:
        letter = 'S';
        break;
        case I:
        letter = 'I';
        break;
        case R:
        letter = 'R';
        break;
        default:
        letter = '0';
        break;
    }
    cout<<"\nSize of the chunks is: "<<wm.getNumberOfChunks()<<"\nReward is: "
    <<reward<<"\nChosen was: "<<((sir_chunk*)(wm.getChunk(x).getData()))->value
    <<" "<<letter<<endl<<"Actual was: "<<current_state->saved<<endl<<endl;
    return reward;
}
void user_state_function(FeatureVector& fv, WorkingMemory& wm)
{
    fv.clearVector();

    state* current_state = (state*) wm.getStateDataStructure();
    switch(current_state->sir)
    {
        case S:
            fv.setValue(0,1.);
        break;
        case I:
            fv.setValue(1,1.);
        break;
        case R:
            fv.setValue(2,1.);
        break;
        default:
        break;
    }
    if(current_state->value>0)
        fv.setValue(2+current_state->value,1.);
    if(current_state->saved>0)
        fv.setValue(6,(float)current_state->saved);//idk if this is right
    return;
}
void user_chunk_function(FeatureVector& fv, Chunk& chk, WorkingMemory& wm)
{
    fv.clearVector();

    if(chk.getType() == "SIR")
    {
        switch(((sir_chunk*) chk.getData())->sir)
        {
            case S:
                fv.setValue(0,1.);
                fv.setValue(1,(double)((sir_chunk*)chk.getData())->value);
            break;
            case I:
                fv.setValue(1,(float)((sir_chunk*)chk.getData())->value);
            break;
            case R:
                fv.setValue(0,2.);
                fv.setValue(1,(float)((sir_chunk*)chk.getData())->value);
                //i dont think this can happen??
            break;
            default:
                //this should only happen if R is called with no predecessor
            break;
        }
    }

}
void user_delete_function(Chunk& chk)
{
    if(chk.getType()=="SIR")
    {
        delete ((sir_chunk*) chk.getData());
        chk.setType("EMPTY");
    }
    else{
        cerr<<"**ERROR** Unidentified chunk attempting to be deleted!\n";
    }
}
