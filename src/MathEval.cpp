#include "MathEval.hpp"
#include "utils.hpp"
#include <cmath>
#include <deque>
#include <functional>
#include <stack>

namespace math {

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

int op_precedence(Operator &op, uint nesting_level) {
  nesting_level *= 5; // one more than the max precedence
  //
  if (op.type == OperatorType::Function) {
    return nesting_level + 4;
  }

  switch (std::get<char>(op.value)) {
  case '^':
    return nesting_level + 3;
  case '*':
  case '/':
  case '%':
    return nesting_level + 2;
  case '+':
  case '-':
    return nesting_level + 1;
  default:
    return -1;
  }
}

static void push_with_precedence(
    std::stack<Operator> &stack,
    Operator &&token,
    std::deque<Token> &postfix,
    const size_t nesting_level
) {
  token.precedence = op_precedence(token, nesting_level);
  while (!stack.empty() && stack.top().precedence >= token.precedence) {
    postfix.emplace_back(std::move(stack.top()));
    stack.pop();
  }
  stack.push(token);
}

std::string
load_brackets(const std::string &input, std::string::const_iterator &it) {
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
    throw parser_exception(
        "Missmatched bracket found",
        input,
        std::distance(input.begin(), startIndex)
    );

  ++it;
  return brackets;
}

std::deque<Token> parse_to_RPN(
    const std::string &input, const std::unordered_map<std::string, float> &vars
) {
  std::deque<Token> rpn_queue{};
  std::stack<Operator> op_stack{};

  auto it = input.begin();
  bool wasNumber = false;

  size_t bracketLevel = 0;

  while (it != input.end()) {
    if (std::isspace(*it)) {
      ++it;
      continue;
    }

    char character = *it;

    if (std::isdigit(character)) {
      if (wasNumber)
        throw parser_exception(
            "Unexpected digit", input, std::distance(input.begin(), it)
        );
      wasNumber = true;

      bool isDecimal = false;

      float value = character - '0';
      float factor = 1;

      while (++it != input.end()) {
        if (std::isdigit(*it)) {
          if (isDecimal) {
            factor *= 0.1f;
            value += (*it - '0') * factor;
          } else {
            value *= 10;
            value += (*it - '0');
          }
        } else if (*it == '.') {
          if (isDecimal) {
            throw parser_exception(
                "Unexpected character '.'",
                input,
                std::distance(input.begin(), it)
            );
          }

          if (it + 1 == input.end() || !std::isdigit(*(it + 1))) {
            throw parser_exception(
                "Expected digit after decimal dot",
                input,
                std::distance(input.begin(), it + 1)
            );
          }

          isDecimal = true;
        } else {
          break;
        }
      }
      rpn_queue.emplace_back(value);
      wasNumber = true;
    } else if (is_letter(character)) {
      if (wasNumber) {
        push_with_precedence(
            op_stack,
            Operator{OperatorType::Binary, {'*'}},
            rpn_queue,
            bracketLevel
        );
      }

      auto start = it;
      while (++it != input.end() && is_letter(*it)) {
        continue;
      }
      std::string token(start, it);
      if (auto var = vars.find(token); var != vars.end()) {
        rpn_queue.emplace_back(var->second);
        wasNumber = true;
      } else {
        push_with_precedence(
            op_stack,
            Operator{OperatorType::Function, token},
            rpn_queue,
            bracketLevel
        );
        wasNumber = false;
      }
    } else {
      switch (character) {
      case '+':
      case '-':
        if (!wasNumber) {
          rpn_queue.emplace_back(0.f);
          wasNumber = true;
        }
        [[fallthrough]];
      case '^':
      case '*':
      case '/':
      case '%':
        if (!wasNumber) {
          throw parser_exception(
              std::format("Unexpected symbol '{}'. Number expected", character),
              input,
              std::distance(input.begin(), it)
          );
        }

        wasNumber = false;
        push_with_precedence(
            op_stack,
            Operator{OperatorType::Binary, character},
            rpn_queue,
            bracketLevel
        );
        ++it;
        break;
      case '(': {
        if (wasNumber) {
          push_with_precedence(
              op_stack,
              Operator{OperatorType::Binary, '*'},
              rpn_queue,
              bracketLevel
          );
        }

        bracketLevel++;
        it++;
        wasNumber = false;
        break;
      }
      case ')':
        if (bracketLevel == 0)
          throw parser_exception(
              "Missmatched bracket found",
              input,
              std::distance(input.begin(), it)
          );

        bracketLevel--;
        it++;
        wasNumber = true;
        break;
      default:
        throw parser_exception(
            std::format("Unexpected symbol '{}'", character),
            input,
            std::distance(input.begin(), it)
        );
      }
    }
  }

  while (!op_stack.empty()) {
    rpn_queue.emplace_back(op_stack.top());
    op_stack.pop();
  }

  return rpn_queue;
}

auto evaluate_RPN(const std::deque<Token> &postfix) {
  std::stack<float> stack{};

  for (const auto &token : postfix) {
    std::visit(
        [&stack](auto &&token) {
          using T = std::decay_t<decltype(token)>;
          if constexpr (std::is_same_v<T, float>) {
            stack.push(token);
          } else if constexpr (std::is_same_v<T, Operator>) {
            switch (token.type) {
            case OperatorType::Binary: {
              auto op2 = stack.top();
              stack.pop();
              auto op1 = stack.top();
              stack.pop();
              auto ch = std::get<char>(token.value);
              if (ch == '+')
                op1 = op1 + op2;
              else if (ch == '-')
                op1 = op1 - op2;
              else if (ch == '*')
                op1 = op1 * op2;
              else if (ch == '/')
                op1 = op1 / op2;
              else if (ch == '^')
                op1 = std::pow(op1, op2);
              else if (ch == '%')
                op1 = std::fmod(op1, op2);
              stack.push(op1);
            }; break;
            case OperatorType::Function: {
              auto function_name = std::get<std::string>(token.value);
              auto function = functions.find(function_name);

              if (function == functions.end())
                throw parser_exception(
                    std::format("Unknown function '{}'", function_name)
                );

              auto n = stack.top();
              stack.pop();
              stack.push(function->second(n));
            }; break;
            }
          } else {
            static_assert(false, "non-exhaustive visitor!");
          }
        },
        token
    );
  }

  if (stack.size() != 1)
    throw parser_exception("Invalid expression");

  return stack.top();
}

} // namespace math

float evaluate(
    const std::string &expr, const std::unordered_map<std::string, float> &vars
) {
  std::string trimmedInput = trim(expr);

  if (trimmedInput.empty())
    throw parser_exception("No input");

  auto rpn = math::parse_to_RPN(trimmedInput, vars);
  return math::evaluate_RPN(rpn);
}
