//
//  board.cpp
//  tictactoe
//
//  Created by Nissan Hajaj on 4/28/16.
//  Copyright © 2016 Nissan Hajaj. All rights reserved.
//

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <tuple>
#include "board.h"

using ::std::array;
using ::std::get;
using ::std::make_pair;
using ::std::pair;
using ::std::sort;
using ::std::vector;

namespace tictactoe {
  
  Board::Board(const vector<int>& field, const vector<int>& macroboard, int move) {
    for (int bb = 0; bb < 9; ++bb) {
      int bbi = bb / 3;
      int bbj = bb % 3;
      for (int b = 0; b < 9; ++b) {
        int bi = b / 3;
        int bj = b % 3;
        _board[bb].add(b, (Player) field[((bbi * 3 + bi) * 3 + bbj) * 3 + bj]);
      }
      _board[bb].update_value();
    }
    int last = -1;
    for (int bb = 0; bb < 9; ++bb) {
      if (macroboard[bb]) {
        if (last < 0) {
          last = bb;
        } else {
          last = -1;
          break;
        }
      }
    }
    _next_board.push_back(last);
  }
  
  void Board::SmallBoard::add(int pos, Player player) {
    board |= player << (2 * pos);
    ++count;
  }
  
  void  Board::SmallBoard::update_value() {
    winner = calc_winner();
    done = winner || (count == 9);
    value.push_back(eval());
  }

  void Board::SmallBoard::push(int pos, Board::Player player) {
    add(pos, player);
    update_value();
  }
  
  void Board::SmallBoard::pop(int pos, Player player) {
    board &= ~(11 << (2 * pos));
    --count;
    winner = NONE;
    done = false;
    value.pop_back();
  }
  
  namespace {
    array<double, 64> init_row_values() {
      static const double one_value = 0.05;
      static const double two_value = 0.3;
      array<double, 64> values = { 0 };
      for (int i = 0; i < 3; ++i) {
        int row = 1 << (2*i);
        values[row] = one_value;
        values[row << 1] = -one_value;
        for (int j = i + 1; j < 3; ++j) {
          int row2 = row | (1 << (2*j));
          values[row2] = two_value;
          values[row2 << 1] = -two_value;
        }
      }
      
      return values;
    }
  }
  double Board::SmallBoard::eval() const {
    if (done) {
      if (winner == NONE) {
        return 0;
      } else if (winner == FIRST) {
        return 1;
      } else {
        return -1;
      }
    }
    double value = 0;
    const int row_mask = 0x3f;
    static const array<double, 64> row_values = init_row_values();
    for (int shift = 0; shift < 18; shift += 6) {
      int row = (board & (row_mask << shift)) >> shift;
      value += row_values[row];
    }
    
    const int col_mask = 0x30c3;
    for (int shift = 0; shift < 6; shift += 2) {
      int col = (board & (col_mask << shift)) >> shift;
      value += row_values[row_mask & (col | col >> 4 | col >> 8)];
    }
    
    const int diag1_mask = 0x30303;
    int d = board & diag1_mask;
    value += row_values[row_mask & (d | d >> 6 | d >> 12)];
    const int diag2_mask = 0x3330;
    d = board & diag2_mask;
    value += row_values[row_mask & (d >> 4 | ((d >> 6) & 0xc) | ((d >> 8) & 0x30))];
    return value;
  }
    
  Board::Player Board::SmallBoard::calc_winner() {
    static const array<int, 8> masks = {0x15, 0x540, 0x1500, 0x1041, 0x4104, 0x10410, 0x10101, 0x1110};
    for (int m : masks) {
      if ((m & board) == m) {
        return FIRST;
      }
      if ((m & (board >> 1)) == m) {
        return SECOND;
      }
    }
    return NONE;
  }
  
  vector<pair<int, int>> Board::allowed_moves() const {
    vector<pair<int, int>> rv;
    if (_next_board.empty() || _board[_next_board.back()].done) {
      for (int i = 0; i < 9; ++i) {
        if (i != _next_board.back()) {
          for (int j = 0; j < 9; ++j) {
            if (_board[i].isfree(j)) {
              rv.push_back(make_pair(i, j));
            }
          }
        }
      }
    }
    return rv;
  }
  
#define WINNER(i) (_board[get<i>(m)].winner)
  Board::Player Board::winner() const {
    typedef ::std::tuple<int, int, int> Tri;
    static const array<Tri, 8> masks = { Tri(0, 1, 2), Tri(3, 4, 5), Tri(6, 7, 8),
      Tri(0, 3, 6), Tri(1, 4, 7), Tri(2, 5, 8),
      Tri(0, 4, 8), Tri(2, 4, 6)};
    for (auto& m : masks) {
      if (WINNER(0) && WINNER(0) == WINNER(1) && WINNER(0) == WINNER(2)) {
        return WINNER(0);
      }
    }
    return NONE;
  }
#undef WINNER
  
  bool Board::done() const {
    for (int i = 0; i < 9; ++i) {
      if (!_board[i].done) {
        return false;
      }
    }
    return true;
  }
  
  namespace {
    double eval_row_evals(double *e123) {
      static const double one_value = 0.05;
      static const double two_value = 0.3;
      
      sort(e123, e123 + 3);
      return 0.5 * (e123[0] + e123[2]) * (one_value + (two_value - one_value) * fabs(e123[1]));
    }
  }
  double Board::eval() const {
    Player win = winner();
    if (win == FIRST) {
      return 1;
    } else if (win == SECOND) {
      return -1;
    }
    if (done()) {
      return 0;
    }
    double value = 0.000001 * ((rand() % 100) - 50);
    double evals[9];
    for (int i = 0; i < 9; ++i) {
      evals[i] = _board[i].eval();
    }
    
    double row[3], col[3];
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        row[j] = evals[3 * i + j];
        col[j] = evals[3 * j + i];
      }
      value += eval_row_evals(row) + eval_row_evals(col);
    }
    row[0] = evals[0]; row[1] = evals[4]; row[2] = evals[8];
    col[0] = evals[2]; col[1] = evals[4]; col[2] = evals[6];
    value += eval_row_evals(row) + eval_row_evals(col);
    return value;
  }
  
}  // namespace tictactoe