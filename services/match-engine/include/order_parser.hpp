#ifndef ORDER_PARSER_HPP
#define ORDER_PARSER_HPP

#include "book.hpp"
#include <string>

class OrderParser {
public:
    static Order parse(const std::string& jsonString);
};

#endif
