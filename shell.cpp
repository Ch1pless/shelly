#include <iostream>
#include <vector>
#include <set>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <filesystem>

#define sz(x) (int)(x).size()
enum STATUS_CODE { SUCCESS = 0, FAILURE = -1, INVALID_ARG, INVALID_NUM_ARGS };

// maybe
struct cmd {
  std::vector<std::string> args;
  std::set<std::string> opts;
};



static std::string shell_name = "mysh";

static std::vector<std::string> cmd_history;
static bool running;

// UTILITIES
int cmd_idx(int i) {
  return (sz(cmd_history)-i-1);
}

std::vector<std::string> tokenizer(std::string s) {
  std::stringstream ss(s);
  std::vector<std::string> ret;
  
  std::string word;
  while (ss >> word) ret.emplace_back(word);
  
  return ret;
}

cmd preprocess_cmd(std::string s) {
  std::vector<std::string> cmd_toks = tokenizer(s);
  cmd command; 
  command.args.emplace_back(cmd_toks[0]); // command's first argument is itself

}

STATUS_CODE call_cmd(std::vector<std::string>) {
  return SUCCESS;
}

// SHELL CONTROL
STATUS_CODE begin_shell() {
  bool history_exists = std::filesystem::exists(std::filesystem::path(shell_name + ".history"));
  if (history_exists) {

  }
  return SUCCESS;
}

// SHELL COMMANDS
STATUS_CODE history(std::vector<std::string> args) {
  if (sz(args) > 2) return INVALID_NUM_ARGS;

  if (sz(args) == 2) {
    if (args[1] == "c") {
      cmd_history.clear();
      return SUCCESS;
    } else {
      return INVALID_ARG;
    }
  }

  for (int i = sz(cmd_history)-1; i >= 0; --i) {
    std::cout << cmd_idx(i) << ": " << cmd_history[i] << "\n";
  }
  return SUCCESS;
}

STATUS_CODE replay(std::vector<std::string> args) {
  if (sz(args) != 1) return INVALID_NUM_ARGS;
  try {
    int i = cmd_idx(std::stoi(args[0]));
    
    if (i < 0 || sz(cmd_history) <= i) {
      return INVALID_ARG;
    } else {
      return call_cmd(prepare_cmd(cmd_history[i]));
    }
  } catch (const std::invalid_argument& ia) {
    return INVALID_ARG;
  }
}

STATUS_CODE byebye() {
  std::ofstream f_out(shell_name + ".history");


  // TODO: write command_history to f_out
  for (auto& cmd : cmd_history) std::cout << cmd;
  f_out.close();

  running = false;
  return SUCCESS;
}


int main(int argc, char* argv[]) {
  running = true;
  std::string command;
  while (running) {
    std::cout << "# ";

    getline(std::cin, command);
    cmd_history.emplace_back(command);
    std::vector<std::string> parsed_command = tokenizer(command);
    if (parsed_command[0] == "history") history();
    if (parsed_command[0] == "byebye") byebye();
  }

  return 0;
}