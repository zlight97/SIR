#include <WMtk.h>
#include "SIR.h"
#include <iostream>
#include <stdlib.h>
/*
* Written by Zach Light, Last edited 2-28-2019
* Requires the WMtk library: https://github.com/jlphillipsphd/wmtk
* Solves the Save Ignore Recall task using AI controlled working memory
* For this the AI must learn to Save the most recent S, and only the most recent S
* This is a little trickier as we give it the ability to remember up to 3 states
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

void RunSimulation(bool verbose, bool end)//these variables hold easy verbose options from terminal
{
    debug = verbose;
    double finished_percentage = .99;
    int number_of_trials = 100000;

//this block holds the settings for the success window
    int window_size = 20;//this will succed with higher values, though it will take a longer time
    double goodness = 0.;
    int goodness_index = 0;
    int window[window_size];
    int q; //counter for sliding window
    for(q=0;q<window_size;q++) //initilizing window
        window[q]=0;

    int wm_size = 3;//Setting this to 1 makes the task MUCH simpler, as it is, the AI must learn to keep only 1 object in memory
    int state_feature_vector_size = 6;
    int chunk_feature_vector_size = 6;
    double lrate = .01;
    double lambda = .7;
    double ngamma = .99;
    double exploration_percentage = .01;//learns faster with ~.05, but more consistant with .01
    time_t random_seed = time(NULL);
    srand(random_seed);
    srand48(random_seed);
    cerr<< "Random Seed is: "<<random_seed<<"\n";
    state current_state;//this is the state data-type
    bool use_actor = false;//this isn't implemented in WMTK
    OR_CODE or_code = NOISY_OR;//read WMTK's FeatureVector.h for more info
                               //I would get best results with NOISY, then MAX, then NO

    WorkingMemory WM(wm_size, state_feature_vector_size,
	chunk_feature_vector_size, &current_state, user_reward_function,
	user_state_function, user_chunk_function, user_delete_function, use_actor,
	or_code);

	// Use learning rate
	WM.getCriticNetwork()->setLearningRate(lrate);

	/// Use lambda (eligibility discount rate)
	WM.getCriticNetwork()->setLambda(lambda);

	// Use gamma (discount rate)
	WM.getCriticNetwork()->setGamma(ngamma);

	// Use exploration percentage (chance it does something random)
	WM.setExplorationPercentage(exploration_percentage);

	// A list of chunk items for consideration for the Working Memory System
	list<Chunk> candidate_chunks;

	// A single chunk
	Chunk chunk;

	// Save the network to a file before beginning the simulation.
	WM.saveNetwork("./starting_network.dat");

    //loop for number of trials
    sir_chunk *ch;
    for(int trial = 0; trial< number_of_trials; trial++)
    {
        //These commented chunks are designed to make it more of a continuous run
        //This doesn't really work, as the AI will usually just learn to guess whatever the last success was
        //Will probably learn, but it will take a very long runtime (greater than MAX_INT)


        // if(trial%10==0)
        // {


            WM.newEpisode(true);
            generateTrial(current_state);
            chunk=generateS();//this force generates an S so we can confirm we always have one
                              //this makes it easier for the AI to pickup on the S as well

        // }else{
        //     WM.newEpisode(false);//this triggers reward
        //     resetSuccess(current_state);
        //     chunk=generateChoice();
        // }

        bool flag = false; //this flag is used to see if we have seen an R
        while(!flag)
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




        //This ending block calculates a running average with a sliding window defined above
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
//This used for a run where we do not forget the previous saved info each run
void resetSuccess(state& current_state)
{
    current_state.success=0;
    current_state.tested=0;
}

//generateTrial() fully generates a new blank state
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

    //first we obtain the state from theh working memory
    state* current_state = (state*) wm.getStateDataStructure();
    double reward = 0.;

    //we can only get a reward if we are told to recall
    if(current_state->sir!=R)
    {
        if(debug)
            cout<<"NOT R: "<<wm.getNumberOfChunks()<<"\n";
        return 0.;
    }
    // if(current_state->tested<0)
    //     return 0;
    int number_of_chunks = wm.getNumberOfChunks();
    if(number_of_chunks==0)
    {
        if(debug)
            cout<<"NO CHUNKS"<<endl;
        current_state->tested = -1.;//this state variable is depricated
        return 0.;
    }
    int x = rand()%number_of_chunks;
    sir_chunk* ch = ((sir_chunk*)(wm.getChunk(x).getData()));
    //this commented block helps the AI learn a little quicker, though it will learn anyway without it
    // if(ch->sir==I)
    // {
    //     reward = -1.;
    // }
    // else 

    if(ch->value==current_state->saved)
    {
        reward = 100.;//we choose a high reward because it makes the negative rewards less signficiant
        current_state->success = 1;//only successful if we choose the right number
    }
    //this is theoretically not needed, though it significantly decreases the time it takes to learn
    else{reward = -1.;}

    
    if(debug)
        cout<<"\nSize of the chunks is: "<<wm.getNumberOfChunks()<<"\nReward is: "
        <<reward<<"\nChosen was: "<<((sir_chunk*)(wm.getChunk(x).getData()))->value
        <<" "<<getLetterFromChunk(wm.getChunk(x))<<endl<<"Actual was: "<<current_state->saved<<endl<<endl;


    return reward;
}

/*
 * The state feature vector holds 6 units for each of the possibilities
 * It holds a binary string of:
 * S I R 1 2 3
 * 0 1 2 3 4 5
 * Each value here is marked based on what we recieve from our current state
 */
void user_state_function(FeatureVector& fv, WorkingMemory& wm)
{
    if(debug)
        cout<<"STATE VECTOR CALLED\n";
    //we clear it as all states are a completely new state
    fv.clearVector();

    //we get the current state from working memory, this will be turned into the feature vector
    state* current_state = (state*) wm.getStateDataStructure();
    switch(current_state->sir) //this switch puts each SIR into it's respective space
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
        fv.setValue(2+current_state->value,1.);//as value is an int we do not actually need another switch case
    return;
}

/*
 * The user chunk function does the same as the state function, except it is based on the passed chunks
 * in all essence it is the same thing, as we only hold one type of chunk
 */
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
        }
    }

}

/*
 * The user delete function deletes our only type of chunk
 * this is require by wmtk
 */
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
