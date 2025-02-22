#pragma once

#include <boost/program_options.hpp>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
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

    static std::unordered_map<std::string,
                       std::function<void(const std::vector<std::string>&)>>
                commands;
    bool        running = true;
    static void SignalHandler(int signum);

    void ExecuteCommand(const std::string& input);
    void PrintHistory();
    static char** ShellCompletion(const char* text, int start, int end);
    static char* CommandGenerator(const char* text, int state);
    void CmdExit();
    void CmdHelp();
    void CmdClear();
    void CmdLoad(const std::vector<std::string>& args);
    void CmdGDebug();
    void CmdFirst(const std::vector<std::string>& args);
    void CmdFollow(const std::vector<std::string>& args);
    void CmdPredictionSymbols(const std::vector<std::string>& args);
    void CmdLL1Table(const std::vector<std::string>& args);
    void CmdAllLRItems(const std::vector<std::string>& args);
    void CmdClosure(const std::vector<std::string>& args);
    void PrintSet(const std::unordered_set<std::string>& set);
};