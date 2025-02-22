#include <iostream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>

#include "grammar.hpp"

namespace poo = boost::program_options;

class Shell {
public:
    Shell();

    void Run();
private:
    Grammar grammar;
    std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> commands;
    bool running = true;

    void ExecuteCommand(const std::string& input);
    void CmdLoad(const std::vector<std::string>& args);
    void CmdGDebug();
    void CmdFirst(const std::vector<std::string>& args);
};