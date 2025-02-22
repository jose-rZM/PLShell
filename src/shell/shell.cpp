#include "../../include/shell.hpp"
#include "shell.hpp"


#define GREEN  "\033[32m"
#define RED    "\033[31m"
#define BLUE   "\033[34m"
#define RESET  "\033[0m"

Shell::Shell() {
    commands["load"] = [this](const std::vector<std::string>& args) {
        CmdLoad(args);
    };
}

void Shell::Run() {
    std::string input;
    while (running) {
        std::cout << GREEN << "pl-shell> " << RESET;
        if (!std::getline(std::cin, input)) break;
        ExecuteCommand(input);
    }
}

void Shell::ExecuteCommand(const std::string& input) {
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) tokens.push_back(token);
    if (tokens.empty()) return;

    auto cmd = commands.find(tokens[0]);
    if (cmd == commands.end()) {
        std::cout << RED << "Command not recognized.\n" << RESET;
        return;
    }
    tokens.erase(tokens.begin());
    cmd->second(tokens);
}

void Shell::CmdLoad(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cout << RED << "pl-shell: load expects one argument.\n" << RESET;
    }
    std::string filename = args[0];
}
