#include "../../include/shell.hpp"
#include <unordered_set>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

std::unordered_map<std::string,
                   std::function<void(const std::vector<std::string>&)>>
    Shell::commands;

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
    commands["ll1"] = [this](const std::vector<std::string>& args) {
        CmdLL1Table(args);
    };
    commands["allitems"] = [this](const std::vector<std::string>& args) {
        CmdAllLRItems(args);
    };
    commands["closure"] = [this](const std::vector<std::string>& args) {
        CmdClosure(args);
    };
    commands["exit"] = [this](const std::vector<std::string>& args) {
        CmdExit();
    };
    commands["history"] = [this](const std::vector<std::string>& args) {
        PrintHistory();
    };
    commands["help"] = [this](const std::vector<std::string>& args) {
        CmdHelp();
    };
    commands["clear"] = [this](const std::vector<std::string>& args) {
        CmdClear();
    };

    std::signal(SIGINT, Shell::SignalHandler);
    std::signal(SIGTSTP, Shell::SignalHandler);
}

void Shell::Run() {

    std::cout << GREEN << "========================================\n";
    std::cout << " Welcome to " << BLUE << "PLShell" << GREEN << "!\n";
    std::cout << " Version: " << YELLOW << "1.0" << GREEN << "\n";
    std::cout << " Created by: " << MAGENTA << "jose-rZM" << GREEN
              << " @ GitHub\n";
    std::cout << GREEN << " Type " << BLUE << "'help'" << GREEN
              << " for a list of commands.\n";
    std::cout << "========================================\n" << RESET;

    rl_attempted_completion_function = ShellCompletion;
    while (running) {
        char* input = readline("\033[32mpl-shell> \033[0m");
        if (!input) {
            break;
        }
        std::string command(input);
        free(input);

        if (!command.empty()) {
            add_history(command.c_str());
            ExecuteCommand(command);
        }
    }
    std::cout << "Bye!\n";
}

void Shell::ExecuteCommand(const std::string& input) {
    std::istringstream       iss(input);
    std::vector<std::string> tokens;
    std::string              token;

    while (iss >> token)
        tokens.push_back(token);
    if (tokens.empty())
        return;

    auto cmd = commands.find(tokens[0]);
    if (cmd == commands.end()) {
        std::cout << RED << "Command not recognized.\n" << RESET;
        SuggestCommand(tokens[0]);
        return;
    }
    tokens.erase(tokens.begin());
    cmd->second(tokens);
}

void Shell::PrintHistory() {
    HIST_ENTRY** hist_list = history_list();
    if (hist_list) {
        for (size_t i = 0; hist_list[i]; ++i) {
            std::cout << i + history_base << " " << hist_list[i]->line << "\n";
        }
    } else {
        std::cerr << RED << "pl-shell: there is no history.\n" << RESET;
    }
}

char* Shell::CommandGenerator(const char* text, int state) {
    static std::unordered_map<
        std::string,
        std::function<void(const std::vector<std::string>&)>>::iterator it;
    static size_t                                                       len;
    const char*                                                         name;

    if (!state) {
        it  = Shell::commands.begin();
        len = strlen(text);
    }

    while (it != Shell::commands.end()) {
        name = it->first.c_str();
        ++it;

        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }
    return nullptr;
}

char** Shell::ShellCompletion(const char* text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, CommandGenerator);
}

void Shell::SuggestCommand(const std::string& input) {
    std::string closest_match;
    size_t      min_distance = std::numeric_limits<size_t>::max();
    bool        has_prefix   = false;

    for (const auto& cmd : Shell::commands) {
        if (cmd.first.find(input) == 0) {
            closest_match = cmd.first;
            has_prefix    = true;
            break;
        }

        size_t distance = LevenshteinDistance(cmd.first, input);
        if (distance < min_distance) {
            min_distance  = distance;
            closest_match = cmd.first;
        }
    }
    if (has_prefix || min_distance <= 2) {
        std::cout << YELLOW << "Did you mean '" << closest_match << "'?\n"
                  << RESET;
    }
}

void Shell::SignalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTSTP) {
        std::cout << RED << "\nType 'exit' to quit.\n"
                  << GREEN << "pl-shell> " << RESET;
        std::cout.flush();
    }
}

void Shell::CmdExit() {
    std::cout << "Are you sure you want to exit? (y/n): ";
    char response;
    std::cin >> response;
    if (response == 'y' || response == 'Y') {
        running = false;
    }
}

void Shell::CmdHelp() {
    std::cout << "Available commands:\n";
    std::cout << "  load         - Load a file\n";
    std::cout << "  gdebug       - Enable/disable debug mode\n";
    std::cout << "  first        - Compute FIRST set\n";
    std::cout << "  follow       - Compute FOLLOW set\n";
    std::cout << "  predsymbols  - List predictive symbols\n";
    std::cout << "  ll1          - Generate LL(1) parsing table\n";
    std::cout << "  allitems     - List all LR(0) items\n";
    std::cout << "  closure      - Compute closure of a set of items\n";
    std::cout << "  exit         - Exit the shell\n";
    std::cout << "  history      - Show command history\n";
    std::cout << "  help         - Show this help message\n";
}

void Shell::CmdClear() {
    std::cout << "\033[2J\033[1;1H";
}

void Shell::CmdLoad(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cout << RED << "pl-shell: load expects one argument.\n" << RESET;
        return;
    }
    std::string filename = args[0];
    grammar              = Grammar();
    if (!grammar.ReadFromFile(filename)) {
        std::cout << RED
                  << "pl-shell: load error when reading grammar from file. "
                     "Check if there are any errors.\n"
                  << RESET;
        return;
    }
    std::cout << GREEN << "Grammar loaded successfully.\n";
    ll1  = LL1Parser(grammar);
    slr1 = SLR1Parser(grammar);
}

void Shell::CmdGDebug() {
    if (grammar.g_.empty()) {
        std::cout << RED
                  << "pl-shell: no grammar was loaded. Load one with load "
                     "<filename>.\n"
                  << RESET;
        return;
    }
    grammar.Debug();
    std::cout << "\nSymbol Table:\nn";
    grammar.st_.Debug();
}

void Shell::CmdFirst(const std::vector<std::string>& args) {
    if (grammar.g_.empty()) {
        std::cout << RED
                  << "pl-shell: no grammar was loaded. Load one with load "
                     "<filename>.\n"
                  << RESET;
        return;
    }
    std::string             arg;
    bool                    verbose_mode = false;
    po::options_description desc("Options");
    desc.add_options()("help,h", "There is no docs, good luck :)")(
        "string", po::value<std::string>(&arg)->required())(
        "verbose,v", po::bool_switch(&verbose_mode));
    po::positional_options_description pos;
    pos.add("string", 1);

    try {
        po::variables_map vm;
        po::store(
            po::command_line_parser(args).options(desc).positional(pos).run(),
            vm);
        po::notify(vm);
        std::vector<std::string> splitted{grammar.Split(arg)};
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
        std::cout << RED
                  << "pl-shell: no grammar was loaded. Load one with load "
                     "<filename>.\n"
                  << RESET;
        return;
    }
    std::string             arg;
    bool                    verbose_mode = false;
    po::options_description desc("Options");
    desc.add_options()("help,h", "There is no docs, good luck :)")(
        "string", po::value<std::string>(&arg)->required())(
        "verbose,v", po::bool_switch(&verbose_mode));
    po::positional_options_description pos;
    pos.add("string", 1);

    try {
        po::variables_map vm;
        po::store(
            po::command_line_parser(args).options(desc).positional(pos).run(),
            vm);
        po::notify(vm);
        if (arg.size() != 1) {
            std::cerr << RED
                      << "pl-shell: follow function can only be applied to non "
                         "terminals characters (strints with size = 1).\n"
                      << RESET;
            return;
        }

        if (verbose_mode) {
            ll1.TeachFollow(arg);
            return;
        } else {
            std::unordered_set<std::string> result{ll1.Follow(arg)};
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
        std::cout << RED
                  << "pl-shell: no grammar was loaded. Load one with load "
                     "<filename>.\n"
                  << RESET;
        return;
    }
    std::string             ant;
    std::string             conseq;
    bool                    verbose_mode = false;
    po::options_description desc("Options");
    desc.add_options()("help,h", "There is no docs, good luck :)")(
        "antecedent", po::value<std::string>(&ant)->required())(
        "consequent", po::value<std::string>(&conseq)->required())(
        "verbose,v", po::bool_switch(&verbose_mode));
    po::positional_options_description pos;
    pos.add("antecedent", 1);
    pos.add("consequent", 2);
    try {
        po::variables_map vm;
        po::store(
            po::command_line_parser(args).options(desc).positional(pos).run(),
            vm);
        po::notify(vm);
        std::vector<std::string> splitted{grammar.Split(conseq)};
        if (splitted.empty() || grammar.g_.find(ant) == grammar.g_.end() ||
            std::find(grammar.g_[ant].begin(), grammar.g_[ant].end(),
                      splitted) == grammar.g_[ant].end()) {
            std::cerr << RED << "pl-shell: rule does not exist.\n" << RESET;
            return;
        }
        if (verbose_mode) {
            ll1.TeachPredictionSymbols(ant, splitted);
            return;
        } else {
            if (grammar.g_.find(ant) == grammar.g_.end() ||
                std::find(grammar.g_[ant].begin(), grammar.g_[ant].end(),
                          splitted) == grammar.g_[ant].end()) {
                std::cerr << RED << "pl-shell: rule does not exist.\n" << RESET;
                return;
            }
            std::unordered_set<std::string> result{
                ll1.PredictionSymbols(ant, splitted)};
            std::cout << GREEN "✔ " << RESET << "PS(" << ant << " -> " << conseq
                      << ") = ";
            PrintSet(result);
            std::cout << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << RED << "pl-shell: " << e.what() << "\n" << RESET;
        return;
    }
}

void Shell::CmdLL1Table(const std::vector<std::string>& args) {
    if (args.size() > 1) {
        std::cerr << RED
                  << "pl-shell: only 1 argument at most can be given to ll1.\n"
                  << RESET;
        return;
    }
    bool verbose_mode = false;
    if (!args.empty()) {
        if (args[0] == "-v" || args[0] == "--verbose") {

            verbose_mode = true;
        } else {
            std::cerr << RED
                      << "pl-shell: unrecognized option in ll1 command. "
                         "Options are: -v or --verbose.\n"
                      << RESET;
            return;
        }
    }
    if (grammar.g_.empty()) {
        std::cerr << RED
                  << "pl-shell: no grammar was loaded. Load one with load "
                     "<filename>.\n"
                  << RESET;
        return;
    }
    ll1.CreateLL1Table();
    if (verbose_mode) {
        ll1.TeachLL1Table();
    } else {
        std::cout << "LL(1) Table:\n";
        ll1.PrintTable();
    }
}

void Shell::CmdAllLRItems(const std::vector<std::string>& args) {
    if (args.size() > 1) {
        std::cerr << RED << "pl-shell: only 1 argument at most can be given.\n"
                  << RESET;
        return;
    }
    bool verbose_mode = false;
    if (!args.empty()) {
        if (args[0] == "-v" || args[0] == "--verbose") {

            verbose_mode = true;
        } else {
            std::cerr << RED
                      << "pl-shell: unrecognized option. "
                         "Options are: -v or --verbose.\n"
                      << RESET;
            return;
        }
    }
    if (grammar.g_.empty()) {
        std::cerr << RED
                  << "pl-shell: no grammar was loaded. Load one with load "
                     "<filename>.\n"
                  << RESET;
        return;
    }
    ll1.CreateLL1Table();
    if (verbose_mode) {
        slr1.TeachAllItems();
    } else {
        std::cout << "All LR0 items:\n";
        std::unordered_set<Lr0Item> items;
        items = slr1.AllItems();
        std::unordered_map<std::string, std::vector<Lr0Item>> grouped_items;
        for (const Lr0Item& item : items) {
            grouped_items[item.antecedent_].push_back(item);
        }

        for (const auto& [antecedent, item_list] : grouped_items) {
            std::cout << "Non-terminal: " << antecedent << "\n";
            for (const Lr0Item& item : item_list) {
                std::cout << "  - " << item.antecedent_ << " -> ";
                for (size_t i = 0; i < item.consequent_.size(); ++i) {
                    if (i == item.dot_) {
                        std::cout << "• ";
                    }
                    std::cout << item.consequent_[i] << " ";
                }
                if (item.dot_ == item.consequent_.size()) {
                    std::cout << "•";
                }
                std::cout << "\n";
            }

            std::cout << "Total LR(0) items generated: " << items.size()
                      << "\n";
        }
    }
}

void Shell::CmdClosure(const std::vector<std::string>& args) {
    if (grammar.g_.empty()) {
        std::cout << RED
                  << "pl-shell: no grammar was loaded. Load one with load "
                     "<filename>.\n"
                  << RESET;
        return;
    }

    bool        verbose_mode = false;
    std::string rules_str;

    po::options_description desc("Options");
    desc.add_options()("help,h", "There is no docs, good luck :)")(
        "rules", po::value<std::string>(&rules_str)->required(),
        "Grammar rules (comma-separated)")("verbose,v",
                                           po::bool_switch(&verbose_mode));
    po::positional_options_description pos;
    pos.add("rules", 1);
    try {
        po::variables_map vm;
        po::store(
            po::command_line_parser(args).options(desc).positional(pos).run(),
            vm);
        po::notify(vm);

        std::stringstream           ss(rules_str);
        char                        del = ',';
        std::string                 token;
        std::unordered_set<Lr0Item> items;
        while (std::getline(ss, token, del)) {
            size_t arrow = token.find("->");
            if (arrow == std::string::npos) {
                std::cerr << RED << "pl-shell: invalid rule format: " << token
                          << RESET << "\n";
                return;
            }
            std::string antecedent = token.substr(0, arrow);
            std::string consequent = token.substr(arrow + 2);

            size_t dot = consequent.find('.');
            if (dot == std::string::npos) {
                std::cerr << RED << "pl-shell: dot not found in: " << token
                          << RESET << "\n";
                return;
            }
            std::string before_dot = consequent.substr(0, dot);
            std::string after_dot  = consequent.substr(dot + 1);

            std::vector<std::string> splitted_before_dot{
                grammar.Split(before_dot)};
            std::vector<std::string> splitted_after_dot{
                grammar.Split(after_dot)};

            std::vector<std::string> splitted{splitted_before_dot.begin(),
                                              splitted_before_dot.end()};
            splitted.insert(splitted.end(), splitted_after_dot.begin(),
                            splitted_after_dot.end());
            size_t  dot_idx = splitted_before_dot.size();
            Lr0Item item{antecedent, splitted, (unsigned int) dot_idx,
                         grammar.st_.EPSILON_, grammar.st_.EOL_};
            items.insert(item);
        }
        slr1.TeachClosure(items);
    } catch (const std::exception& e) {
        std::cerr << RED << "pl-shell: " << e.what() << RESET << "\n";
    }
}

void Shell::PrintSet(const std::unordered_set<std::string>& set) {
    std::cout << "{ ";
    for (const std::string& str : set) {
        std::cout << str << " ";
    }
    std::cout << "}";
}

size_t Shell::LevenshteinDistance(const std::string& w1,
                                  const std::string& w2) {
    size_t size_w1 = w1.size();
    size_t size_w2 = w2.size();
    int    dp[size_w1 + 1][size_w2 + 1];

    if (size_w1 == 0)
        return size_w2;
    else if (size_w2 == 0)
        return size_w1;

    for (size_t i = 0; i <= size_w1; ++i) {
        dp[i][0] = i;
    }
    for (size_t i = 0; i <= size_w2; ++i) {
        dp[0][i] = i;
    }

    for (size_t i = 1; i <= size_w1; ++i) {
        for (size_t j = 1; j <= size_w2; ++j) {
            int cost = w1[i - 1] == w2[j - 1] ? 0 : 1;
            dp[i][j] = std::min(std::min(dp[i - 1][j] + 1, dp[i][j - 1] + 1),
                                dp[i - 1][j - 1] + cost);
        }
    }
    return dp[size_w1][size_w2];
}