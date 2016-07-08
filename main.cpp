//
//  main.cpp
//  Codewords
//
//  Created by Kyle Mahowald on 5/22/16.
//  Copyright © 2016 Kyle Mahowald. All rights reserved.
//

#include <iostream>
#include "Header.h"


void Node::update_p(double value, int u) {
    p = parent->p * value/(parent->denom);
    denom = parent->denom - value;
    utility = parent->utility + u;
    exp_value = utility * p;
}


void Node::set_initial_node(vector< vector<double> > &v, int j, set<int> &remaining_values ) {
    denom = 0;
    utility = 0;
    p = 1;
    for (auto it = remaining_values.begin(); it != remaining_values.end(); it++) {
        denom += v[*it][j];
    }
}


//pick the words for the board, remove them from the lexicon
void Codewords::pick_board_words(int n) {
    set<int> pos_to_delete;
    for (int i = 0; i < n; i++) {
        int r =  rand() % lexicon_words.size();
        while (pos_to_delete.count(r) > 0) {
            r =  rand() % lexicon_words.size();
        }
        pos_to_delete.insert(r);
        vector <double> rvalue = lexicon_values[r];
        string rword = lexicon_words[r];
        board_words.push_back(rword);
        board_values.push_back(rvalue);
    }
    lexicon_values = delete_multiple_items_from_vector_matrix(lexicon_values, pos_to_delete);
    lexicon_words = delete_multiple_items_from_string_vector(lexicon_words, pos_to_delete);
}


void Codewords::setup_board() {
    for (int i = 0; i < TEAM0; i++) {
        board_setup.push_back(0);
    }
    for (int i = 0; i < TEAM1; i++) {
        board_setup.push_back(1);
    }
    for (int i = 0; i < NEUT; i++) {
        board_setup.push_back(2);
    }
    for (int i = 0; i < BOMB; i++) {
        board_setup.push_back(3);
    }
    std::random_shuffle(board_setup.begin(), board_setup.end(), RNG());
    utility[0] = 25;
    utility[1] = -15;
    utility[2] = 0;
    utility[3] = -20;
}




void Codewords::make_dist_matrix() {
    vector<double> col_sums = get_vector_of_size(lexicon_values.size());
    for (int i = 0; i < board_words.size(); i++) {
        vector <double> s = get_vector_of_size(lexicon_values.size());
        dist_matrix.push_back(s);
        for (int j = 0; j < lexicon_values.size(); j++) {
            double wd = pow(1 - cosine_similarity(lexicon_values[j], board_values[i]), 6);
            dist_matrix[i][j] = wd;
            col_sums[j] += wd;
        }
    }
    for (int i = 0; i < board_words.size(); i++) {
        for (int j = 0; j < lexicon_values.size(); j++) {
            double normalized_value = (dist_matrix[i][j])/col_sums[j];
            dist_matrix[i][j] = normalized_value;
        }
    }
}

void Codewords::print_board() {
    int i = 0;
    for  (auto it = remaining_values.begin(); it != remaining_values.end(); it++) {
        cout << board_words[*it]; //<< " " << board_setup[*it];
        if (i == 4) {cout << "\n"; i = 0;}
        else cout << "\t";
        i++;
    }
}

void Codewords::print_board(int j) {
    int i = 0;
    for  (auto it = remaining_values.begin(); it != remaining_values.end(); it++) {
        cout << board_words[*it] << " " << board_setup[*it] << "\n";
        i++;
    }
}

void Codewords::initialize_game() {
    pick_board_words(NUM_OF_WORDS);
    setup_board();
    make_dist_matrix();
    for (int i = 0; i < NUM_OF_WORDS; i++ ) {
        remaining_values.insert(i);
        int_to_string[i] = board_words[i];
        string_to_int[board_words[i]] = i;
    }
    for (int i = 0; i < lexicon_words.size(); i++) {
        clue_string_to_int[lexicon_words[i]] = i;
    }
    print_board();
}


void Codewords::initialize_first_node() {
    
    //add first layer, for all legal moves
    for  (auto it = remaining_values.begin(); it != remaining_values.end(); it++) {
        Node new_node;
        new_node.parent = &initial_node;
        new_node.tree.push_back(*it);
        new_node.last_entry = *it;
        allowed_moves_tree[0].push_back(new_node);
    }
}

void Codewords::add_all_subsequent_nodes(int n) {
    for (int i = 1; i < n; i++) {
        for (int j = 0; j < (allowed_moves_tree[i - 1]).size(); j++) {
            for  (auto it = remaining_values.begin(); it != remaining_values.end(); it++) {
                int last_entry = allowed_moves_tree[i-1][j].last_entry;
                vector<int> tree = allowed_moves_tree[i-1][j].tree;
                if (vector_find(tree, *it) == 0 && board_setup[last_entry] == current_team) {
                    Node new_node;
                    new_node.parent = &(allowed_moves_tree[i-1][j]);
                    new_node.tree = new_node.parent->tree;
                    new_node.tree.push_back(*it);
                    new_node.last_entry = *it;
                    allowed_moves_tree[i].push_back(new_node);
                }
            }
        }
    }
}

void Codewords::initialize_allowed_moves_tree(int n) {
    
    //clear old tree
    allowed_moves_tree.clear();
    
    //initialize tree to have n layers
    for (int i = 0; i < n; i++ ) {
        vector<Node> v;
        allowed_moves_tree.push_back(v);
    }

    initialize_first_node();
    add_all_subsequent_nodes(n);

}



double Codewords::update_allowed_moves_tree(int lexicon_id) {
    double sum_p = 0;
    double total_exp_value = 0; ///TODO: update this and return
    initial_node.set_initial_node(dist_matrix, lexicon_id, remaining_values);
    for (int i = 0; i < allowed_moves_tree.size(); i++) {
        for (int j = 0; j < allowed_moves_tree[i].size(); j++) {
            int board_id = allowed_moves_tree[i][j].last_entry;
            allowed_moves_tree[i][j].update_p(get_dist(board_id,  lexicon_id), get_utility(board_id));
            
            //if it's a TERMINAL node, add the expected value
            if (i == allowed_moves_tree.size() - 1 || board_setup[board_id] != current_team) {
                total_exp_value += allowed_moves_tree[i][j].get_exp_value();
                sum_p += allowed_moves_tree[i][j].p;
                
            }
        }
    }
    return total_exp_value;
}


//Choose best 1, best 2, best 3, etc. Then choose best of those.
CluePair Codewords::pick_best_clue(int n) {
    initialize_allowed_moves_tree(n);
    CluePair best_clue;
    double max_exp_value = -1000000;
    double cur_exp_value;
    best_clue.n = n;
    for (int i = 0; i < lexicon_words.size(); i++) {
        cur_exp_value = update_allowed_moves_tree(i);
        if (cur_exp_value > max_exp_value) {
            max_exp_value = cur_exp_value;
            best_clue.clue = lexicon_words[i];
            best_clue.utility = cur_exp_value;
            //cout << "BEST: "  << best_clue.clue << " " << best_clue.utility << " " << best_clue.n << "\n";
        }
    }
    return best_clue;
}



//Make L0 guess
int Codewords::L0(string clue) {
    int lexicon_id = clue_string_to_int[clue];
    double max_p = -100000;
    double cur_dist;
    int cur_guess;
    int best_guess = -1;
    for (auto it = remaining_values.begin(); it != remaining_values.end(); it++) {
        cur_guess = *it;
        cur_dist =  get_dist(cur_guess, lexicon_id);
        if (cur_dist > max_p) {
            max_p = cur_dist;
            best_guess = cur_guess;
        }
    }
    return best_guess;
}

//Take the human guess as input, return as a board ID
int Codewords::take_human_guess() {
    cout << "\n*****************\n";
    print_board();
    cout << "\n*****************\n";
    cout << "\nTeam " << current_team << " MAKE A GUESS: ";
    string s;
    cin >> s;
    if (s == "PASS") {return -1;}
    return get_value_from_string(s);
}

//evaluate guess and return 1 if correct, 0 if wrong, -1 if bomb
int Codewords::evaluate_guess(int guess_id) {
    if (board_setup[guess_id] == current_team) {
        cout << "CORRECT!\n";
        return 1;
    }
    if (board_setup[guess_id] == 3) {
        cout << "BOMB!\n";
        return -1;
    }
    if (board_setup[guess_id] == 1 - current_team) {
        cout << "WRONG! Other team.\n";
    }
    else cout << "WRONG! Neutral.\n";
    return 0;
}


//make guess, return 1 if right, 0 if wrong, -1 if bomb
int Codewords::make_guess(int is_computer, string clue) {
    //IF is_computer, then do L0 guess
    int guess_id;
    if (is_computer == 1) {
        guess_id = L0(clue);
        cout << "I GUESS: " << get_string_from_value(guess_id) << "\n";
        
    }
    
    //ELSE, take human input and make guess
    else {
        guess_id = take_human_guess();
        if (guess_id == -1) return 0; //PASS was chosen
        while (remaining_values.count(guess_id) == 0) {
            cout << "NOT A VALID GUESS. TRY AGAIN!\n";
            guess_id = take_human_guess();
        }
        
    }
    
    //EVALUATE guess, REPORT result
    int correct = evaluate_guess(guess_id);
    
    //DELETE the guessed item
    //cout << "DELETING: " << guess_id << "\n";
    remaining_values.erase(guess_id);
    
    return correct;
}


//Print the current status and return whether the game is over (0 if over)
int Codewords::print_status() {
    int team1 = 0;
    int team0 = 0;
    for (int i = 0; i < board_setup.size(); i++) {
        if (remaining_values.count(i) > 0) {
            if (board_setup[i] == 1) team1++;
            if (board_setup[i] == 0) team0++;
        }
    }
    cout << "\n*************************\n";
    cout << "Team 0 has: " << team0 << " remaining.\n";
    cout << "Team 1 has: " << team1 << " remaining.\n";
    if (team1 == 0 || team0 == 0) {return 0;}
    return 1;
    
}

//Loop to guess n + 1 items
int Codewords::process_guess_turn(int n, int is_computer, string clue) {
    int correct = 0;
    int status = 0;
    for (int i = 0; i < n + 1; i++) {
        cout << "\nTHE CLUE IS: " << clue << " " << n;
        correct = make_guess(is_computer, clue);
        status = print_status();
        if (correct == -1) {
            return 0;
        }
        if (correct != 1) {
            return status;
        }
    }
    return status;
}


//Simple function to switch active team from 0 to 1 or 1 to 0
void Codewords::switch_team() {
    current_team = 1 - current_team;
    int temp = utility[0];
    utility[0] = utility[1];
    utility[1] = temp;
}


//Main loop of game
void Codewords::main_loop() {
    cout << "\n\nTHE GAME IS BEGINNING.\nEnter PASS (all caps) to pass.\n***********\n";
    int game_still_going = 1;
    int computer_gives_clue;
    int computer_guesses;
    CluePair cp;
    while (game_still_going == 1) {
        computer_gives_clue = cpu_clues[current_team];
        computer_guesses = cpu_guesses[current_team];
        if (computer_gives_clue == 0) {
            print_board(1);
            cp = give_clue(0);
        }
        else { cp = give_clue(1);
            //print_board();
        }
        cout << "\nThe clue is: " << cp.clue <<  " " << cp.n << " " << "\n************\n";
        print_board();
        if (computer_guesses == 1) {cp.n = cp.n - 1;}
        game_still_going = process_guess_turn(cp.n, computer_guesses, cp.clue);
        switch_team();
    }
    cout << "GAME OVER!\n";
    return;
}

CluePair Codewords::get_human_clue() {
    CluePair cp;
    cout << "\n Enter clue for Team " << current_team << ": ";
    cin >> cp.clue;
    int found = 0;
    while (found == 0) {
        for (int i = 0; i < lexicon_words.size(); i++) {
            if (lexicon_words[i] == cp.clue) found = 1;
        }
        if (found == 0) {
            cout << "\n NOT VALID. Enter clue: ";
            cin >> cp.clue;
        }
    }
    cout << "\n Enter number to guess: ";
    cin >> cp.n;
    return cp;
}


CluePair Codewords::give_clue(int is_computer) {
    CluePair cp;
    cp.n = MAXLEVEL;
    cp.clue = "clue goes here";
    cp.utility = -1000000;
    
    //IF is_computer, then get the computer's best guess
    if (is_computer == 1) {
        
        for (int i = 1; i < (MAXLEVEL + 1); i++) {
            CluePair current_clue = pick_best_clue(i);
            if (current_clue.utility > cp.utility) {
                cp = current_clue;
            }
        }
        
        return cp;
    }
    
    //ELSE, take human input
    else {
        cp = get_human_clue();
        return cp;
    }
    
}

void Codewords::run_tests() {
    for (int i = 0; i < 5; i++) {
        cout << lexicon_words[i] << " ";
        cout << lexicon_values[i][0] << "\n";
    }
    cout << lexicon_words.size() << " should =  " << lexicon_values.size() << "\n";
    
    initialize_game();
    cout << lexicon_words.size() << " should =  " << lexicon_values.size() << "\n";
    
    
    string firstword = board_words[0];
    double dist = -1000;
    string maxword = "";
    for (int i = 0; i < lexicon_words.size(); i++) {
        if (dist_matrix[0][i] > dist) {
            maxword = lexicon_words[i];
            dist = dist_matrix[0][i];
            cout << maxword << " ";
            cout << dist << "\n";
        }
    }
    cout << firstword << " is closest to: " << maxword;
}






int main(int argc, const char * argv[]) {
    srand (time(NULL));
    Codewords x;
    x.run_tests();
    x.play_code_words();
}
