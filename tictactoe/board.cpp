//
//  board.cpp
//  tictactoe
//
//  Created by Nissan Hajaj on 4/28/16.
//  Copyright © 2016 Nissan Hajaj. All rights reserved.
//

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include "board.h"

using ::std::array;
using ::std::chrono::milliseconds;
using ::std::chrono::steady_clock;
using ::std::get;
using ::std::greater;
using ::std::make_pair;
using ::std::min;
using ::std::pair;
using ::std::string;
using ::std::sort;
using ::std::to_string;
using ::std::vector;

namespace tictactoe {
  
  Board::Board(const vector<int>& field, const vector<int>& macroboard) {
    for (int bb = 0; bb < 9; ++bb) {
      int bbi = bb / 3;
      int bbj = bb % 3;
      for (int b = 0; b < 9; ++b) {
        int bi = b / 3;
        int bj = b % 3;
        board_[bb].add(b, (Player) field[((bbi * 3 + bi) * 3 + bbj) * 3 + bj]);
      }
      board_[bb].update_value();
      // if (board_[bb].done) {
      //   debug(to_string(bb) + " done = " + to_string(board_[bb].winner) + ", " + to_string(board_[bb].count));
      // }
    }
    int last = -1;
    for (int bb = 0; bb < 9; ++bb) {
      if (macroboard[bb] == -1) {
        if (last == -1) {
          last = bb;
        } else {
          last = -1;
          break;
        }
      }
    }
    next_board_.push_back(last);
    count_ = 0;
    for (const auto& b : board_) {
      count_ += b.count;
    }
  }
  
  void Board::SmallBoard::add(int pos, Player player) {
    if (player == NONE) return;
    board |= player << (2 * pos);
    ++count;
  }
  
  
  namespace {
    array<pair<double, double>, 64> init_row_values() {
      static const double mult = 0.125;
      static const double zero_value = mult * 0.125;
      static const double one_value = mult * 0.25;
      static const double two_value = mult * 0.5;
      array<pair<double, double>, 64> values = { pair<double, double>(0., 0.) };
      values[0].first =  values[0].second = zero_value;
      for (int i = 0; i < 3; ++i) {
        int row = 1 << (2*i);
        values[row].first = one_value;
        values[row << 1].second = one_value;
        for (int j = i + 1; j < 3; ++j) {
          int row2 = row | (1 << (2*j));
          values[row2].first = two_value;
          values[row2 << 1].second = two_value;
        }
      }
      
      return values;
    }
    
    void sum_pair(const pair<double, double>& p, pair<double, double>* tp) {
      tp->first += p.first;
      tp->second += p.second;
    }
    
    pair<double, double> calc_eval(const Board::SmallBoard& sb) {
      if (sb.done) {
        if (sb.winner == Board::NONE) {
          return make_pair(0., 0.);
        } else if (sb.winner == Board::FIRST) {
          return make_pair(1., 0.);
        } else {
          return make_pair(0., 1.);
        }
      }
      pair<double, double> value(0, 0);
      const int row_mask = 0x3f;
      static const array<pair<double, double>, 64> row_values = init_row_values();
      for (int shift = 0; shift < 18; shift += 6) {
        int row = (sb.board & (row_mask << shift)) >> shift;
        sum_pair(row_values[row], &value);
      }
      
      const int col_mask = 0x30c3;
      for (int shift = 0; shift < 6; shift += 2) {
        int col = (sb.board & (col_mask << shift)) >> shift;
        sum_pair(row_values[row_mask & (col | col >> 4 | col >> 8)], &value);
      }
      
      const int diag1_mask = 0x30303;
      int d = sb.board & diag1_mask;
      sum_pair(row_values[row_mask & (d | d >> 6 | d >> 12)], &value);
      const int diag2_mask = 0x3330;
      d = sb.board & diag2_mask;
      sum_pair(row_values[((d >> 4) & 0x3) | ((d >> 6) & 0xc) | ((d >> 8) & 0x30)], &value);
      return value;
    }
    
  }  // namespace
  
  
  void  Board::SmallBoard::update_value() {
    winner = calc_winner();
    done = winner || (count == 9);
    value.push_back(calc_eval(*this));
  }
  
  void Board::SmallBoard::push(int pos, Board::Player player) {
    add(pos, player);
    update_value();
  }
  
  void Board::SmallBoard::pop(int pos) {
    board &= ~(0x3 << (2 * pos));
    --count;
    winner = NONE;
    done = false;
    value.pop_back();
  }
  
  
  Board::Player Board::SmallBoard::calc_winner() {
    static const array<int, 8> masks = {0x15, 0x540, 0x15000, 0x1041, 0x4104, 0x10410, 0x10101, 0x1110};
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
  
  vector<Board::Move> Board::allowed_moves() const {
    vector<pair<int, int>> rv;
    if (next_board_.back() == -1 || board_[next_board_.back()].done) {
      for (int i = 0; i < 9; ++i) {
        if (!board_[i].done) {
          for (int j = 0; j < 9; ++j) {
            if (board_[i].isfree(j)) {
              rv.push_back(make_pair(i, j));
            }
          }
        }
      }
    } else {
      const int i = next_board_.back();
      for (int j = 0; j < 9; ++j) {
        if (board_[i].isfree(j)) {
          rv.push_back(make_pair(i, j));
        }
      }
    }
    /*
     if (rv.empty()) {
     debug(">>> no moves: " + to_string(next_board_.back()) + "\n" + toString());
     }
     */
    return rv;
  }
  
  Board::Move Board::get_best_move(Board::Player player, int level, int time_ms) {
    start_time_ = steady_clock::now();
    remaining_time_ms_ = time_ms;
    if (count_ == 0) {
      return Move(4, 4);
    }
    
    pair<Board::Move, double> best = get_best_rec(player, level, player == FIRST ? 2. : -2);
    // debug("best move value: " + to_string(best.second));
    last_value_ = best.second;
    return best.first;
  }
  
#define OTHER(p) ((Player) (3 - p))
  pair<Board::Move, double> Board::get_best_rec(Player player, int level, double prune_value) {
    vector<pair<pair<double, bool>, Board::Move>> evals;
    {
      vector<Board::Move> moves = allowed_moves();
      evals.reserve(moves.size());
      for (const auto& move : moves) {
        board_[move.first].push(move.second, player);
        next_board_.push_back(move.first);
        evals.push_back(make_pair(eval(), move));
        next_board_.pop_back();
        board_[move.first].pop(move.second);
      }
    }
    if (player == FIRST) {
      sort(evals.begin(), evals.end(), greater<pair<pair<double, bool>, Board::Move>>());
    } else {
      sort(evals.begin(), evals.end());
    }
    pair<Board::Move, double> best(make_pair(make_pair(-1, -1), (player == FIRST) ? -2. : 2.));
    if (level > 0) {
      for (const auto& eval_move : evals) {
        const Move& move = eval_move.second;
        pair<Board::Move, double> move_value;
        if (eval_move.first.second) {
          // done
          move_value.first = eval_move.second;
          move_value.second = eval_move.first.first;
        } else {
          board_[move.first].push(move.second, player);
          next_board_.push_back(move.second);
          move_value = Board::get_best_rec(OTHER(player), level - 1, best.second);
          next_board_.pop_back();
          board_[move.first].pop(move.second);
        }
        if (player == FIRST) {
          if (move_value.second > best.second) {
            best.first = move;
            best.second = move_value.second;
            if (best.second > prune_value) {
              break;
            }
          }
        } else {
          if (move_value.second < best.second) {
            best.first = move;
            best.second = move_value.second;
            if (best.second < prune_value) {
              break;
            }
          }
        }
        if ((steady_clock::now() - start_time_) > milliseconds(remaining_time_ms_)) {
          // debug("break");
          // debug(to_string((steady_clock::now() - start_time_).count()));
          break;
        }
      }
    } else {
      best.first = evals.front().second;
      best.second = evals.front().first.first;
    }
    //debug("first " + to_string(level) + ", " + to_string(best.second));
    return best;
  }
#undef OTHER
  
#define WINNER(i) (board_[get<i>(m)].winner)
#define WINNER_ROW (WINNER(0) && WINNER(0) == WINNER(1) && WINNER(0) == WINNER(2))
  Board::Player Board::winner() const {
    typedef ::std::tuple<int, int, int> Tri;
    static const array<Tri, 8> masks = { Tri(0, 1, 2), Tri(3, 4, 5), Tri(6, 7, 8),
      Tri(0, 3, 6), Tri(1, 4, 7), Tri(2, 5, 8),
      Tri(0, 4, 8), Tri(2, 4, 6)};
    for (auto& m : masks) {
      if (WINNER_ROW) {
        return WINNER(0);
      }
    }
    return NONE;
  }
#undef WINNER_ROW
#undef WINNER
  
  bool Board::done() const {
    for (int i = 0; i < 9; ++i) {
      if (!board_[i].done) {
        return false;
      }
    }
    return true;
  }
  
  namespace {
    inline double eval_row_evals(const pair<double, double> *e123) {
      static const double multiplier = 0.125;
      return multiplier * (e123[0].first * e123[1].first * e123[2].first -
                           e123[0].second * e123[1].second * e123[2].second);
    }
  }
  
  pair<double, bool> Board::eval() const {
    Player win = winner();
    if (win == FIRST) {
      return make_pair(1., true);
    } else if (win == SECOND) {
      return make_pair(-1., true);
    }
    if (done()) {
      return make_pair(0., true);
    }
    double value = 0.00000001 * ((rand() % 100) - 50);
    pair<double, double> evals[9];
    for (int i = 0; i < 9; ++i) {
      evals[i] = board_[i].value.back();
      /*
       if (evals[i].first > 1.0 || evals[i].second > 1.0) {
       debug(">>> eval[" + to_string(i) +"]= " + to_string(evals[i].first) + ", " + to_string(evals[i].second) + "\n");
       string out;
       for (int ii = 0; ii < 3; ++ii) {
       for (int jj = 0; jj < 3; ++jj) {
       out += board_[i].at(3*ii + jj) == NONE ? "_ " : (((board_[i].at(3*ii + jj) == FIRST) ? "X " : "O "));
       }
       out += "\n";
       }
       debug(out);
       }
       */
    }
    
    pair<double, double> row[3], col[3];
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
    /*
     if (fabs(value) >= 1.0) {
     debug(">>> eval= " + to_string(value) + "\n" + toString());
     }
     */
    return make_pair(value, false);
  }
  
  string Board::toString() const {
    string out;
    for (int i = 0; i < 3; ++i) {
      for (int ii = 0; ii < 3; ++ii) {
        for (int j = 0; j < 3; ++j) {
          for(int jj = 0; jj < 3; ++jj) {
            const Player player = board_[3*i + j].at(3*ii + jj);
            out += (player == NONE) ? "_ " : (player == FIRST) ? "X " : "O ";
          }
          out += " ";
        }
        out += "\n";
      }
      out += "\n";
    }
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        const Player winner = board_[3*i + j].winner;
        out += board_[3*i + j].done ? ((winner == NONE) ? "* " : (winner == FIRST) ? "X " : "O ") : "_ ";
      }
      out += "\n";
    }
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        // if (board_[3*i + j].value.empty()) {
        //   debug("empty value");
        // }
        const pair<double, double>& value = board_[3*i + j].value.back();
        out += "(" + to_string(value.first) + ", " +  to_string(value.second) + ") ";
      }
      out += "\n";
    }
    
    out += "next: " + to_string(next_board_.back()) + "\n";
    out += "eval: " + to_string(eval().first) + "\n";
    out += "last full eval: " + to_string(last_value_) + "\n";
    return out;
  }
  
  void Board::debug(const string &s) const{
    ::std::cerr << s << ::std::endl << ::std::flush;
  }
  
}  // namespace tictactoe