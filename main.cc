// Game of Othello -- Example of main
// Universidad Simon Bolivar, 2012.
// Author: Blai Bonet
// Last Revision: 1/11/16
// Modified by: Emmanuel Bandres, 14-10071
//              Daniela Caballero, 14-10140
//              Fernando Gonzalez, 08-10464

#include <iostream>
#include <limits>
#include <fstream>
#include <string>
#include <chrono>
#include "othello_cut.h" // won't work correctly until .h is fixed!
#include "utils.h"

#include <unordered_map>

using namespace std;
using namespace std::chrono;

unsigned expanded = 0;
unsigned generated = 0;
int tt_threshold = 32; // threshold to save entries in TT
double time_limit = 3600;
ofstream myfile;


// Transposition table (it is not necessary to implement TT)
struct stored_info_t {
    int value_;
    int depth_;
    int type_;
    enum { EXACT, LOWER, UPPER };
    stored_info_t(int value = -100, int depth = 0, int type = LOWER) : value_(value), depth_(depth), type_(type) { }
};

struct hash_function_t {
    size_t operator()(const state_t &state) const {
        return state.hash();
    }
};

class hash_table_t : public unordered_map<state_t, stored_info_t, hash_function_t> {
};

hash_table_t TTable[2];

//int maxmin(state_t state, int depth, bool use_tt);
//int minmax(state_t state, int depth, bool use_tt = false);
//int maxmin(state_t state, int depth, bool use_tt = false);

vector<state_t> child_vector(state_t state, int color) {
    // Contrincante = true para negro y false para blanco
    bool Contrincante = (color == 1)? true:false;
    vector<state_t> movement;
    state_t new_state;

    // Almacena los movimientos validos
    for(int position = 0; position < DIM; ++position) 
    {
        if (state.outflank(Contrincante, position)) 
        {
            new_state = state.move(Contrincante, position);
            movement.push_back(new_state);
        }
    }
    return movement;
};

void check_time(time_point<high_resolution_clock> st) {
    auto curr_time = high_resolution_clock::now();
	duration<double> elapsed = curr_time - st;
    if (elapsed.count() > time_limit)
    {
        cout << "Time limit reached" << endl;
        myfile << "Time limit reached" << endl;
        myfile.close();
        exit(1);
    }
}

int negamax(state_t state, int depth, int color, time_point<high_resolution_clock> st, bool use_tt){
    
    check_time(st);

    ++generated;

    if (use_tt && (TTable[1 == color].find(state) != TTable[1 == color].end())) {

        stored_info_t info = TTable[1 == color].at(state);
        if (info.depth_ >= depth) return info.value_ ;
        
    }

    if (depth == 0 || state.terminal())
    {
        return color * state.value();
    }

    // si no es estado terminal, expande
    ++expanded;
    int alpha = numeric_limits<int>::min();
    // generando movimientos validos
    vector<state_t> child_states = child_vector(state,color);
    
    if (child_states.size() != 0)
    {
        for (state_t child : child_states) 
        {
            alpha = max(alpha, -negamax(child, depth - 1, -color, st, use_tt));
        }
    }

    else
    {
        // Sin modificaciones al estado, el otro color juega con el mismo estado.
        alpha = max(alpha, -negamax(state, depth - 1, -color, st, use_tt));
    }

    if (use_tt) {
        stored_info_t state_info = stored_info_t(alpha, depth, 0);
        TTable[1 == color][state] = state_info;

    }
    return alpha;
};


int negamax_alphabeta(state_t state, int depth, int alpha, int beta, int color, time_point<high_resolution_clock> st, bool use_tt){
    check_time(st);

    int score;
    int val;
    ++generated;
    int prev_alpha = alpha;

    if (use_tt && (TTable[1 == color].find(state) != TTable[1 == color].end())) {

        stored_info_t info = TTable[1 == color].at(state);
        if (info.depth_ >= depth) {

            if (info.type_ == 0) return info.value_ ;
            else if (info.type_ == 1) alpha = max(alpha, info.value_);
            else if (info.type_ == 2) beta = min(beta, info.value_);
            if (alpha >= beta) return info.value_ ;
        }
    }

    if (depth == 0 || state.terminal())
    {
        return color * state.value();
    }
    // si no es estado terminal, expande.
    ++expanded;
    score = numeric_limits<int>::min();
    // generando movimientos validos
    vector<state_t> child_states = child_vector(state,color);

    if (child_states.size() != 0)
    {
        for (state_t child : child_states) 
        {
            val = -negamax_alphabeta(child, depth - 1, -beta, -alpha, -color, st, use_tt);
            score = max(score,val);
            alpha = max(alpha,val);
            if (alpha >= beta)
            {
                break;
            } 
        }
    }
    else
    {
        // Sin movimientos disponibles, entonces el otro color juega con el mismo estado
        val = -negamax_alphabeta(state, depth - 1, -beta, -alpha, -color, st, use_tt);
        score = max(score,val);
    }

    if (use_tt) {
        stored_info_t state_info = stored_info_t(score, depth);

        if (score <= prev_alpha) state_info.type_ = 2;
        else if (score >= beta) state_info.type_ = 1;
        else state_info.type_ = 0;

        TTable[1 == color][state] = state_info;
    }
    
    return score;
};

bool test(state_t state, int depth, int score, bool comp, int color) {
    if (depth == 0 || state.terminal()) {
        if (comp) {
            return state.value() > score ? true : false;
        }
        return state.value() >= score ? true : false;
    }

    vector<state_t> child_states = child_vector(state,color);

    if (child_states.size() != 0) {
        for (state_t child : child_states) {
            if (color == 1 && test(child, depth - 1, score, comp, -color)) {
                return true;
            }

            if (color != 1 && !test(child, depth - 1, score, comp, -color)) {
                return false;
            }
        }
    } else {
        return test(state, depth - 1, score, comp, -color);
    }
    return !(color == 1);
}

int scout(state_t state, int depth, int color, time_point<high_resolution_clock> st, bool use_tt){
    check_time(st);
    
    int score = 0;
    int firstChild = 1;

    ++generated;

    if (use_tt && (TTable[1 == color].find(state) != TTable[1 == color].end())) {

        stored_info_t info = TTable[1 == color].at(state);

        if (info.depth_ >= depth) return info.value_ ;
    }

    if (depth == 0 || state.terminal()) {

        return state.value();
    }

    ++expanded;

    vector<state_t> child_states = child_vector(state,color);

    if (child_states.size() != 0) {

        for (state_t child : child_states) {

            if (firstChild) {

                firstChild = 0;
                score = scout(child, depth - 1, -color, st, use_tt);

            } else {

                if (color == 1 && test(child, depth, score, 1, -color)) {
                    score = scout(child, depth - 1, -color, st, use_tt);
                }
                if (color != 1 && !test(child, depth, score, 0, -color)) {
                    score = scout(child, depth - 1, -color, st, use_tt);
                }
            }
        }
        
    } else {
        score = scout(state, depth - 1, -color, st, use_tt);
        
    }

    if (use_tt) {
        stored_info_t state_info = stored_info_t(score, depth, 0);
        TTable[1 == color][state] = state_info;
    }

    return score;
}


int negascout(state_t state, int depth, int alpha, int beta, int color, time_point<high_resolution_clock> st, bool use_tt){
    check_time(st);
    
    int score;
    int frstChild = 1;
    int prev_alpha = alpha;

    ++generated;

    if (use_tt && (TTable[1 == color].find(state) != TTable[1 == color].end())) {

        stored_info_t info = TTable[1 == color].at(state);

        if (info.depth_ >= depth) {

            if (info.type_ == 0) return info.value_ ;
            else if (info.type_ == 1) alpha = max(alpha, info.value_);
            else if (info.type_ == 2) beta = min(beta, info.value_);
            if (alpha >= beta) return info.value_ ;
        }
    }

    if (depth == 0 || state.terminal())
    {
        return color * state.value();
    }

    // Si no es estado terminal, expande.
    ++expanded;
    // generando movimientos validos
    vector<state_t> children = child_vector(state,color);

    if (children.size() == 0) 
    {
        // Sin movimientos disponibles, entonces el otro color juega con el mismo estado
        score = -negascout(state, depth - 1,-beta,-alpha,-color, st, use_tt);
        alpha = max(alpha, score);
    }
    else{
        for (state_t child : children) 
        {
            if (frstChild)
            {
                frstChild = 0;
                score = -negascout(child, depth - 1, -beta, -alpha, -color, st, use_tt);
            }
            else
            {
                score = -negascout(child, depth - 1, -alpha - 1, -alpha, -color, st, use_tt);
                if ((alpha < score) && (score < beta))
                {
                    score = -negascout(child, depth - 1,-beta,-score,-color, st, use_tt);
                }
            }

            alpha = std::max(alpha,score);
            if (alpha >= beta)
            {
                break;
            }
        }
    }

    if (use_tt) {
        stored_info_t state_info = stored_info_t(alpha, depth);

        if (alpha <= prev_alpha) state_info.type_ = 2;
        else if (alpha >= beta) state_info.type_ = 1;
        else state_info.type_ = 0;

        TTable[1 == color][state] = state_info;
    }

    return alpha;
};


int main(int argc, const char **argv) {
    state_t pv[128];
    int npv = 0;
    for( int i = 0; PV[i] != -1; ++i ) ++npv;

    int algorithm = 0;
    if( argc > 1 ) algorithm = atoi(argv[1]);
    bool use_tt = argc > 2;
    string tt = use_tt ? "_tt" : "";

    myfile.open(to_string(algorithm) + tt + ".txt", ios::out | ios::trunc);

    // Extract principal variation of the game
    state_t state;
    cout << "Extracting principal variation (PV) with " << npv << " plays ... " << flush;
    for( int i = 0; PV[i] != -1; ++i ) {
        bool player = i % 2 == 0; // black moves first!
        int pos = PV[i];
        pv[npv - i] = state;
        state = state.move(player, pos);
    }
    pv[0] = state;
    cout << "done!" << endl;

#if 0
    // print principal variation
    for( int i = 0; i <= npv; ++i )
        cout << pv[npv - i];
#endif

    // Print name of algorithm
    cout << "Algorithm: ";
    myfile << "Algorithm: ";
    if( algorithm == 1 ) {
        cout << "Negamax (minmax version)";
        myfile << "Negamax (minmax version)";
    }
    else if( algorithm == 2 ) {
        cout << "Negamax (alpha-beta version)";
        myfile << "Negamax (alpha-beta version)";
    }
    else if( algorithm == 3 ) {
        cout << "Scout";
        myfile << "Scout";
    }
    else if( algorithm == 4 ) {
        cout << "Negascout";
        myfile << "Negascout";
    }

    cout << (use_tt ? " w/ transposition table" : "") << endl;
    myfile << (use_tt ? " w/ transposition table" : "") << endl;

    // Run algorithm along PV (bacwards)
    cout << "Moving along PV:" << endl;
    myfile << "Moving along PV:" << endl;

    time_point<high_resolution_clock> main_start_time = high_resolution_clock::now();
    for( int i = 0; i <= npv; ++i ) {
        //cout << pv[i];
        int value = 0;
        TTable[0].clear();
        TTable[1].clear();
        time_point<high_resolution_clock> start_time = high_resolution_clock::now();
        expanded = 0;
        generated = 0;
        int color = i % 2 == 1 ? 1 : -1;

        try {
            if( algorithm == 1 ) {
                value = negamax(pv[i], 33, color, main_start_time, use_tt);
            } else if( algorithm == 2 ) {
                value = negamax_alphabeta(pv[i], 33, -200, 200, color, main_start_time,use_tt);
            } else if( algorithm == 3 ) {
                value = color * scout(pv[i], 33, color, main_start_time, use_tt);
            } else if( algorithm == 4 ) {
                value = negascout(pv[i], 33, -200, 200, color, main_start_time, use_tt);
            }
        } catch( const bad_alloc &e ) {
            cout << "size TT[0]: size=" << TTable[0].size() << ", #buckets=" << TTable[0].bucket_count() << endl;
            myfile << "size TT[0]: size=" << TTable[0].size() << ", #buckets=" << TTable[0].bucket_count() << endl;
            cout << "size TT[1]: size=" << TTable[1].size() << ", #buckets=" << TTable[1].bucket_count() << endl;
            myfile << "size TT[1]: size=" << TTable[1].size() << ", #buckets=" << TTable[1].bucket_count() << endl;
            use_tt = false;
        }

        duration<double> elapsed_time = high_resolution_clock::now() - start_time;

        cout << npv + 1 - i << ". " << (color == 1 ? "Black" : "White") << " moves: "
             << "value=" << color * value
             << ", #expanded=" << expanded
             << ", #generated=" << generated
             << ", seconds=" << elapsed_time.count()
             << ", #generated/second=" << generated/elapsed_time.count()
             << endl;
        myfile << npv + 1 - i << ". " << (color == 1 ? "Black" : "White") << " moves: "
             << "value=" << color * value
             << ", #expanded=" << expanded
             << ", #generated=" << generated
             << ", seconds=" << elapsed_time.count()
             << ", #generated/second=" << generated/elapsed_time.count()
             << endl;
    }

    return 0;
}

