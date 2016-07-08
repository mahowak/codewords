//
//  Header.h
//  Codewords
//
//  Created by Kyle Mahowald on 5/22/16.
//  Copyright © 2016 Kyle Mahowald. All rights reserved.
//

//TODO: fix loops so that we go through all n possibilities for each word
//TODO: fix it so that if you mistype a real word, it doesn't do something silly
//TODO: write out guesses to file for later


#include "helper.h"
#include <set>
#include <map>
#include <algorithm>    // std::random_shuffle
#include <cstdlib>      // std::rand, std::srand

string RELEVANT_GLOVE = "relevant_glove_300.csv";
string GLOVE_WORDS = "glove_words_300.txt";
int MAXLEVEL = 4;
int TEAM0 = 9;
int TEAM1 = 8;
int NEUT = 7;
int BOMB = 1;
int TEAM0_COMPUTER_GUESSES = 0;
int TEAM1_COMPUTER_GUESSES = 0;
int TEAM0_COMPUTER_CLUES = 1;
int TEAM1_COMPUTER_CLUES = 1;
bool VERBOSE = false;
int NUM_OF_WORDS = TEAM0 + TEAM1 + NEUT + BOMB;

int cpu_guesses[2] = {TEAM0_COMPUTER_GUESSES, TEAM1_COMPUTER_GUESSES};
int cpu_clues[2] = {TEAM0_COMPUTER_CLUES, TEAM1_COMPUTER_CLUES};


//We need this to get random_shuffle to work right.
struct RNG {
    int operator() (int n) {return std::rand() / (1.0 + RAND_MAX) * n;}
};

//Store n, clue, and utlity of a particular clue
struct CluePair {
    int n;
    string clue;
    double utility;
};


//Find an element in a vector, say 1 if it's there 0 o.w.
bool vector_find(vector<int> &v, int j) {
    for (int i = 0; i < v.size(); i++) {if (v[i] == j) return 1;}
    return 0;
}


//Node for storing possible decisions as to which word to guess
//Node.tree is the sequence of items picked.
//p is the probability, denom is the denominator at that particular level
//p is computed by multiplying the parent node's probability by the current value and dividing by the parent's denom
//exp_value is found by multiplying utility by p
//Utility is additive and is the sum of all the nodes in the parent's line
class Node {
    
public:
    vector <int> tree;
    int last_entry = -1;
    double p = 1;
    double denom = 1;
    Node *parent;
    int utility = 0;
    double exp_value = 0;
    
    void update_p(double value, int u);
    
    double get_exp_value() {return exp_value;}
    
    void set_initial_node(vector< vector<double> > &v, int j, set<int> &remaining_values );
    
    
};


class Codewords {
    
private:
    vector<vector <double> > lexicon_values = read_trans_matrix(RELEVANT_GLOVE);
    vector <string> lexicon_words = simple_line_read(GLOVE_WORDS);
    vector <string> board_words;
    vector <vector <double> > board_values;
    vector <vector < double> > dist_matrix; //distance matrix for all lexicon values to board values
    map <string, int> string_to_int; //dictionary from board string to its id
    map <int, string> int_to_string; //dictionary from board id to its string
    map<string, int> clue_string_to_int;
    Node initial_node;
    int current_team = 0;
    vector<int> board_setup;
    int utility[4];
    vector< vector < Node > > allowed_moves_tree;
    set <int> remaining_values; //values that are still allowed
    
    //input a value on the board, get its string
    string get_string_from_value(int const s) {return int_to_string[s];}
    
    //input a string on the board, get its value
    int get_value_from_string(string const s) {return string_to_int[s];}
    
    //pick the words for the board, remove them from the lexicon
    void pick_board_words(int n);
    
    
    void setup_board();
    
    int get_utility(int board_id) {return utility[board_setup[board_id]];}
    
    double get_dist(int board_id, int lexicon_id) {return dist_matrix[board_id][lexicon_id];}
    
    //Normalize, compute distance, make a probability distribution of it (sum to 1)
    //Dimensions are board_words x lexicon
    void make_dist_matrix();
    
    void print_board();
    
    void print_board(int j);
    
    void initialize_game();
    
    //At start of each turn, set up allowed moves tree
    //N-dimensional vector with nodes, each node pointing to parent
    void initialize_allowed_moves_tree(int n);
    
    
    //Iterate through the nodes in allowed_moves_tree, updating the probabilities and expected values as we go
    double update_allowed_moves_tree(int lexicon_id);
    
    
    //Choose best 1, best 2, best 3, etc. Then choose best of those.
    CluePair pick_best_clue(int n);
    
    //Make L0 guess
    int L0(string clue);
    
    //Take the human guess as input, return as a board ID
    int take_human_guess();
    
    //evaluate guess and return 1 if correct, 0 if wrong, -1 if bomb
    int evaluate_guess(int guess_id);
    
    
    //make guess, return 1 if right, 0 if wrong, -1 if bomb
    int make_guess(int is_computer, string clue);
    
    
    //Print the current status and return whether the game is over (0 if over)
    int print_status();
    
    //Loop to guess n + 1 items
    int process_guess_turn(int n, int is_computer, string clue);
    
    //Helper functions for initializing tree;
    void initialize_first_node();
    void add_all_subsequent_nodes(int n);

    //Simple function to switch active team from 0 to 1 or 1 to 0
    void switch_team();
    
    //Main loop of game
    void main_loop();
    
    CluePair get_human_clue();
    
    //Give clue, return a struct with the n and the clue
    CluePair give_clue(int is_computer);
    
public:
    void run_tests();
    
    void play_code_words() {main_loop();}
    
};


