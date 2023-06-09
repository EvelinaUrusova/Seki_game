//  Алгоритм решения игры Секи, использующий альфа-бета отсечение (рекурсивный поиск)
//  Суть в поиске в грубину, при том, что некоторые позиции мы из теоретических соображений
//  умеем оценивать, например "диагональные" позиции - когда в каждой строке и столбце
//  стоит ровно одно ненулевое число - тогда игра форсирована
//  кроме того, представим, что матрица такова, что из неё можно выделить два "диагональных" подмножества, а остальные - нули
//  Рассматривая это как независимые игры, мы точно знаем, что если побеждаем в обеих - то побеждаем и в целом
// Аналогично если можно разбить на большее кол-во "диагональных" игр, и в каждой у нас стоит победа, то мы выигрываем и в целом
// это ещё не реализовано, последняя часть

#ifndef SOLVER_H
#define SOLVER_H

#include <stdlib.h>
#include <vector>
#include <array>
#include <iostream>

namespace core {

// Maximum width and height of the field
    constexpr size_t max_field_dim = 6;

// Class that contains all the data about the Move
// can be compared by value
    class Move {
    public:
        float value;
        int x, y;
        bool is_pass;
        // конструкторы
        Move() : x(0), y(0) {}
        Move(float value, int x, int y);

        // Compares values
        bool operator>(const Move &other) const;

        bool operator>=(const Move &other) const;

        bool operator<(const Move &other) const;

        bool operator<=(const Move &other) const;

    };

// Contains state of the field and utility functions
    class Field {
    private:
        std::array<int, 2> shape;
        // [row][col]
        std::array<std::array<int, max_field_dim>, max_field_dim> data;
        std::array<int, max_field_dim> row_sum, col_sum;
    public:
        Field(const std::vector<std::vector<int>> &new_data);
        //добавить число v в клетку x,y
        void add(int x, int y, int v);
        // получить значение из клетки (x,y)
        int get(int x, int y) const;

        std::array<int, 2> get_shape() const;

        // возвращают номер минимальной колонки/строки
        int get_min_row() const;

        int get_min_col() const;

        // если есть ряд/колонка из одних нулей, вернёт истину
        bool has_zero_row() const;

        bool has_zero_col() const;

        //функции, возвращающие единственное ненулевое значение в строке/столбце, а если таковое не одно, то -1
        int row_has_one(int num) const;

        int col_has_one(int num) const;

        // проверка, является ли матрица "диагональной" используемом здесь смысле
        bool is_diagonal() const;
        // возвращает сумму чисел в матрице
        int mat_sum() const;

        const std::array<int, max_field_dim> &get_row_sum() const;

        const std::array<int, max_field_dim> &get_col_sum() const;

        friend std::ostream &operator<<(std::ostream &s, const Field &f);

    };
    //политика паса
    enum PassType {
        NO_PASS = 0,
        R_PASS = 1,
        C_PASS = 2,
        ANY_PASS = 3,
    };
    //тип игры
    enum SekiType {
        SEKI = 0,
        DSEKI,
    };


// Stores everything needed for the current state of the game
    class GameState {
    private:
        Field field;
        bool is_r;  //чей ход
        int depth;  //глубина
        int pass_count;
        const SekiType type;
        const PassType pass_policy;
    public:

        GameState(Field field, bool is_r, int depth, SekiType p_type,
                  PassType p_pass_policy);

        const Field &get_field() const;

        bool get_is_r() const;

        int get_depth() const;

        void set_pass_count(int p_pass_count) { pass_count = p_pass_count; }

        int get_pass_count() const;

        void apply_move(const Move &m);

        float get_value() const;  //выдать эвристическую оценку позиции

        //закончена ли игра в этой позиции
        bool is_terminal() const;

        // - является ли позиция 
        bool is_win_position() const;


        enum Status {
            Ongoing,
            R_won,
            C_won,
            Draw,
        };

        Status get_status() const;

        SekiType get_type() const { return type; }

        PassType get_passtype() const { return pass_policy; }
        //выдаёт массив возможных ходов
        std::vector<Move> get_moves() const;
    };

// Class that solves the game
    class SekiSolver {
    private:
        GameState state;
    public:

        // Amount of states that were looked at on the last call of find optimal
        int unrolled;
        SekiSolver(GameState p_state);

        void apply_move(Move m);
        
        //основная функция, ищет оптимальный ход, принимая состояние игры, лучший и худший ходы текущие
        Move _find_optimal_impl(const GameState &state, Move alpha, Move beta);
        Move _find_optimal_impl1(const GameState &state, Move alpha, Move beta);

        //возвращает _find_optimal_impl
        Move find_optimal();

        const GameState &get_state() const;

    };

}

#endif // DIRECT_SOLVER_H
