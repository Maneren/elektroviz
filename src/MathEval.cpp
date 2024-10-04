#include "MathEval.hpp"
#include <cctype>
#include <cmath>
#include <functional>
#include <print>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>

class parser_exception : public std::runtime_error {
public:
  parser_exception(const std::string &message)
      : std::runtime_error("Parser Error: " + message) {}

  parser_exception(const std::string &message, const std::string &input,
                   int index)
      : std::runtime_error(std::format("Parser Error: {}\n{}\n{}\n^", message,
                                       std::string(index, ' '), input)) {}
};

const std::unordered_map<std::string, std::function<float(float)>> functions = {
    {"sin", [](float x) { return std::sin(x); }},
    {"cos", [](float x) { return std::cos(x); }},
    {"tan", [](float x) { return std::tan(x); }},
    {"asin", [](float x) { return std::asin(x); }},
    {"acos", [](float x) { return std::acos(x); }},
    {"atan", [](float x) { return std::atan(x); }},
    {"sqrt", [](float x) { return std::sqrt(x); }},
    {"exp", [](float x) { return std::exp(x); }},
    {"log", [](float x) { return std::log(x); }},
    {"abs", [](float x) { return std::abs(x); }},
    {"ceil", [](float x) { return std::ceil(x); }},
    {"floor", [](float x) { return std::floor(x); }},
    {"sign", [](float x) { return std::signbit(x) ? -1.f : 1.f; }},
};

bool is_letter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int op_precedence(char ch) {
  switch (ch) {
  case 0: // function
    return 4;
  case '^':
    return 3;
  case '*':
  case '/':
    return 3;
  case '+':
  case '-':
    return 1;
  default:
    return -1;
  }
}

static void push_with_precedence(std::stack<Token> &stack, Token token,
                                 std::deque<Token> &postfix) {
  int precedence = op_precedence(token.ch);
  while (!stack.empty() && op_precedence(stack.top().ch) >= precedence) {
    postfix.push_back(stack.top());
    stack.pop();
  }
  stack.push(token);
}

std::string load_brackets(const std::string &input,
                          std::string::const_iterator &it) {
  std::string brackets;
  int bracketLevel = 1;
  auto startIndex = it;

  while (++it != input.end()) {
    if (*it == ')') {
      bracketLevel--;
    } else if (*it == '(') {
      bracketLevel++;
    }

    if (bracketLevel == 0)
      break;

    brackets += *it;
  }

  if (bracketLevel != 0)
    throw parser_exception("Missmatched bracket found", input,
                           std::distance(input.begin(), startIndex));

  ++it;
  return brackets;
}

std::deque<Token>
parse_to_RPN(const std::string &input,
             const std::unordered_map<std::string, float> &vars) {
  std::deque<Token> rpn_queue;
  std::stack<Token> op_stack;

  auto it = input.begin();
  bool wasNumber = false;

  while (it != input.end()) {
    if (std::isspace(*it)) {
      ++it;
      continue;
    }

    char character = *it;

    if (std::isdigit(character)) {
      if (wasNumber)
        throw parser_exception("Unexpected digit", input,
                               std::distance(input.begin(), it));
      wasNumber = true;

      bool isDecimal = false;

      float value = character - '0';
      float factor = 1;

      while (++it != input.end()) {
        if (std::isdigit(*it)) {
          if (isDecimal) {
            factor *= 0.1;
            value += (*it - '0') * factor;
          } else {
            value *= 10;
            value += (*it - '0');
          }
        } else if (*it == '.') {
          if (isDecimal) {
            throw parser_exception("Unexpected character '.'", input,
                                   std::distance(input.begin(), it));
          }

          if (it + 1 == input.end() || !std::isdigit(*(it + 1))) {
            throw parser_exception("Expected digit after decimal dot", input,
                                   std::distance(input.begin(), it + 1));
          }

          isDecimal = true;
        } else {
          break;
        }
      }
      rpn_queue.push_back({TokenType::Number, 0, value});
    } else if (is_letter(character)) {
      if (wasNumber) {
        push_with_precedence(op_stack, Token{TokenType::BinaryOperator, '*', 0},
                             rpn_queue);
      }

      auto start = it;
      while (++it != input.end() && is_letter(*it)) {
        continue;
      }
      std::string token(start, it);
      if (auto var = vars.find(token); var != vars.end()) {
        rpn_queue.push_back({TokenType::Number, 0, var->second});
        wasNumber = true;
      } else {
        push_with_precedence(op_stack, {TokenType::UnaryOperator, 0, 0, token},
                             rpn_queue);
        wasNumber = false;
      }
    } else {
      switch (character) {
      case '+':
      case '-':
        if (!wasNumber)
          rpn_queue.push_back({TokenType::UnaryOperator, character, 0});
      case '^':
      case '*':
      case '/':
        if (!wasNumber) {
          throw parser_exception(
              std::format("Unexpected symbol '{}'. Number expected", character),
              input, std::distance(input.begin(), it));
        }

        wasNumber = false;
        push_with_precedence(op_stack,
                             Token{TokenType::BinaryOperator, character, 0},
                             rpn_queue);
        ++it;
        break;
      case '(': {
        if (wasNumber) {
          push_with_precedence(
              op_stack, Token{TokenType::BinaryOperator, '*', 0}, rpn_queue);
          wasNumber = false;
        }
        std::string brackets = load_brackets(input, it);

        float result = evaluate(brackets, vars);

        rpn_queue.push_back({TokenType::Number, 0, result});
        wasNumber = true;
        break;
      }
      default:
        throw parser_exception(std::format("Unexpected symbol '{}'", character),
                               input, std::distance(input.begin(), it));
      }
    }
  }

  while (!op_stack.empty()) {
    rpn_queue.push_back(op_stack.top());
    op_stack.pop();
  }

  return rpn_queue;
}

auto evaluate_RPN(const std::deque<Token> &postfix) {
  std::stack<float> stack{};


  for (const auto &token : postfix) {
    if (token.type == TokenType::Number) {
      stack.push(token.value);
    } else if (token.type == TokenType::UnaryOperator && token.ch != 0) {
      double op = stack.top();
      stack.pop();
      if (token.ch == '-')
        op = -op;
      stack.push(op);
    } else if (token.type == TokenType::UnaryOperator && token.ch == 0) {
      if (auto function = functions.find(token.name);
          function != functions.end()) {
        auto n = stack.top();
        stack.pop();
        stack.push(function->second(n));
      } else {
        throw parser_exception(
            std::format("Unknown function '{}'", token.name));
      }
    } else if (token.type == TokenType::BinaryOperator && token.ch != 0) {
      auto op2 = stack.top();
      stack.pop();
      auto op1 = stack.top();
      stack.pop();
      if (token.ch == '+')
        op1 = op1 + op2;
      else if (token.ch == '-')
        op1 = op1 - op2;
      else if (token.ch == '*')
        op1 = op1 * op2;
      else if (token.ch == '/')
        op1 = op1 / op2;
      else if (token.ch == '^')
        op1 = std::pow(op1, op2);
      stack.push(op1);
    }
  }

  if (stack.size() != 1)
    throw parser_exception("Invalid expression");


  return stack.top();
}

std::string trim(const std::string &str) {
  auto first = str.find_first_not_of(' ');
  if (first == std::string::npos)
    return "";
  auto last = str.find_last_not_of(' ');
  return str.substr(first, last - first + 1);
}

float evaluate(const std::string &expr,
               const std::unordered_map<std::string, float> &vars) {
  std::string trimmedInput = trim(expr);

  if (trimmedInput.empty())
    throw parser_exception("No input");

  auto rpn = parse_to_RPN(trimmedInput, vars);
  return evaluate_RPN(rpn);
}
