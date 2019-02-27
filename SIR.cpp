#include <WMtk.h>
#include "SIR.h"
#include <iostream>
#include <stdlib.h>
/*
* Written by Zach Light, Last edited 2-27-2019
* 
*
*
*
8
*/

using namespace std;

bool debug = false;

char getLetterFromChunk(Chunk c)
{
    char letter;
    switch(((sir_chunk*)(c.getData()))->sir)
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
    return letter;
}

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
    if(c1==2)
        ret->value = 0;
    Chunk ch;
    ch.setData(ret);
    ch.setType("SIR");
    return ch;
}

Chunk generateS()
{
    int c1 = rand()%3+1;
    sir_chunk *ret = new sir_chunk;
    ret->value = c1;
    ret->sir = S;
    Chunk ch;
    ch.setData(ret);
    ch.setType("SIR");
    return ch;
}

void RunSimulation(bool verbose, bool end)
{
    debug = verbose;
    double finished_percentage = .99;
    int number_of_trials = 100000;

    int window_size = 50;
    double goodness = 0.;
    int goodness_index = 0;
    int window[window_size];

    int q; //counter for window

    for(q=0;q<window_size;q++) //initilizing window
        window[q]=0;

    int wm_size = 3;
    int state_feature_vector_size = 6;
    int chunk_feature_vector_size = 6;
    double lrate = .01;
    double lambda = .7;
    double ngamma = .99;
    double exploration_percentage = .01;
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
    sir_chunk *ch;
    chunk = generateS();
    for(int trial = 0; trial< number_of_trials; trial++)
    {
        //These commented chunks are designed to make it more of a continuous run
        //This doesn't really work, as the AI will just learn to guess whatever the last success was
        // if(trial%10==0)
        // {
            WM.newEpisode(true);
            generateTrial(current_state);
            chunk=generateS();
        // }else{
        //     WM.newEpisode(false);//this triggers reward
        //     resetSuccess(current_state);
        //     chunk=generateChoice();
        // }

        bool flag = false;
        while(/*ch->sir!=R||*/!flag)
        {
            ch = (sir_chunk*)chunk.getData();
            if(debug)
                cout<<"Generated Choice: "<<getLetterFromChunk(chunk)<<" "<<ch->value<<endl;
            current_state.sir = ch->sir;
            current_state.value = ch->value;
            if(current_state.sir==S)
            {
                current_state.saved=current_state.value;
            }
            if(ch->sir==R)
                flag = true;
            candidate_chunks.push_back(chunk);
            WM.tickEpisodeClock(candidate_chunks,false);//this triggers reward
            chunk=generateChoice();
        }





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
			
            if(debug||end)
                cout<<"END!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
            if(!end)
                break;
            
		}
        if(debug)
            cout<<"END OF TRIAL!!!!!!!!!\n\n\n\n";
    }
}

void resetSuccess(state& current_state)
{
    // current_state.sir = NOTHING;
    // current_state.value = 0;
    // current_state.saved = 0;
    current_state.success=0;
    current_state.tested=0;
}


void generateTrial(state& current_state)
{
    current_state.sir = NOTHING;
    current_state.value = 0;
    current_state.saved = 0;
    current_state.success=0;
    current_state.tested=0;
}

double user_reward_function(WorkingMemory& wm)
{
    if(debug)
        cout<<"REWARD CALLED: "<<endl;
    state* current_state = (state*) wm.getStateDataStructure();
    double reward = 0.;
    if(current_state->sir!=R)
    {
        if(debug)
            cout<<"NOT R: "<<wm.getNumberOfChunks()<<"\n";
        return 0.;
    }
    //this next line holds the case where R is called before S
    //This also may be where I need to pull a random one from memory
    // if(current_state->tested<0)
    //     return 0;
    int number_of_chunks = wm.getNumberOfChunks();
    if(number_of_chunks==0)
    {
        if(debug)
            cout<<"NO CHUNKS"<<endl;
        current_state->tested = -1.;
        return 0.;
    }
    int x = rand()%number_of_chunks;
    sir_chunk* ch = ((sir_chunk*)(wm.getChunk(x).getData()));
    if(ch->sir==I)
    {
        reward = -1.;
    }
    else 
    if(ch->value==current_state->saved)
    {
        reward = 100.;//changing the value of the given reward drastically affects runtime
        current_state->success = 1;
    }
    else{reward = -1.;}

    
    if(debug)
        cout<<"\nSize of the chunks is: "<<wm.getNumberOfChunks()<<"\nReward is: "
        <<reward<<"\nChosen was: "<<((sir_chunk*)(wm.getChunk(x).getData()))->value
        <<" "<<getLetterFromChunk(wm.getChunk(x))<<endl<<"Actual was: "<<current_state->saved<<endl<<endl;


    return reward;
}
void user_state_function(FeatureVector& fv, WorkingMemory& wm)
{
    if(debug)
        cout<<"STATE VECTOR CALLED\n";
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
    // if(current_state->saved>0)
    //     fv.setValue(6,(float)current_state->saved);//idk if this is right
    return;
}
void user_chunk_function(FeatureVector& fv, Chunk& chk, WorkingMemory& wm)
{
    fv.clearVector();
    sir_chunk* ch = ((sir_chunk*)chk.getData());
    if(chk.getType() == "SIR")
    {
        switch(ch->sir)
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
    if(ch->value>0)
    {
        fv.setValue(ch->value+2,1.);
        // cout<<"INDEX: "<<ch->value+2<<endl;
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
