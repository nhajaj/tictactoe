#include <iostream>
#include <algorithm>
#include <sstream>
#include <vector>

#include "board.h"

using ::std::cerr;
using ::std::cin;
using ::std::cout;
using ::std::endl;
using ::std::flush;
using ::std::getline;
using ::std::istringstream;
using ::std::make_pair;
using ::std::min;
using ::std::pair;
using ::std::string;
using ::std::stringstream;
using ::std::transform;
using ::std::vector;


namespace tictactoe {
  
  vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    elems.clear();
    while (getline(ss, item, delim)) {
      elems.push_back(item);
    }
    return elems;
  }
  
  int stringToInt(const string &s) {
    istringstream ss(s);
    int result;
    ss >> result;
    return result;
  }
  
  class BotIO {
  public:
    
    /**
     * Initialize your bot here.
     */
    BotIO() {
      srand(static_cast<unsigned int>(time(0)));
      _field.resize(81);
      _macroboard.resize(9);
    }
    
    
    void loop() {
      string line;
      vector<string> command;
      command.reserve(256);
      
      while (getline(cin, line)) {
        processCommand(split(line, ' ', command));
      }
    }
    
  private:
    pair<int, int> action(const string &type, int time) {
      /**
       * Implement this function.
       * type is always "move"
       *
       * return value must be position in x,y presentation
       *      (use std::make_pair(x, y))
       */
      return getMove(time);
    }
    
    pair<int, int> getMove(int time) const {
      Board board(_field, _macroboard);
      // debug(board.toString());
      Board::Move move = board.get_best_move((Board::Player)_botId, 8, min(1000, time / 2));
      int i = move.first;
      int j = move.second;
      
      const auto out = make_pair(3 * (i % 3) + j % 3, 3 * (i/3) + j / 3);
      return out;
    }
    
    void processCommand(const vector<string> &command) {
      if (command.empty()) {
        return;
      }
      if (command[0] == "action") {
        auto point = action(command[1], stringToInt(command[2]));
        cout << "place_move " << point.first << " " << point.second << endl << flush;
      }
      else if (command[0] == "update") {
        update(command[1], command[2], command[3]);
      }
      else if (command[0] == "settings") {
        setting(command[1], command[2]);
      }
      else {
        debug("Unknown command <" + command[0] + ">.");
      }
    }
    
    void update(const string& player, const string& type, const string& value) {
      if (player != "game" && player != _myName) {
        // It's not my update!
        return;
      }
      if (type == "round") {
        _round = stringToInt(value);
      } else if (type == "move") {
        _move = stringToInt(value);
      } else if (type == "macroboard" || type == "field") {
        vector<string> rawValues;
        split(value, ',', rawValues);
        vector<int>::iterator choice = (type == "field" ? _field.begin() : _macroboard.begin());
        transform(rawValues.begin(), rawValues.end(), choice, stringToInt);
      }
      else {
        debug("Unknown update <" + type + ">.");
      }
    }
    
    void setting(const string& type, const string& value) {
      if (type == "timebank") {
        _timebank = stringToInt(value);
      }
      else if (type == "time_per_move") {
        _timePerMove = stringToInt(value);
      }
      else if (type == "player_names") {
        split(value, ',', _playerNames);
      }
      else if (type == "your_bot") {
        _myName = value;
      }
      else if (type == "your_botid") {
        _botId = stringToInt(value);
      }
      else {
        debug("Unknown setting <" + type + ">.");
      }
    }
    
    void debug(const string &s) const{
      cerr << s << endl << flush;
    }
    
  private:
    // static settings
    int _timebank;
    int _timePerMove;
    int _botId;
    vector<string> _playerNames;
    string _myName;
    
    // dynamic settings
    int _round;
    int _move;
    vector<int> _macroboard;
    vector<int> _field;
  };
  
}  // namespace tictactoe

/**
 * don't change this code.
 * See BotIO::action method.
 **/
int main() {
  ::tictactoe::BotIO bot;
  bot.loop();
  return 0;
}