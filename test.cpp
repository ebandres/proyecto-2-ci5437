// Testeando la representación. El primer movimiento corresponde al primero de la
// variación principal y el segundo fue para probar que el código de las diagonales funciona bien.
// & son las fichas negras, 0 son las blancas y . son las posiciones libres

#include "othello_cut.h"

int main() {
	
	state_t state = state_t();
	state.print(std::cout, 3);

	state = state.move(true,12);
	state.print(std::cout,1);

	state = state.move(false,11);
	state.print(std::cout,1);

	return 0;
}

