#include "../../include/shell.hpp"
#include <unordered_set>

#define GREEN  "\033[32m"
#define RED    "\033[31m"
#define BLUE   "\033[34m"
#define RESET  "\033[0m"


Shell::Shell() {
    commands["load"] = [this](const std::vector<std::string>& args) {
        CmdLoad(args);
    };
    commands["gdebug"] = [this](const std::vector<std::string>& args) {
        CmdGDebug();
    };
    commands["first"] = [this](const std::vector<std::string>& args) {
        CmdFirst(args);
    };
    commands["follow"] = [this](const std::vector<std::string>& args) {
        CmdFollow(args);
    };
    commands["predsymbols"] = [this](const std::vector<std::string>& args) {
        CmdPredictionSymbols(args);
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
        return;
    }
    std::string filename = args[0];
    grammar = Grammar();
    if (!grammar.ReadFromFile(filename)) {
        std::cout << RED << "pl-shell: load error when reading grammar from file. Check if there are any errors.\n" << RESET;
        return;
    }
    std::cout << GREEN << "Grammar loaded successfully.\n";
    ll1 = LL1Parser(grammar);
    slr1 = SLR1Parser(grammar);
}

void Shell::CmdGDebug() {
    if (grammar.g_.empty()) {
        std::cout << RED << "pl-shell: no grammar was loaded. Load one with load <filename>.\n" << RESET;
        return;
    }
    grammar.Debug();
    std::cout << "\nSymbol Table:\nn";
    grammar.st_.Debug();
}

void Shell::CmdFirst(const std::vector<std::string>& args) {
    if (grammar.g_.empty()) {
        std::cout << RED << "pl-shell: no grammar was loaded. Load one with load <filename>.\n" << RESET;
        return;
    }
    std::string arg;
    bool verbose_mode = false;
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "There is no docs, good luck :)")
        ("string", po::value<std::string>(&arg)->required())
        ("verbose,v", po::bool_switch(&verbose_mode));
    po::positional_options_description pos;
    pos.add("string", 1); 

    try {
        po::variables_map vm;
        po::store(po::command_line_parser(args)
        .options(desc)
        .positional(pos)
        .run(), vm);
        po::notify(vm);
        std::vector<std::string> splitted {grammar.Split(arg)};
        if (verbose_mode) {
            ll1.TeachFirst(splitted);
            return;
        } else {
            std::unordered_set<std::string> result;
            ll1.First(splitted, result);
            std::cout << GREEN "✔ " << RESET << "FIRST(" << arg << ") = ";
            PrintSet(result);
            std::cout << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << RED << "pl-shell: " << e.what() << "\n" << RESET;
        return;
    }
}

void Shell::CmdFollow(const std::vector<std::string>& args) {
    if (grammar.g_.empty()) {
        std::cout << RED << "pl-shell: no grammar was loaded. Load one with load <filename>.\n" << RESET;
        return;
    }
    std::string arg;
    bool verbose_mode = false;
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "There is no docs, good luck :)")
        ("string", po::value<std::string>(&arg)->required())
        ("verbose,v", po::bool_switch(&verbose_mode));
    po::positional_options_description pos;
    pos.add("string", 1); 

    try {
        po::variables_map vm;
        po::store(po::command_line_parser(args)
        .options(desc)
        .positional(pos)
        .run(), vm);
        po::notify(vm);
        if (arg.size() != 1) {
            std::cerr << RED << "pl-shell: follow function can only be applied to non terminals characters (strints with size = 1).\n" << RESET;
            return;
        }

        if (verbose_mode) {
            ll1.TeachFollow(arg);
            return;
        } else {
            std::unordered_set<std::string> result {ll1.Follow(arg)};
            std::cout << GREEN "✔ " << RESET << "FOLLOW(" << arg << ") = ";
            PrintSet(result);
            std::cout << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << RED << "pl-shell: " << e.what() << "\n" << RESET;
        return;
    }
}

void Shell::CmdPredictionSymbols(const std::vector<std::string>& args) {
    if (grammar.g_.empty()) {
        std::cout << RED << "pl-shell: no grammar was loaded. Load one with load <filename>.\n" << RESET;
        return;
    }
    std::string ant;
    std::string conseq;
    bool verbose_mode = false;
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "There is no docs, good luck :)")
        ("antecedent", po::value<std::string>(&ant)->required())
        ("consequent", po::value<std::string>(&conseq)->required())
        ("verbose,v", po::bool_switch(&verbose_mode));
    po::positional_options_description pos;
    pos.add("antecedent", 1); 
    pos.add("consequent", 2);
    try {
        po::variables_map vm;
        po::store(po::command_line_parser(args)
        .options(desc)
        .positional(pos)
        .run(), vm);
        po::notify(vm);
        std::vector<std::string> splitted {grammar.Split(conseq)};
        if (splitted.empty() || grammar.g_.find(ant) == grammar.g_.end() || std::find(grammar.g_[ant].begin(), grammar.g_[ant].end(), splitted) == grammar.g_[ant].end()) {
                std::cerr << RED << "pl-shell: rule does not exist.\n" << RESET;
                return;
            }
        if (verbose_mode) {
            ll1.TeachPredictionSymbols(ant, splitted);
            return;
        } else {
            if (grammar.g_.find(ant) == grammar.g_.end() || std::find(grammar.g_[ant].begin(), grammar.g_[ant].end(), splitted) == grammar.g_[ant].end()) {
                std::cerr << RED << "pl-shell: rule does not exist.\n" << RESET;
                return;
            }
            std::unordered_set<std::string> result{ll1.PredictionSymbols(ant, splitted)};
            std::cout << GREEN "✔ " << RESET << "PS(" << ant << " -> " << conseq << ") = ";
            PrintSet(result);
            std::cout << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << RED << "pl-shell: " << e.what() << "\n" << RESET;
        return;
    }
}

void Shell::PrintSet(const std::unordered_set<std::string>& set) {
    std::cout << "{ ";
    for (const std::string& str : set) {
        std::cout << str << " ";
    }
    std::cout << "}";
}