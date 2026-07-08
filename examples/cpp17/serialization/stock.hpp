//
// stock.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SERIALIZATION_STOCK_HPP
#define SERIALIZATION_STOCK_HPP

#include <string>

namespace s11n_example {

/// Structure to hold information about a single stock.
struct stock
{
  std::string code;
  std::string name;
  double open_price;
  double high_price;
  double low_price;
  double last_price;
  double buy_price;
  int buy_quantity;
  double sell_price;
  int sell_quantity;
};

} // namespace s11n_example

#endif // SERIALIZATION_STOCK_HPP
