#include "../../include/symbol_table.hpp"
#include "../../include/tabulate.hpp"
#include <unordered_map>
#include <vector>

void SymbolTable::PutSymbol(const std::string& identifier,
                            const std::string& regex) {
    st_.insert({identifier, {TERMINAL, regex}});
    terminals_.insert(identifier);
    terminals_wtho_eol_.insert(identifier);
}

void SymbolTable::PutSymbol(const std::string& identifier) {
    st_.insert({identifier, {NO_TERMINAL, ""}});
    non_terminals_.insert(identifier);
}

bool SymbolTable::In(const std::string& s) {
    return st_.find(s) != st_.cend();
}

bool SymbolTable::IsTerminal(const std::string& s) {
    return terminals_.find(s) != terminals_.end();
}

bool SymbolTable::IsTerminalWthoEol(const std::string& s) {
    return s != EPSILON_ && terminals_.find(s) != terminals_.end();
}

void SymbolTable::Debug() {
    using namespace tabulate;
    Table table;

    Table::Row_t header     = {"Identifier", "Type", "Regex"};
    auto&        header_row = table.add_row(header);
    header_row.format()
        .font_align(FontAlign::center)
        .font_style({FontStyle::bold});

    for (const auto& [identifier, content] : st_) {
        if (content.first == NO_TERMINAL) {
            continue;
        }
        table.add_row({identifier, "TERMINAL", content.second});
    }

    for (const auto& identifier : non_terminals_) {
        table.add_row({identifier, "NON TERMINAL", "-"});
    }

    table.format().font_align(FontAlign::center);
    table.column(0).format().font_color(Color::yellow);
    table.column(1).format().font_color(Color::magenta);
    table.column(2).format().font_color(Color::cyan);

    std::cout << table << "\n";
}
