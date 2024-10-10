#pragma once
#include <format>
#include <string>
#include <unordered_map>

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

template <> struct std::formatter<TokenType> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const TokenType &p, FormatContext &ctx) const {
    const std::string msg;
    switch (p) {
    case TokenType::Number:
      return std::format_to(ctx.out(), "Number");
    case TokenType::UnaryOperator:
      return std::format_to(ctx.out(), "UnaryOperator");
    case TokenType::BinaryOperator:
      return std::format_to(ctx.out(), "BinaryOperator");
    case TokenType::Parenthesis:
      return std::format_to(ctx.out(), "Parenthesis");
    }
    return std::format_to(ctx.out(), "{}", msg);
  }
};

template <> struct std::formatter<Token> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const Token &p, FormatContext &ctx) const {
    return std::format_to(
        ctx.out(), "Token(type: {}, char: {}, value: {}, prec: {}, name: {})",
        p.type, p.ch, p.value, p.prec, p.name);
  }
};

float evaluate(const std::string &expr,
               const std::unordered_map<std::string, float> &vars);
