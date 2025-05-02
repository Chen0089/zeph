// ====== class ======
#include "header.hh"
#include <iostream>
#include <sstream>
#include <cctype>
#include <stdexcept>

using namespace std;

// 此处为类内函数的实现

// 空格跳过函数
void SimpleLang::skip_whitespace(const string& s, size_t& pos) {
    while (pos < s.size() && isspace(s[pos])) pos++;
}

// 表达式解析全实现
int64_t SimpleLang::parse_expression(const string& code, size_t& pos) {
    int64_t left = parse_term(code, pos);
    while (true) {
        skip_whitespace(code, pos);
        if (pos >= code.size()) break;
        char op = code[pos];
        if (op == '+' || op == '-') {
            pos++;
            int64_t right = parse_term(code, pos);
            left = (op == '+') ? left + right : left - right;
        } else {
            break;
        }
    }
    return left;
}

int64_t SimpleLang::parse_term(const string& code, size_t& pos) {
    int64_t left = parse_factor(code, pos);
    while (true) {
        skip_whitespace(code, pos);
        if (pos >= code.size()) break;
        char op = code[pos];
        if (op == '*' || op == '/') {
            pos++;
            int64_t right = parse_factor(code, pos);
            if (op == '*') left *= right;
            else {
                if (right == 0) throw runtime_error("除以零");
                left /= right;
            }
        } else {
            break;
        }
    }
    return left;
}

int64_t SimpleLang::parse_factor(const string& code, size_t& pos) {
    skip_whitespace(code, pos);
    if (pos >= code.size()) throw runtime_error("无效表达式");

    if (code[pos] == '(') {
        pos++;
        int64_t val = parse_expression(code, pos);
        skip_whitespace(code, pos);
        if (pos >= code.size() || code[pos++] != ')') 
            throw runtime_error("缺少右括号");
        return val;
    }

    if (isdigit(code[pos])) {
        int64_t num = 0;
        while (pos < code.size() && isdigit(code[pos])) {
            const int digit = code[pos] - '0';
            if (num > (INT64_MAX - digit)/10)
                throw overflow_error("整数溢出");
            num = num * 10 + digit;
            pos++;
        }
        return num;
    }

    if (isalpha(code[pos])) {
        string name;
        while (pos < code.size() && isalnum(code[pos])) name += code[pos++];
        return get_variable(name);
    }

    throw runtime_error("无效表达式");
}

// 代码分割实现
vector<string> SimpleLang::split_lines(const string& code) {
    istringstream iss(code);
    vector<string> lines;
    string line;
    while (getline(iss, line)) {
        if (!line.empty()) lines.push_back(line);
    }
    return lines;
}

// 完整流程处理
void SimpleLang::process_line(const string& line, size_t& pos, Block& current_block) {
    skip_whitespace(line, pos);
    if (pos >= line.size()) return;

    // 在制作新的命令时，请在这里处理命令关键字
    // 如果不处理，关键字识别不到，将导致你的新增的命令在测试时毫无效果
    if (line.substr(pos, 2) == "if") {
        handle_if(line, pos, current_block);
    }
    else if (line[pos] == '{') {
        scopes.push_back({});
        current_block.scope_level++;
    }
    else if (line[pos] == '}') {
        if (!scopes.empty() && current_block.scope_level > 0) {
            scopes.pop_back();
            current_block.scope_level--;
        }
    }
    else if (current_block.execute) {
        if (line.substr(pos, 5) == "print") {
            pos += 5;
            skip_whitespace(line, pos);
            cout << parse_expression(line, pos) << endl;
        }
        else if (line.substr(pos, 3) == "var") {
            handle_var(line, pos);
        }
    }
}

// 条件语句处理
void SimpleLang::handle_if(const string& line, size_t& pos, Block& parent_block) {
    pos += 2;
    skip_whitespace(line, pos);

    if (line[pos++] != '(') throw runtime_error("需要左括号");
    int64_t cond = parse_expression(line, pos);
    if (line[pos++] != ')') throw runtime_error("需要右括号");

    Block new_block;
    new_block.execute = parent_block.execute && (cond != 0);
    new_block.scope_level = parent_block.scope_level;

    // 动态代码块处理
    size_t start_line = parent_block.pc - 1;
    size_t brace_level = 0;
    size_t end_line = start_line;

    do {
        const string& l = parent_block.lines[end_line];
        for (char c : l) {
            if (c == '{') brace_level++;
            else if (c == '}') brace_level--;
        }
        end_line++;
    } while (brace_level > 0 && end_line < parent_block.lines.size());

    new_block.lines.assign(
        parent_block.lines.begin() + start_line + 1,
        parent_block.lines.begin() + end_line - (brace_level == 0 ? 1 : 0)
    );

    blocks.push_back(new_block);
    parent_block.pc = end_line;
}

// 变量声明实现
void SimpleLang::handle_var(const string& line, size_t& pos) {
    pos += 3;
    skip_whitespace(line, pos);
    string name;
    while (pos < line.size() && isalnum(line[pos])) name += line[pos++];
    skip_whitespace(line, pos);
    if (line[pos++] != '=') throw runtime_error("需要等号");
    scopes.back()[name] = parse_expression(line, pos);
}

// 变量查找实现
int64_t SimpleLang::get_variable(const string& name) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->count(name)) return it->at(name);
    }
    throw runtime_error("未定义变量: " + name);
}
