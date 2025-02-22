#include <algorithm>
#include <cstddef>
#include <iostream>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "../../include/grammar.hpp"
#include "../../include/ll1_parser.hpp"
#include "../../include/symbol_table.hpp"
#include "../../include/tabulate.hpp"

LL1Parser::LL1Parser(Grammar gr) : gr_(std::move(gr)) {
    ComputeFirstSets();
    ComputeFollowSets();
}

bool LL1Parser::CreateLL1Table() {
    if (first_sets_.empty() || follow_sets_.empty()) {
        ComputeFirstSets();
        ComputeFollowSets();
    }
    size_t nrows{gr_.g_.size()};
    ll1_t_.reserve(nrows);
    bool has_conflict{false};
    for (const auto& rule : gr_.g_) {
        std::unordered_map<std::string, std::vector<production>> column;
        for (const production& p : rule.second) {
            std::unordered_set<std::string> ds =
                PredictionSymbols(rule.first, p);
            column.reserve(ds.size());
            for (const std::string& symbol : ds) {
                auto& cell = column[symbol];
                if (!cell.empty()) {
                    has_conflict = true;
                }
                cell.push_back(p);
            }
        }
        ll1_t_.insert({rule.first, column});
    }
    return !has_conflict;
}

void LL1Parser::First(std::span<const std::string>     rule,
                      std::unordered_set<std::string>& result) {
    if (rule.empty() || (rule.size() == 1 && rule[0] == gr_.st_.EPSILON_)) {
        result.insert(gr_.st_.EPSILON_);
        return;
    }

    if (rule.size() > 1 && rule[0] == gr_.st_.EPSILON_) {
        First(std::span<const std::string>(rule.begin() + 1, rule.end()),
              result);
    } else {

        if (gr_.st_.IsTerminal(rule[0])) {
            // EOL cannot be in first sets, if we reach EOL it means that the
            // axiom is nullable, so epsilon is included instead
            if (rule[0] == gr_.st_.EOL_) {
                result.insert(gr_.st_.EPSILON_);
                return;
            }
            result.insert(rule[0]);
            return;
        }

        const std::unordered_set<std::string>& fii = first_sets_[rule[0]];
        for (const auto& s : fii) {
            if (s != gr_.st_.EPSILON_) {
                result.insert(s);
            }
        }

        if (fii.find(gr_.st_.EPSILON_) == fii.cend()) {
            return;
        }
        First(std::span<const std::string>(rule.begin() + 1, rule.end()),
              result);
    }
}

// Least fixed point
void LL1Parser::ComputeFirstSets() {
    // Init all FIRST to empty
    for (const auto& [nonTerminal, _] : gr_.g_) {
        first_sets_[nonTerminal] = {};
    }

    bool changed;
    do {
        auto old_first_sets = first_sets_; // Copy current state

        for (const auto& [nonTerminal, productions] : gr_.g_) {
            for (const auto& prod : productions) {
                std::unordered_set<std::string> tempFirst;
                First(prod, tempFirst);

                if (tempFirst.find(gr_.st_.EOL_) != tempFirst.end()) {
                    tempFirst.erase(gr_.st_.EOL_);
                    tempFirst.insert(gr_.st_.EPSILON_);
                }

                auto& current_set = first_sets_[nonTerminal];
                current_set.insert(tempFirst.begin(), tempFirst.end());
            }
        }

        // Until all remain the same
        changed = (old_first_sets != first_sets_);

    } while (changed);
}

void LL1Parser::ComputeFollowSets() {
    for (const auto& [nt, _] : gr_.g_) {
        follow_sets_[nt] = {};
    }
    follow_sets_[gr_.axiom_].insert(gr_.st_.EOL_);

    bool changed;
    do {
        changed = false;
        for (const auto& rule : gr_.g_) {
            const std::string& lhs = rule.first;
            for (const production& rhs : rule.second) {
                for (size_t i = 0; i < rhs.size(); ++i) {
                    const std::string& symbol = rhs[i];
                    if (!gr_.st_.IsTerminal(symbol)) {
                        std::unordered_set<std::string> first_remaining;

                        if (i + 1 < rhs.size()) {
                            First(std::span<const std::string>(
                                      rhs.begin() + i + 1, rhs.end()),
                                  first_remaining);
                        } else {
                            first_remaining.insert(gr_.st_.EPSILON_);
                        }

                        for (const std::string& terminal : first_remaining) {
                            if (terminal != gr_.st_.EPSILON_) {
                                if (follow_sets_[symbol]
                                        .insert(terminal)
                                        .second) {
                                    changed = true;
                                }
                            }
                        }

                        if (first_remaining.find(gr_.st_.EPSILON_) !=
                            first_remaining.end()) {
                            for (const std::string& terminal :
                                 follow_sets_[lhs]) {
                                if (follow_sets_[symbol]
                                        .insert(terminal)
                                        .second) {
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    } while (changed);
}

std::unordered_set<std::string> LL1Parser::Follow(const std::string& arg) {
    if (follow_sets_.find(arg) == follow_sets_.end()) {
        return {};
    }
    return follow_sets_.at(arg);
}

std::unordered_set<std::string>
LL1Parser::PredictionSymbols(const std::string&              antecedent,
                             const std::vector<std::string>& consequent) {
    std::unordered_set<std::string> hd{};
    First({consequent}, hd);
    if (hd.find(gr_.st_.EPSILON_) == hd.end()) {
        return hd;
    }
    hd.erase(gr_.st_.EPSILON_);
    hd.merge(Follow(antecedent));
    return hd;
}

void LL1Parser::TeachFirst(const std::vector<std::string>& symbols) {
    std::cout << "Process of finding First(";
    for (const std::string& symbol : symbols) {
        std::cout << symbol;
    }
    std::cout << "):\n";

    std::unordered_set<std::string> first_set;
    TeachFirstUtil(symbols, first_set, 0);

    // Display the final First set
    std::cout << "Final First set: { ";
    for (const std::string& symbol : first_set) {
        std::cout << symbol << " ";
    }
    std::cout << "}\n";
}

void LL1Parser::TeachFirstUtil(const std::vector<std::string>&  symbols,
                               std::unordered_set<std::string>& first_set,
                               int                              depth) {
    // Base case: If all symbols are processed, return
    if (symbols.empty()) {
        return;
    }

    std::string              current_symbol = symbols[0];
    std::vector<std::string> remaining_symbols(symbols.begin() + 1,
                                               symbols.end());

    // Indent based on depth for better readability
    std::string indent(depth * 2, ' ');

    // Case 1: Current symbol is a terminal
    if (gr_.st_.IsTerminal(current_symbol)) {
        std::cout << indent << "- String: " << current_symbol << " ";
        for (const std::string& symbol : remaining_symbols) {
            std::cout << symbol << " ";
        }
        std::cout << "\n";
        std::cout << indent << "- Found terminal: " << current_symbol << "\n";
        std::vector<std::string>        all{current_symbol};
        std::unordered_set<std::string> fii;
        all.insert(all.end(), remaining_symbols.begin(),
                   remaining_symbols.end());
        First(std::span<const std::string>(all.begin(), all.end()), fii);
        first_set.insert(fii.begin(), fii.end());
        return;
    }

    // Case 2: Current symbol is a non-terminal
    std::cout << indent << "- Deriving non-terminal: " << current_symbol
              << "\n";
    const auto& productions = gr_.g_.at(current_symbol);
    for (const auto& prod : productions) {
        std::cout << indent << "  Using production: " << current_symbol
                  << " -> ";
        for (const std::string& symbol : prod) {
            std::cout << symbol << " ";
        }
        std::cout << "\n";

        // Recursively derive the production
        std::vector<std::string> new_symbols = prod;
        new_symbols.insert(new_symbols.end(), remaining_symbols.begin(),
                           remaining_symbols.end());
        TeachFirstUtil(new_symbols, first_set, depth + 1);

        // Check if ε is in First(prod)
        bool has_epsilon =
            std::find(prod.begin(), prod.end(), gr_.st_.EPSILON_) != prod.end();

        // If ε is in First(prod), continue deriving the remaining symbols
        if (has_epsilon) {
            std::cout
                << indent
                << "  - ε found in production. Deriving remaining symbols: ";
            for (const std::string& symbol : remaining_symbols) {
                std::cout << symbol << " ";
            }
            std::cout << "\n";
            TeachFirstUtil(remaining_symbols, first_set, depth + 1);
        }
    }

    // Display the current First set
    std::cout << indent << "Current First set: { ";
    for (const std::string& symbol : first_set) {
        std::cout << symbol << " ";
    }
    std::cout << "}\n";
}

void LL1Parser::TeachFollow(const std::string& non_terminal) {
    std::cout << "Process of finding Follow symbols of " << non_terminal
              << ":\n";

    if (non_terminal == gr_.axiom_) {
        std::cout << "Since " << non_terminal << " is the axiom, FOLLOW("
                  << non_terminal << ") = { " << gr_.st_.EOL_ << " }\n";
        return;
    }
    // Step 1: Find all rules where the non-terminal appears in the consequent
    std::vector<std::pair<std::string, production>> rules_with_nt;
    for (const auto& [antecedent, productions] : gr_.g_) {
        for (const auto& prod : productions) {
            auto it = std::find(prod.begin(), prod.end(), non_terminal);
            if (it != prod.end()) {
                rules_with_nt.emplace_back(antecedent, prod);
            }
        }
    }

    if (rules_with_nt.empty()) {
        std::cout << "1. " << non_terminal
                  << " does not appear in any consequent.\n";
        return;
    }

    std::cout << "1. Find the rules where " << non_terminal
              << " is in the consequent:\n";
    for (const auto& [antecedent, prod] : rules_with_nt) {
        std::cout << "   - " << antecedent << " -> ";
        for (const std::string& symbol : prod) {
            std::cout << symbol << " ";
        }
        std::cout << "\n";
    }

    // Step 2: Compute Follow for each occurrence of the non-terminal in the
    // rules
    std::unordered_set<std::string> follow_set;
    for (const auto& [antecedent, prod] : rules_with_nt) {
        for (auto it = prod.begin(); it != prod.end(); ++it) {
            if (*it == non_terminal) {
                // Case 1: Non-terminal is not at the end of the production
                if (std::next(it) != prod.end()) {
                    std::vector<std::string> remaining_symbols(std::next(it),
                                                               prod.end());
                    std::unordered_set<std::string> first_of_remaining;
                    First(remaining_symbols, first_of_remaining);

                    std::cout << "2. Compute First of the substring after "
                              << non_terminal << ": { ";
                    for (const std::string& symbol : remaining_symbols) {
                        std::cout << symbol << " ";
                    }
                    std::cout << "} = { ";
                    for (const std::string& symbol : first_of_remaining) {
                        std::cout << symbol << " ";
                    }
                    std::cout << "}\n";

                    // Add First(remaining_symbols) to Follow(non_terminal)
                    for (const std::string& symbol : first_of_remaining) {
                        if (symbol != gr_.st_.EPSILON_) {
                            follow_set.insert(symbol);
                        }
                    }

                    // If ε ∈ First(remaining_symbols), add Follow(antecedent)
                    if (first_of_remaining.find(gr_.st_.EPSILON_) !=
                        first_of_remaining.end()) {
                        std::cout << "   - Since ε ∈ First, add Follow("
                                  << antecedent << ") = { ";
                        std::unordered_set<std::string> ant_follow(
                            Follow(antecedent));
                        for (const std::string& str : ant_follow) {
                            std::cout << str << " ";
                        }
                        std::cout << "} to Follow(" << non_terminal << ")\n";
                        follow_set.insert(ant_follow.begin(), ant_follow.end());
                    }
                }
                // Case 2: Non-terminal is at the end of the production
                else {
                    std::cout << "2. " << non_terminal
                              << " is at the end of the production. Add Follow("
                              << antecedent << ") = { ";
                    std::unordered_set<std::string> ant_follow(
                        Follow(antecedent));
                    for (const std::string& str : ant_follow) {
                        std::cout << str << " ";
                    }

                    std::cout << "} to Follow(" << non_terminal << ")\n";
                    follow_set.insert(ant_follow.begin(), ant_follow.end());
                }
            }
        }
    }

    // Step 3: Display the final Follow set
    std::cout << "3. Final Follow(" << non_terminal << ") = { ";
    for (const std::string& symbol : follow_set) {
        std::cout << symbol << " ";
    }
    std::cout << "}\n";
}

void LL1Parser::TeachPredictionSymbols(const std::string& antecedent,
                                       const production&  consequent) {
    // Convert the consequent to a string for display purposes
    std::string consequent_str;
    for (const std::string& symbol : consequent) {
        consequent_str += symbol + " ";
    }
    if (!consequent_str.empty()) {
        consequent_str.pop_back(); // Remove the trailing space
    }

    std::cout << "Process of finding prediction symbols for the rule "
              << antecedent << " -> " << consequent_str << ":\n";

    // Step 1: Compute First(consequent)
    std::unordered_set<std::string> first_of_consequent;
    First(consequent, first_of_consequent);

    std::cout << "1. Compute First(" << consequent_str << ") = { ";
    for (const std::string& symbol : first_of_consequent) {
        std::cout << symbol << " ";
    }
    std::cout << "}\n";

    // Step 2: Initialize prediction symbols with First(consequent) excluding ε
    std::unordered_set<std::string> prediction_symbols;
    for (const std::string& symbol : first_of_consequent) {
        if (symbol != gr_.st_.EPSILON_) {
            prediction_symbols.insert(symbol);
        }
    }

    std::cout << "2. Initialize prediction symbols with First("
              << consequent_str << ") excluding ε: { ";
    for (const std::string& symbol : prediction_symbols) {
        std::cout << symbol << " ";
    }
    std::cout << "}\n";

    // Step 3: If ε ∈ First(consequent), add Follow(antecedent) to prediction
    // symbols
    if (first_of_consequent.find(gr_.st_.EPSILON_) !=
        first_of_consequent.end()) {
        std::cout << "\t- Since ε ∈ First(" << consequent_str
                  << "), add Follow(" << antecedent
                  << ") to prediction symbols.\n";
        const auto& follow_antecedent = Follow(antecedent);
        prediction_symbols.insert(follow_antecedent.begin(),
                                  follow_antecedent.end());

        std::cout << "\t\tFollow(" << antecedent << ") = { ";
        for (const std::string& symbol : follow_antecedent) {
            std::cout << symbol << " ";
        }
        std::cout << "}\n";
    }

    // Step 4: Display the final prediction symbols
    std::cout << "3. Final prediction symbols for " << antecedent << " -> "
              << consequent_str << " are: { ";
    for (const std::string& symbol : prediction_symbols) {
        std::cout << symbol << " ";
    }
    std::cout << "}\n";
}

void LL1Parser::TeachLL1Table() {
    std::cout << "1. Process of building the LL(1) table:\n";
    std::cout << "LL(1) table is built by defining all prediction symbols for "
                 "each rule.\n";
    size_t i = 1;
    for (const auto& [nt, prods] : gr_.g_) {
        for (const production& prod : prods) {
            std::unordered_set<std::string> pred;
            pred = PredictionSymbols(nt, prod);
            std::cout << "\t" << i + ". PD( " << nt << " -> ";
            for (const std::string& symbol : prod) {
                std::cout << symbol << " ";
            }
            std::cout << ") = { ";
            for (const std::string& symbol : pred) {
                std::cout << symbol << " ";
            }
            std::cout << "}\n";
        }
    }
    std::cout
        << "2. A grammar meets LL condition if for every non terminal, none of "
           "its productions have common prediction symbols.\nThat is, for "
           "every rule A -> X and A -> Y, PS(A -> X) ∩ S(A -> Y) = ∅\n";
    bool has_conflicts = false;
    for (const auto& [nt, cols] : ll1_t_) {
        for (const auto& col : cols) {
            if (col.second.size() > 1) {
                has_conflicts                       = true;
                const std::vector<production> prods = col.second;
                std::cout << "- Conflict under " << col.first << ":\n";
                for (const production& prod : prods) {
                    std::cout << "\tPD( " << nt << " -> ";
                    for (const std::string& symbol : prod) {
                        std::cout << symbol << " ";
                    }
                    std::cout << ")\n";
                }
            }
        }
    }
    if (!has_conflicts) {
        std::cout << "3. Prediction symbols sets does not overlap. Grammar is "
                     "LL(1). LL(1) table is built by the following way.\n";
        std::cout << "4. Have one row for each non terminal symbol ("
                  << gr_.st_.non_terminals_.size()
                  << " rows), and one column for each terminal plus "
                  << gr_.st_.EOL_ << " (" << gr_.st_.terminals_.size()
                  << " columns).\n";
        std::cout
            << "5. Place α in the cell (A,β) if β ∈ PS(A ->α), empty if not.\n";
        for (const auto& [nt, cols] : ll1_t_) {
            for (const auto& col : cols) {
                std::cout << "\t- ll1(" << nt << ", " << col.first << ") = ";
                for (const std::string& symbol : col.second.at(0)) {
                    std::cout << symbol << " ";
                }
                std::cout << "\n";
            }
        }
        std::cout << "6. Final LL(1) table:\n";
        PrintTable();
    } else {
        std::cout << "3. Since there is at least two sets with common symbols "
                     "under the same non terminal, grammar is not LL(1).\n";
    }
}

void LL1Parser::PrintTable() {
    using namespace tabulate;
    Table table;

    Table::Row_t                          headers = {"Non-terminal"};
    std::unordered_map<std::string, bool> columns;

    for (const auto& outerPair : ll1_t_) {
        for (const auto& innerPair : outerPair.second) {
            columns[innerPair.first] = true;
        }
    }

    for (const auto& col : columns) {
        headers.push_back(col.first);
    }

    auto& header_row = table.add_row(headers);
    header_row.format()
        .font_align(FontAlign::center)
        .font_color(Color::yellow)
        .font_style({FontStyle::bold});

    std::vector<std::string> non_terminals;
    for (const auto& outerPair : ll1_t_) {
        non_terminals.push_back(outerPair.first);
    }

    std::sort(non_terminals.begin(), non_terminals.end(),
              [this](const std::string& a, const std::string& b) {
                  if (a == gr_.axiom_)
                      return true; // Axiom comes first
                  if (b == gr_.axiom_)
                      return false; // Axiom comes first
                  return a < b;     // Sort the rest alphabetically
              });

    for (const std::string& nonTerminal : non_terminals) {
        Table::Row_t row_data = {nonTerminal};

        for (const auto& col : columns) {
            auto innerIt = ll1_t_.at(nonTerminal).find(col.first);
            if (innerIt != ll1_t_.at(nonTerminal).end()) {
                std::string cell_content;
                for (const auto& prod : innerIt->second) {
                    cell_content += "[ ";
                    for (const std::string& elem : prod) {
                        cell_content += elem + " ";
                    }
                    cell_content += "] ";
                }
                row_data.push_back(cell_content);
            } else {
                row_data.push_back("-");
            }
        }

        table.add_row(row_data);
    }

    table[0].format().font_color(Color::cyan).font_style({FontStyle::bold});
    for (size_t i = 1; i < table.size(); ++i) {
        for (size_t j = 1; j < table[i].size(); ++j) {
            if (table[i][j].get_text().find("] [") != std::string::npos) {
                table[i][j].format().font_color(Color::red);
            }
        }
    }
    table.format().font_align(FontAlign::center);
    table.column(0).format().font_color(Color::cyan);

    // Print the table
    std::cout << table << std::endl;
}