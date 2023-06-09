#include "solver.h"
#include <numeric>
#include <algorithm>
#include <assert.h>
#include <random>

using namespace std;

namespace core {

    Move::Move(float value, int x, int y) {
        this->value = value;
        this->x = x;
        this->y = y;
        this->is_pass = false;
    }

    bool Move::operator>(const Move &other) const {
        return value > other.value;
    }

    bool Move::operator>=(const Move &other) const {
        return value >= other.value;
    }

    bool Move::operator<(const Move &other) const {
        return value < other.value;
    }

    bool Move::operator<=(const Move &other) const {
        return value <= other.value;
    }

    Field::Field(const vector <vector<int>> &new_data) {
        for (size_t i = 0; i < new_data.size(); ++i) {
            auto row = new_data[i];
            for (size_t j = 0; j < row.size(); ++j) {
                data[i][j] = row[j];
            }
        }
        int n_rows = new_data.size();
        int n_cols = new_data[0].size();
        shape[0] = n_rows;
        shape[1] = n_cols;
        // row_sum.resize(n_rows);
        // col_sum.resize(n_cols);
        for (int i = 0; i < n_rows; ++i) {
            row_sum[i] = accumulate(new_data[i].begin(), new_data[i].end(), 0);
        }
        for (int i = 0; i < n_cols; ++i) {
            col_sum[i] = 0;
            for (int j = 0; j < n_rows; ++j) {
                col_sum[i] += new_data[j][i];
            }
        }
    }

    void Field::add(int x, int y, int v) {
        data[y][x] += v;
        row_sum[y] += v;
        col_sum[x] += v;
    }

    int Field::get(int x, int y) const {
        return data[y][x];
    }

    array<int, 2> Field::get_shape() const {
        return this->shape;
    }

    int Field::get_min_row() const {
        auto it = min_element(row_sum.begin(), row_sum.begin() + shape[0]);
        return *it;
    }

    int Field::get_min_col() const {
        auto it = min_element(col_sum.begin(), col_sum.begin() + shape[1]);
        return *it;
    }

    bool Field::has_zero_row() const {
        return get_min_row() == 0;
    }

    bool Field::has_zero_col() const {
        return get_min_col() == 0;
    }

    int Field::col_has_one(int num) const {
        int c = 0;
        int ret = 0;
        for (int i = 0; i < shape[0]; ++i) {
            if (data[num][i] != 0) {
                ret += 1;
                c = data[num][i];
            }
        }
        if (ret != 1) {
            return -1;
        } else { return c; }
    }

    int Field::row_has_one(int num) const {
        int c = 0;
        int ret = 0;
        for (int i = 0; i < shape[1]; ++i) {
            if (data[i][num] != 0) {
                ret += 1;
                c = data[i][num];
            }
        }
        if (ret != 1) {
            return -1;
        } else { return c; }
    }

    bool Field::is_diagonal() const {
        for (int i = 0; i < shape[0]; i++) {
            if( ( !(row_has_one(i)==1) ) || (!(col_has_one(i)==1) ) )
                return false;
        }
        return true;
    }

    int Field::mat_sum() const {
       int ssum = 0;
       for(int i=0; i<shape[0]; i++)
       ssum+= row_sum[i];
       return ssum;

    }

    bool GameState::is_win_position() const
    {
        int c=0;
        auto shape = field.get_shape();
        if (field.is_diagonal())
        {
            for(int i =0; i< shape[0]; i++)
                {c+= field.get_row_sum()[i];
                if (field.get_row_sum()[i]==1)
                 return get_is_r();
                }
            if (c%2 == 0)
                return get_is_r();
        }
        return false;
    }

    const std::array<int, max_field_dim> &Field::get_row_sum() const {
        return row_sum;
    }

    const std::array<int, max_field_dim> &Field::get_col_sum() const {
        return col_sum;
    }

    vector <Move> GameState::get_moves() const {
        vector<Move> moves;

        auto shape = field.get_shape();

        for (int i = 0; i < shape[0]; ++i) {
            for (int j = 0; j < shape[1]; ++j) {
                if (field.get(j, i) != 0) {
                    moves.emplace_back(0, j, i);
                }
            }
        }

        auto row_sum = field.get_row_sum();
        auto col_sum = field.get_col_sum();
        std::random_device rd;
        std::mt19937 g(rd());
        // Shuffle the array to have true randomness
        std::shuffle(moves.begin(), moves.end(), g);
        // Partition array into good moves and bad moves

        //указатель на то место в массиве, где кончаются ходы с положительным value и начинаются с отрицательым
        auto it = partition(moves.begin(), moves.end(),
                            [this, &row_sum, &col_sum](const Move &m1) {
                                int value = row_sum[m1.y] - col_sum[m1.x];  //нужная эвристика
                                if (!is_r) value = -value;
                                return value < 0;
                            }); 

        auto move_value = [this, &row_sum, &col_sum](const Move &m1) {
            if ((this->get_field().is_diagonal())) { return is_r && is_win_position() ? +1000 : -1000; }
            else
                return is_r ? row_sum[m1.y] : col_sum[m1.x];
        };

        auto compare_moves = [move_value](const Move &m1, const Move &m2) {
            return move_value(m1) < move_value(m2);
        };

        // Sort first half of the moves
        sort(moves.begin(), it, compare_moves);
        // Sort second half of the moves
        sort(it, moves.end(), compare_moves);
        //moves = {moves[0],moves[1],moves[2],moves[3] }; //берём 4 лучших хода

        Move pass = Move(0.0, 0, 0);
        pass.is_pass = true;

        if ((is_r && pass_policy & PassType::R_PASS)
            || (!is_r && pass_policy & PassType::C_PASS)) {
            moves.insert(it, pass);
        }
        //берём по 3 хода только
      /*  vector<Move> vec(6);
        int nn = moves.size();
        if (nn > 5)
    {   vec[0] = moves[0];
        vec[1] = moves[1];
        vec[2] = moves[2];
        vec[6] = moves[nn-1];
        vec[5] = moves[nn-2];
        vec[4] = moves[nn-3];
        return vec; 
        
        }*/

        return moves;
    }

    GameState::GameState(Field field, bool is_r, int depth, SekiType p_type,
                         PassType p_pass_policy)
            : field(field), is_r(is_r), depth(depth), pass_count(0),
              type(p_type), pass_policy(p_pass_policy) {}

    const Field &GameState::get_field() const {
        return field;
    }

    bool GameState::get_is_r() const {
        return is_r;
    }

    int GameState::get_depth() const {
        return depth;
    }

    int GameState::get_pass_count() const {
        return pass_count;
    }

    float seki_eval_func(const GameState &state) {

        // Draw in case of double pass
        if (state.get_pass_count() == 2) {
            return 0.0;
        }

        bool r_won = false;
        auto f = state.get_field();

        if (f.has_zero_col() && f.has_zero_row()) {
            r_won = !state.get_is_r();
        } else if (f.has_zero_row()) {
            r_won = true;
        } else {
            r_won = false;
        }

        if(f.is_diagonal()){
            r_won = state.get_is_r();
            float val=0;
            if(state.is_win_position())
            {
                val = r_won ? 1 : -1;
            }
            else{
                val = r_won ? -1 : 1;
            }
            return val; 
            } 
        else{ 

        float value = r_won ? -1 : 1;
        return value / state.get_depth();
        }
    }

    float dseki_eval_func(const GameState &state) {
        auto f = state.get_field();
        if (f.has_zero_col() && f.has_zero_row()) {
            return 0;
        }
        return seki_eval_func(state);
    }

    float GameState::get_value() const {
        switch (type) {
            case SekiType::SEKI:
                return seki_eval_func(*this);
                break;
            case SekiType::DSEKI:
                return dseki_eval_func(*this);
                break;
            default:
                assert(false);
        }
    }

    GameState::Status GameState::get_status() const {
        // TODO dseki
        if (!is_terminal()) {
            return Status::Ongoing;
        }
        float v = get_value();
        if (v < 0) {
            return Status::R_won;
        } else if (v > 0) {
            return Status::C_won;
        } else {
            return Status::Draw;
        }
    }


    void GameState::apply_move(const Move &m) {
        if (!m.is_pass) {
            field.add(m.x, m.y, -1);
            pass_count = 0;
        } else {
            ++pass_count;
        }
        is_r = !is_r;
        ++depth;
    }

    bool GameState::is_terminal() const {
        return field.has_zero_col() || field.has_zero_row() || pass_count == 2;
    }


    ostream &operator<<(ostream &s, const Field &f) {
        for (int i = 0; i < f.shape[0]; ++i) {
            for (int j = 0; j < f.shape[1]; ++j) {
                s << f.data[i][j] << " ";
            }
            s << "\n";
        }
        return s;
    }

    float get_guarantee(const GameState &state) {
        auto f = state.get_field();
        if (state.get_is_r()) {
            return 1.0 / (float) (state.get_depth() + f.get_min_col());
        } else {
            return -1.0 / (float) (state.get_depth() + f.get_min_row());
        }
    }

    SekiSolver::SekiSolver(GameState p_state)
            : state(p_state) {
        unrolled = 0;
    }

    void SekiSolver::apply_move(Move m) {
        state.apply_move(m);
    }

    Move SekiSolver::_find_optimal_impl(const GameState &state,
                                        Move alpha, Move beta) {
        

        if( state.get_field().mat_sum() > 18 )
        {
            auto f = state.get_field();
            int c = -1000; int im=0; int in=0;
            bool t= state.get_is_r();
            if(t)
            c=1000;
            for(int i=0; i<f.get_shape()[0]; i++)
            for(int j=0; j<f.get_shape()[0]; j++)
            {
                if(f.get(i,j)>0)
                {
                if(t )
                {
                    if  (f.get_col_sum()[i]-f.get_row_sum()[j] < c)  
                    {c=f.get_col_sum()[i]-f.get_row_sum()[j]; im=i; in=j;}
                }
                else
                {
                    if (f.get_col_sum()[i]-f.get_row_sum()[j] > c)
                    {c=f.get_col_sum()[i]-f.get_row_sum()[j]; im=i; in=j;}
                }
                }

            }
            float vl = (float)c/(f.mat_sum());
            return Move(vl, im,in);
        }
        else
        {Move m=  SekiSolver::_find_optimal_impl1(state, alpha, beta);
        return m;}
       
        
    }

    
    Move SekiSolver::_find_optimal_impl1(const GameState &state,
    Move alpha, Move beta) {
  if (state.is_terminal()) {
    float fv = state.get_value();
    return Move(fv, 0, 0);
  }
 
  auto value = Move(state.get_is_r() ? 2 : -2, 0, 0);

  auto update_bounds_and_value =
    [&value, &alpha, &beta, &state] (const Move &new_value) {
    if (state.get_is_r()) {
      value = min(value, new_value);
      beta = min(beta, value);
    } else {
      value = max(value, new_value);
      alpha = max(alpha, value);
    }
  };

  auto should_prune = [&alpha, &beta] () {
    return alpha >= beta;
  };

  {
    float gurantee = get_guarantee(state);
    auto g = Move(gurantee, 0, 0);
    update_bounds_and_value(g);
    if (should_prune())
      return value;
  }

  unrolled += 1;

  for (Move m : state.get_moves()) {
    GameState new_state = state;
    new_state.apply_move(m);
    Move new_value = _find_optimal_impl(new_state,
        alpha, beta);
    m.value = new_value.value;
    update_bounds_and_value(m);
    if (should_prune())
      break;
  }
  return value;
}


    Move SekiSolver::find_optimal() {
        unrolled = 0;
        Move opt = _find_optimal_impl(state, Move(-2, 0, 0), Move(2, 0, 0));
        return opt;
    }

    const GameState &SekiSolver::get_state() const {
        return state;
    }

}