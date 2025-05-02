// ====== header ======
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

class SimpleLang {
public:
    void execute(const std::string& code);

private:
    struct Block {
        std::vector<std::string> lines;
        size_t pc = 0;
        bool execute = true;
        int scope_level = 0;
    };

    std::vector<Block> blocks;
    std::vector<std::unordered_map<std::string, int64_t>> scopes;

    // 完整函数声明列表
    // 在class.cpp中声明类内函数时，必须在此注册，否则会引起错误

    void skip_whitespace(const std::string& s, size_t& pos);
    int64_t parse_expression(const std::string& code, size_t& pos);
    int64_t parse_term(const std::string& code, size_t& pos);
    int64_t parse_factor(const std::string& code, size_t& pos);
    std::vector<std::string> split_lines(const std::string& code);
    void process_line(const std::string& line, size_t& pos, Block& current_block);
    void handle_if(const std::string& line, size_t& pos, Block& parent_block);
    void handle_var(const std::string& line, size_t& pos);
    int64_t get_variable(const std::string& name) const;
};
