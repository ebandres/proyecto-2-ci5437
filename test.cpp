// Testeando la representación. El primer movimiento corresponde al primero de la
// variación principal y el segundo fue para probar que el código de las diagonales funciona bien.
// & son las fichas negras, 0 son las blancas y . son las posiciones libres

#include "othello_cut.h"
#include <unordered_map>

struct stored_info_t {
    int value_;
    int type_;
    enum { EXACT, LOWER, UPPER };
    stored_info_t(int value = -100, int type = EXACT) : value_(value), type_(type) { }
};

struct hash_function_t {
    size_t operator()(const state_t &state) const {
        return state.hash();
    }
};

class hash_table_t : public std::unordered_map<state_t, stored_info_t, hash_function_t> {
};

hash_table_t TTable[2];

int main() {
	
	state_t state = state_t();
	state.print(std::cout, 3);
	state_t prev = state;

	stored_info_t info1 = stored_info_t(state.value());
	TTable[0][state] = info1;
	stored_info_t value = TTable[0].at(state);
	std::cout << value.value_ << std::endl;

	state = state.move(true,12);
	state.print(std::cout,1);
	stored_info_t info2 = stored_info_t(state.value());
	TTable[0][state] = info2;
	stored_info_t value2 = TTable[0].at(state);
	std::cout << value2.type_ << std::endl;

	stored_info_t value3 = TTable[0].at(prev);
	std::cout << value3.type_ << std::endl;



	state = state.move(false,11);

	if (TTable[0].find(state) == TTable[0].end()) {
		std::cout <<"NOP" << std::endl;
	}
	
	state.print(std::cout,1);

	return 0;
}

