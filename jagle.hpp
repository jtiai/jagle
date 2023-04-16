#pragma once

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

extern int _jagle_data_idx;
extern std::variant<int, float, std::string> _jagle_data[];

std::variant<int, float> val(const std::string& s) {
    int int_val = std::stoi(s);
    return (std::to_string(int_val) == s) ? int_val : std::stof(s);
}

template <typename T, typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::variant<T, Ts...>& var) {
    std::visit([&os](const auto& val) { os << val; }, var);
    return os;
}

void data_read(int& x) {
    x = std::get<int>(_jagle_data[_jagle_data_idx]);
    _jagle_data_idx++;
}

void data_read(float& x) {
    x = std::get<float>(_jagle_data[_jagle_data_idx]);
    _jagle_data_idx++;
}

void data_read(std::string& x) {
    x = std::get<std::string>(_jagle_data[_jagle_data_idx]);
    _jagle_data_idx++;
}

void data_restore() {
    _jagle_data_idx = 0;
}


void prompt_input(const std::string& prompt, int& variable, bool allow_empty, int default_value = 0) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);

    if (input.empty()) {
        if (allow_empty) {
            variable = default_value;
            return;
        }
        else {
            std::cerr << "Input cannot be empty." << std::endl;
            prompt_input(prompt, variable, allow_empty, default_value);
            return;
        }
    }

    std::stringstream ss(input);
    ss >> variable;

    if (ss.fail() || !ss.eof()) {
        std::cerr << "Invalid input. Please try again." << std::endl;
        prompt_input(prompt, variable, allow_empty, default_value);
        return;
    }
}

void prompt_input(const std::string& prompt, float& variable, bool allow_empty, float default_value = 0.0f) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);

    if (input.empty()) {
        if (allow_empty) {
            variable = default_value;
            return;
        }
        else {
            std::cerr << "Input cannot be empty." << std::endl;
            prompt_input(prompt, variable, allow_empty, default_value);
            return;
        }
    }

    std::stringstream ss(input);
    ss >> variable;

    if (ss.fail() || !ss.eof()) {
        std::cerr << "Invalid input. Please try again." << std::endl;
        prompt_input(prompt, variable, allow_empty, default_value);
        return;
    }
}

void prompt_input(const std::string& prompt, std::string& variable, bool allow_empty, const std::string& default_value = "") {
    std::cout << prompt;
    std::getline(std::cin, variable);

    if (variable.empty()) {
        if (allow_empty) {
            variable = default_value;
            return;
        }
        else {
            std::cerr << "Input cannot be empty." << std::endl;
            prompt_input(prompt, variable, allow_empty, default_value);
            return;
        }
    }
}