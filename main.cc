// Game of Othello -- Example of main
// Universidad Simon Bolivar, 2012.
// Author: Blai Bonet
// Last Revision: 1/11/16
// Modified by: 

#include <iostream>
#include <limits>
#include "othello_cut.h" // won't work correctly until .h is fixed!
#include "utils.h"

#include <unordered_map>

using namespace std;

unsigned expanded = 0;
unsigned generated = 0;
int tt_threshold = 32; // threshold to save entries in TT

// Transposition table (it is not necessary to implement TT)
struct stored_info_t {
    int value_;
    int type_;
    enum { EXACT, LOWER, UPPER };
    stored_info_t(int value = -100, int type = LOWER) : value_(value), type_(type) { }
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

int negamax(state_t state, int depth, int color, bool use_tt = false){
    ++generated;
    if (state.terminal())
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
            alpha = max(alpha, -negamax(child, ++depth, -color, use_tt));
        }
    }

    else
    {
        // Sin modificaciones al estado, el otro color juega con el mismo estado.
        alpha = max(alpha, -negamax(state, ++depth, -color, use_tt));
    }
    return alpha;
};


int negamax_alphabeta(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false){

    int score;
    int val;
    ++generated;

    if (state.terminal())
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
            val = -negamax_alphabeta(child, ++depth, -beta, -alpha, -color, use_tt);
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
        val = -negamax_alphabeta(state, ++depth, -beta, -alpha, -color, use_tt);
        score = max(score,val);
    }
    return score;
};

int scout(state_t state, int depth, int color, bool use_tt = false){
    int score = 0;
    int firstChild = 1;

    if (depth == 0 || state.terminal()) {
        return state.value();
    }

    vector<state_t> child_states = child_vector(state,color);

    if (child_states.size() != 0) {

        for (state_t child : child_states) {

            if (firstChild) {

                firstChild = 0;
                score = scout(child, depth -1, -color, use_tt);

            } else {

                if (color == 1 && test(child, depth, score, 1, color)) {
                    score = scout(child, depth - 1, -color, use_tt);
                }
                if (color != 1 && !test(child, depth, score, 0, color)) {
                    score = scout(child, depth -1, -color, use_tt);
                }
            }
        }
        
    } else {
        scout(state, depth - 1, -color, use_tt);
    }
    return score;
}

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
    }
    return !(color == 1);
}


int negascout(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false){
    int score;
    int frstChild = 1;

    ++generated;
    if (state.terminal())
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
        score = -negascout(state, ++depth,-beta,-alpha,-color);
        alpha = max(alpha, score);
    }
    else{
        for (state_t child : children) 
        {
            if (frstChild)
            {
                frstChild = 0;
                score = -negascout(child, ++depth, -beta, -alpha, -color);
            }
            else
            {
                score = -negascout(child, ++depth, -alpha - 1, -alpha, -color);
                if ((alpha < score) && (score < beta))
                {
                    score = -negascout(child, ++depth,-beta,-score,-color);
                }
            }

            alpha = std::max(alpha,score);
            if (alpha >= beta)
            {
                break;
            }
        }
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
    if( algorithm == 1 )
        cout << "Negamax (minmax version)";
    else if( algorithm == 2 )
        cout << "Negamax (alpha-beta version)";
    else if( algorithm == 3 )
        cout << "Scout";
    else if( algorithm == 4 )
        cout << "Negascout";
    cout << (use_tt ? " w/ transposition table" : "") << endl;

    // Run algorithm along PV (bacwards)
    cout << "Moving along PV:" << endl;
    for( int i = 0; i <= npv; ++i ) {
        //cout << pv[i];
        int value = 0;
        TTable[0].clear();
        TTable[1].clear();
        float start_time = Utils::read_time_in_seconds();
        expanded = 0;
        generated = 0;
        int color = i % 2 == 1 ? 1 : -1;

        try {
            if( algorithm == 1 ) {
                value = negamax(pv[i], 0, color, use_tt);
            } else if( algorithm == 2 ) {
                value = negamax_alphabeta(pv[i], 0, -200, 200, color, use_tt);
            } else if( algorithm == 3 ) {
                //value = scout(pv[i], 0, color, use_tt);
            } else if( algorithm == 4 ) {
                value = negascout(pv[i], 0, -200, 200, color, use_tt);
            }
        } catch( const bad_alloc &e ) {
            cout << "size TT[0]: size=" << TTable[0].size() << ", #buckets=" << TTable[0].bucket_count() << endl;
            cout << "size TT[1]: size=" << TTable[1].size() << ", #buckets=" << TTable[1].bucket_count() << endl;
            use_tt = false;
        }

        float elapsed_time = Utils::read_time_in_seconds() - start_time;

        cout << npv + 1 - i << ". " << (color == 1 ? "Black" : "White") << " moves: "
             << "value=" << color * value
             << ", #expanded=" << expanded
             << ", #generated=" << generated
             << ", seconds=" << elapsed_time
             << ", #generated/second=" << generated/elapsed_time
             << endl;
    }

    return 0;
}

