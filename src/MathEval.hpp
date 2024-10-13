#pragma once
#include <format>
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

namespace math {

enum TokenType {
  Number,
  UnaryOperator,
  BinaryOperator,
  Parenthesis,
};

struct Token {
  TokenType type;
  char ch;
  float value;
  uint prec = 0;
  std::string name = "";
};

} // namespace math

template <> struct std::formatter<math::TokenType> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const math::TokenType &p, FormatContext &ctx) const {
    const std::string msg;
    switch (p) {
    case math::TokenType::Number:
      return std::format_to(ctx.out(), "Number");
    case math::TokenType::UnaryOperator:
      return std::format_to(ctx.out(), "UnaryOperator");
    case math::TokenType::BinaryOperator:
      return std::format_to(ctx.out(), "BinaryOperator");
    case math::TokenType::Parenthesis:
      return std::format_to(ctx.out(), "Parenthesis");
    }
    return std::format_to(ctx.out(), "{}", msg);
  }
};

template <> struct std::formatter<math::Token> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const math::Token &p, FormatContext &ctx) const {
    return std::format_to(
        ctx.out(), "Token(type: {}, char: {}, value: {}, prec: {}, name: {})",
        p.type, p.ch, p.value, p.prec, p.name);
  }
};

float evaluate(const std::string &expr,
               const std::unordered_map<std::string, float> &vars);
