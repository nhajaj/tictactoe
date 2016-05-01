//
//  board.h
//  tictactoe
//
//  Created by Nissan Hajaj on 4/27/16.
//  Copyright © 2016 Nissan Hajaj. All rights reserved.
//

#ifndef board_h
#define board_h

#include <utility>
#include <vector>

namespace tictactoe {
  class Board {
  public:
    Board(const ::std::vector<int>& field, const ::std::vector<int>& macroboard, int move);
    enum Player {NONE =  0, FIRST = 1, SECOND = 2};
    ::std::vector<::std::pair<int, int>> allowed_moves() const;
    double eval() const;
    
  private:
    Player winner() const;
    bool done() const;

    struct SmallBoard {
      int board = 0;
      bool done = false;
      Player winner = NONE;
      ::std::vector<double> value;
      int count = 0;
      
      void add(int pos, Player player);
      void update_value();
      void push(int pos, Player player);
      void pop(int pos, Player player);
      double get_value() { return value.empty() ? 0 : value.back(); }
      Player at(int pos) const { return (Player) ((board & (11 << (2 * pos))) >> (2 * pos)); }
      bool isfree(int pos) const { return !(board & (11 << (2 * pos))); }
      double eval() const;
      Player calc_winner();
    

    };
    SmallBoard _board[9];
    ::std::vector<int> _next_board;
  };
  
}  // namespace tictactoe

#endif /* board_h */
