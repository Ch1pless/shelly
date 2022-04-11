// streams
#include <iostream>
#include <sstream>
#include <fstream>
// containers
#include <vector>
#include <set>
// auxiliary
#include <filesystem>
namespace fs = std::filesystem;
#include <stdexcept>
// process control
#include <sys/wait.h>
#include <unistd.h>

#define sz(x) (int)(x).size()
enum STATUS_CODE { FAILURE=-1, SUCCESS, INVALID_ARG, INVALID_NARG };

static std::string shell_name = "mysh";


bool running;
std::vector<std::string> cmd_history;
std::set<pid_t> background_procs;

// UTILITIES
int commandIdx(int i) {
  return (sz(cmd_history)-i-1);
}

std::vector<std::string> tokenizer(std::string s) {
  std::stringstream ss(s);
  std::vector<std::string> ret;
  
  std::string word;
  while (ss >> word) ret.emplace_back(word);
  
  return ret;
}

std::string getCommand() {
  std::string cmd; 
  getline(std::cin, cmd);
  while (cmd.find("\r") != std::string::npos) {
    cmd.erase(cmd.find("\r"));
  }
  while (cmd.find("\n") != std::string::npos) {
    cmd.erase(cmd.find("\n"));
  }
  return cmd;
}

// PROTOTYPES
STATUS_CODE callCommand(std::vector<std::string>);

// SHELL CONTROL
STATUS_CODE beginShell() {
  bool history_exists = fs::exists(fs::path(shell_name + ".history"));
  if (history_exists) {
    std::ifstream f_in;
    f_in.open(shell_name + ".history");

    std::string line;
    while (getline(f_in, line)) {
      cmd_history.push_back(line);
    }
    f_in.close();
  }
  return SUCCESS;
}

// SHELL COMMANDS
STATUS_CODE history(std::vector<std::string> args) {
  if (sz(args) > 2) return INVALID_NARG;

  if (sz(args) == 2) {
    if (args[1] == "-c") {
      cmd_history.clear();
      return SUCCESS;
    } else {
      return INVALID_ARG;
    }
  }

  for (int i = sz(cmd_history)-1; i >= 0; --i) {
    std::cout << commandIdx(i) << ": " << cmd_history[i] << "\n";
  }
  return SUCCESS;
}

STATUS_CODE byebye(std::vector<std::string> args) {
  std::ofstream f_out;
  f_out.open(shell_name + ".history");
  if (!f_out.is_open()) return FAILURE;

  // TODO: write command_history to f_out
  for (auto& cmd : cmd_history) f_out << cmd << "\n";
  f_out.close();

  running = false;
  return SUCCESS;
}

STATUS_CODE replay(std::vector<std::string> args) {
  if (sz(args) != 2) return INVALID_NARG;
  try {
    // add 1 due to saving command prior to call
    int i = commandIdx(std::stoi(args[1])+1);
    
    if (i < 0 || sz(cmd_history) <= i) {
      return INVALID_ARG;
    } else {
      return callCommand(tokenizer(cmd_history[i]));
    }
  } catch (const std::invalid_argument& ia) {
    return INVALID_ARG;
  }
}

STATUS_CODE start(std::vector<std::string> args) {
  if (sz(args) < 2) return INVALID_NARG;
  
  pid_t id = fork();
  if (id == 0) {
    char *parameters[sz(args) - 1];
    for (int i = 0; i < sz(args)-2; ++i)
      parameters[i] = args[i+2].data();
    parameters[sz(args)-2] = 0; // tell args where to stop

    int errored = execv(args[1].data(), parameters);
    std::cout << "[Error] Program could not be started.\n";
    exit(errored);
  } else {
    if (id == waitpid(id, nullptr, 0))
      return SUCCESS;
  }
  return FAILURE;
}

STATUS_CODE background(std::vector<std::string> args) {
  if (sz(args) < 2) return INVALID_NARG;

  pid_t id = fork();
  if (id == 0) {
    char *parameters[sz(args) - 1];
    for (int i = 0; i < sz(args)-2; ++i)
      parameters[i] = args[i+2].data();
    parameters[sz(args)-2] = 0; // tell args where to stop

    int errored = execv(args[1].data(), parameters);
    std::cout << "[Error] Program could not be started.\n";
    exit(errored);
  } else {
    std::cout << "[Background] Running " << id << ".\n";
    background_procs.insert(id);
  }
  return SUCCESS;
}

STATUS_CODE terminate(std::vector<std::string> args) {
  if (sz(args) != 2) return INVALID_NARG;

  try {
    int id = std::stoi(args[1]);

    if (background_procs.count(id))
      background_procs.erase(id);
    
    if (kill(id, SIGKILL)) {
      std::cout << "[Background] Killed " << id << ".\n";
    } else {
      std::cout << "[Background] " << id << " is not a proccess.\n";
    }
    return SUCCESS;
  } catch (const std::invalid_argument& ia) {
    return INVALID_ARG;
  }
}

STATUS_CODE repeat(std::vector<std::string> args) {
  if (sz(args) < 3) return INVALID_NARG;

  try {
    int n = std::stoi(args[1]);
    std::vector<std::string> sub_args;
    sub_args.push_back("background");
    for (int i = 2; i < sz(args); ++i) {
      sub_args.push_back(args[i]);
    }
    for (int i = 0; i < n; ++i) 
      if (callCommand(sub_args) != SUCCESS)
        return FAILURE;
  } catch (const std::invalid_argument& ia) {
    return INVALID_ARG;
  }
  return SUCCESS;
}

STATUS_CODE terminateall(std::vector<std::string> args) {
  if (sz(args) != 1) return INVALID_NARG;
  if (background_procs.empty()) {
    std::cout << "No processes to terminate.\n";
    return SUCCESS;
  }
  std::cout << "Terminating " << sz(background_procs) << " processes: ";
  for (auto id : background_procs) {
    kill(id, SIGKILL);
    std::cout << id << " ";
  }
  std::cout << "\n";
  background_procs.clear();
  return SUCCESS;
}

STATUS_CODE maik(std::vector<std::string> args) {
  if (sz(args) != 2) return INVALID_NARG;
  if (fs::exists(fs::path(args[1]))) return INVALID_ARG;
  std::fstream nFile;

  nFile.open(args[1], std::ios::out);

  if (!nFile) return FAILURE;

  nFile << "Draft";
  nFile.close();

  return SUCCESS;
}

STATUS_CODE dwelt(std::vector<std::string> args) {
  if (sz(args) != 2) return INVALID_NARG;
  fs::file_status stats = fs::status(fs::path(args[1]));
  if (!fs::exists(stats)) {
    std::cout << "Dwelt not.\n";
  } else if (fs::is_directory(stats)) {
    std::cout << "Abode is\n";
  } else if (fs::is_regular_file(stats)) {
    std::cout << "Dwelt indeed.\n";
  } else {
    return FAILURE;
  }
  return SUCCESS;
}

STATUS_CODE coppy(std::vector<std::string> args) {
  if (sz(args) != 3) return INVALID_NARG;
  fs::file_status src_stats = fs::status(fs::path(args[1]));
  fs::file_status dst_stats = fs::status(fs::path(args[2]));

  if (!fs::exists(src_stats) || fs::exists(dst_stats) /* TODO: || !fs::exists(dst_directory) */) {
    return INVALID_ARG;
  }

  // TODO: Copy the file

  return FAILURE;
}

STATUS_CODE coppyabode(std::vector<std::string> args) {
  if (sz(args) != 3) return INVALID_NARG;
  fs::file_status src_stats = fs::status(fs::path(args[1]));
  fs::file_status dst_stats = fs::status(fs::path(args[2]));

  // TODO: Copy the directory

  return FAILURE;
}

STATUS_CODE callCommand(std::vector<std::string> args) {
  if      (args[0] == "history")      return history(args);
  else if (args[0] == "byebye")       return byebye(args);
  else if (args[0] == "replay")       return replay(args);
  else if (args[0] == "start")        return start(args);
  else if (args[0] == "background")   return background(args);
  else if (args[0] == "terminate")    return terminate(args);
  else if (args[0] == "repeat")       return repeat(args);
  else if (args[0] == "terminateall") return terminateall(args);
  else if (args[0] == "maik")         return maik(args);
  else if (args[0] == "dwelt")        return dwelt(args);
  else if (args[0] == "coppy")        return coppy(args);
  else if (args[0] == "coppyabode")   return coppyabode(args);
  return FAILURE;
}


int main(int argc, char* argv[]) {
  beginShell();
  running = true;
  while (running) {
    std::cout << "# ";
    std::string cmd = getCommand();                   // get
    cmd_history.emplace_back(cmd);                    // save
    STATUS_CODE status = callCommand(tokenizer(cmd)); // call
    switch (status) {                                 // report
      case SUCCESS:
        break;
      case INVALID_ARG:
        std::cout << "[Error] Invalid Argument.\n";
        break;
      case INVALID_NARG:
        std::cout << "[Error] Invalid Number of Arguments.\n";
        break;
      default:
        std::cout << "[Error] Command Failed.\n";
        break;
    }
  }

  return 0;
}