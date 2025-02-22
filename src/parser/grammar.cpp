#include "../../include/grammar.hpp"
#include "../../include/symbol_table.hpp"
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <regex>
#include <fstream>


bool Grammar::ReadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::in);

    if (!file.is_open()) {
            return false;
    }

    std::unordered_map<std::string, std::vector<std::string>> p_grammar;
    std::regex rx_terminal{R"(terminal\s+([a-zA-Z_\'][a-zA-Z_0-9\']*)\s+([^]*);\s*)"};
    std::regex rx_axiom{R"(start\s+with\s+([a-zA-Z_\'][a-zA-Z_0-9\']*);\s*)"};
    std::regex rx_empty_production{R"(([a-zA-Z_\'][a-zA-Z_0-9\']*)\s*->;\s*)"};
    std::regex rx_production{"([a-zA-Z_\\'][a-zA-Z_0-9\\']*)\\s*->\\s*([a-zA-Z_\\'][a-zA-Z_0-9\\s$\\']*);"};

    std::string input;
    std::smatch match;

    if (file.peek() == std::ifstream::traits_type::eof()) {
        return false;
    }

    try {
        while (getline(file, input) && input != ";") {
            std::string id;
            std::string value;

            if (std::regex_match(input, match, rx_terminal)) {
                st_.PutSymbol(match[1], match[2]);
            } else if (std::regex_match(input, match, rx_axiom)) {
                SetAxiom(match[1]);
            } else {
                return false;
            }
        }

        while (getline(file, input) && input != ";") {
            if (std::regex_match(input, match, rx_production)) {
                std::string s = match[2];
                s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                p_grammar[match[1]].push_back(s);
            } else if (std::regex_match(input, match, rx_empty_production)) {
                p_grammar[match[1]].push_back(st_.EPSILON_);
                st_.terminals_.insert(st_.EPSILON_);
            } else {
                return false;
            }
        }
    } catch (const std::exception& e) {
        file.close();
        return false;
    }

    file.close();

    // Add non-terminal symbols
    for (const auto& entry : p_grammar) {
        st_.PutSymbol(entry.first);
    }

    // Add all rules
    for (const auto& entry : p_grammar) {
        for (const auto& prod : entry.second) {
            if (!AddRule(entry.first, prod)) {
                return false;
            }
        }
    }

    return true; // Todo salió bien
}

std::vector<std::string> Grammar::Split(const std::string& s) {
    if (s == st_.EPSILON_) {
        return {st_.EPSILON_};
    }
    std::vector<std::string> splitted{};
    std::string              str;
    unsigned                 start{0};
    unsigned                 end{1};
    while (end <= s.size()) {
        str = s.substr(start, end - start);

        if (st_.In(str)) {
            unsigned lookahead = end + 1;
            while (lookahead <= s.size()) {
                std::string extended = s.substr(start, lookahead - start);
                if (st_.In(extended)) {
                    end = lookahead;
                }
                ++lookahead;
            }
            splitted.push_back(s.substr(start, end - start));
            start = end;
            end   = start + 1;
        } else {
            ++end;
        }
    }

    // If start < end - 1 there is at least one symbol not recognized
    if (start < end - 1) {
        return {};
    }

    return splitted;
}

bool Grammar::AddRule(const std::string& antecedent,
                      const std::string& consequent) {
    std::vector<std::string> splitted_consequent{Split(consequent)};
    if (splitted_consequent.empty()) return false;
    g_[antecedent].push_back(splitted_consequent);
    return true;
}

void Grammar::SetAxiom(const std::string& axiom) {
    axiom_ = axiom;
}

bool Grammar::HasEmptyProduction(const std::string& antecedent) {
    auto rules{g_.at(antecedent)};
    return std::find_if(rules.cbegin(), rules.cend(), [&](const auto& rule) {
               return rule[0] == st_.EPSILON_;
           }) != rules.cend();
}

std::vector<std::pair<const std::string, production>>
Grammar::FilterRulesByConsequent(const std::string& arg) {
    std::vector<std::pair<const std::string, production>> rules;
    for (const auto& rule : g_) {
        for (const production& prod : rule.second) {
            if (std::find(prod.cbegin(), prod.cend(), arg) != prod.cend()) {
                rules.emplace_back(rule.first, prod);
            }
        }
    }
    return rules;
}

void Grammar::Debug() {
    std::cout << "Grammar:\n";

    std::cout << axiom_ << " -> ";
    const auto& axiom_productions = g_.at(axiom_);
    for (size_t i = 0; i < axiom_productions.size(); ++i) {
        for (const std::string& symbol : axiom_productions[i]) {
            std::cout << symbol << " ";
        }
        if (i < axiom_productions.size() - 1) {
            std::cout << "| ";
        }
    }
    std::cout << "\n";

    std::vector<std::string> non_terminals;
    for (const auto& entry : g_) {
        if (entry.first != axiom_) {
            non_terminals.push_back(entry.first);
        }
    }

    std::sort(non_terminals.begin(), non_terminals.end());

    for (const std::string& nt : non_terminals) {
        std::cout << nt << " -> ";
        const auto& productions = g_.at(nt);
        for (size_t i = 0; i < productions.size(); ++i) {
            for (const std::string& symbol : productions[i]) {
                std::cout << symbol << " ";
            }
            if (i < productions.size() - 1) {
                std::cout << "| ";
            }
        }
        std::cout << "\n";
    }
}

bool Grammar::HasLeftRecursion(const std::string&              antecedent,
                               const std::vector<std::string>& consequent) {
    return consequent.at(0) == antecedent;
}

void Grammar::AddProduction(const std::string&              antecedent,
                            const std::vector<std::string>& consequent) {
    g_[antecedent].push_back(std::move(consequent));
}
