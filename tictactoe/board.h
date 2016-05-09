//
//  board.h
//  tictactoe
//
//  Created by Nissan Hajaj on 4/27/16.
//  Copyright © 2016 Nissan Hajaj. All rights reserved.
//

#ifndef board_h
#define board_h
#include <chrono>
#include <utility>
#include <vector>

namespace tictactoe {
  class Board {
  public:
    Board(const ::std::vector<int>& field, const ::std::vector<int>& macroboard);
    enum Player {NONE =  0, FIRST = 1, SECOND = 2};
    typedef ::std::pair<int, int> Move;
    ::std::vector<Move> allowed_moves() const;
    ::std::pair<double, bool> eval() const;
    Move get_best_move(Player player, int level, int time_ms = 86400000);
    ::std::string toString() const;
    void debug(const ::std::string &s) const;
    int count() const { return count_; }
    
    struct SmallBoard {
      int board = 0;
      bool done = false;
      Player winner = NONE;
      ::std::vector<::std::pair<double, double>> value;
      int count = 0;
      
      void add(int pos, Player player);
      void update_value();
      void push(int pos, Player player);
      void pop(int pos);
      double get_value1() const { return value.empty() ? 0 : value.back().first; }
      double get_value2() const { return value.empty() ? 0 : value.back().second; }
      
      Player at(int pos) const { return (Player) ((board & (0x3 << (2 * pos))) >> (2 * pos)); }
      bool isfree(int pos) const { return !(board & (0x3 << (2 * pos))); }
      Player calc_winner();
    };
    
  private:
    Player winner() const;
    bool done() const;
    ::std::pair<Move, double> get_best_rec(Player player, int level, double prune_value);
    
    
    SmallBoard board_[9];
    ::std::vector<int> next_board_;
    int count_;
    ::std::chrono::steady_clock::time_point start_time_;
    int remaining_time_ms_;
    double last_value_ = 0;
  };
  
}  // namespace tictactoe

#endif /* board_h */
