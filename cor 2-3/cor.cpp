#include <iostream>
#include "solver.h"


int main() {
    // Инициализация игрового состояния
    std::vector<std::vector<int>> initial_field = {
            {16, 0, 0, 0, 0, 0},
            {0, 1, 0, 0, 0, 0},
            {0, 0, 1, 0, 0, 0},
            {0, 0, 0, 1, 0, 0},
            {0, 0, 0, 0, 1, 0},
            {0, 0, 0, 0, 0, 1}
    };

    std::vector<std::vector<int>> initial_field2 = {
            {1, 0, 0},
            {0, 0, 2},
            {0, 1, 1}
    };

    core::Field field(initial_field2);
    core::GameState state(field, false, 0, core::SekiType::DSEKI, core::PassType::NO_PASS);

    // Создание solver с начальным состоянием игры
    core::SekiSolver solver(state);

    // Поиск оптимального хода
    core::Move optimal_move = solver.find_optimal();
    std::cout << "Optimal move: (" << optimal_move.x << ", " << optimal_move.y << "), value: " << optimal_move.value;
    if(optimal_move.is_pass) 
    std::cout << " it was pass" << std::endl;
    return 0;
}


