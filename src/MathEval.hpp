#pragma once
#include <format>
#include <stddef.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

class parser_exception : public std::runtime_error {
public:
  explicit parser_exception(const std::string &message)
      : std::runtime_error("Parser Error: " + message) {}

  parser_exception(const std::string &message, const std::string &input,
                   size_t index)
      : std::runtime_error(std::format("Parser Error: {}\n{}\n{}\n^", message,
                                       std::string(index, ' '), input)) {}
};

namespace math {

enum class UnaryOperator { MINUS, PLUS };

struct BinaryOperator {
  char ch;
};

enum class TokenType { Number, OperatorT };

enum class OperatorType {
  Binary,
  Function,
};

struct Operator {
  OperatorType type;
  std::variant<char, std::string> value;
  size_t precedence = 0;
};

union TokenData {
  float number;
  Operator operator_;
};

using Token = std::variant<float, Operator>;

// struct Token {
//   std::variant<float, Operator> data;
// };

} // namespace math

template <> struct std::formatter<math::Operator> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(math::Operator const &op, FormatContext const &ctx) const {
    switch (op.type) {
    case math::OperatorType::Binary:
      return std::format_to(ctx.out(), "BinaryOperator({})",
                            std::get<char>(op.value));
    case math::OperatorType::Function:
      return std::format_to(ctx.out(), "Function({})",
                            std::get<std::string>(op.value));
    }
  }
};

template <> struct std::formatter<math::TokenType> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const math::TokenType &p, FormatContext &ctx) const {
    switch (p) {
    case math::TokenType::Number:
      return std::format_to(ctx.out(), "Number");
    case math::TokenType::OperatorT:
      return std::format_to(ctx.out(), "Operator");
    }
  }
};

template <> struct std::formatter<math::Token> {
  constexpr auto parse(std::format_parse_context const &ctx) const {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const math::Token &p, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "Token(value: {})",
                          std::visit([](auto &&x) { return x; }, p));
  }
};

float evaluate(const std::string &expr,
               const std::unordered_map<std::string, float> &vars);
