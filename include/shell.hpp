#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "grammar.hpp"
#include "ll1_parser.hpp"
#include "slr1_parser.hpp"

namespace po = boost::program_options;

class Shell {
  public:
    Shell();

    void Run();

  private:
    Grammar    grammar;
    LL1Parser  ll1;
    SLR1Parser slr1;

    std::unordered_map<std::string,
                       std::function<void(const std::vector<std::string>&)>>
         commands;
    bool running = true;

    void ExecuteCommand(const std::string& input);
    void CmdLoad(const std::vector<std::string>& args);
    void CmdGDebug();
    void CmdFirst(const std::vector<std::string>& args);
    void CmdFollow(const std::vector<std::string>& args);
    void CmdPredictionSymbols(const std::vector<std::string>& args);
    void CmdLL1Table(const std::vector<std::string>& args);
    void PrintSet(const std::unordered_set<std::string>& set);
};