//
// blocking_file_copy.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include "asio.hpp"

#if defined(ASIO_HAS_FILE)

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: blocking_file_copy <from> <to>\n";
      return 1;
    }

    xio::io_context io_context;

    xio::stream_file from_file(io_context, argv[1],
        xio::stream_file::read_only);

    xio::stream_file to_file(io_context, argv[2],
        xio::stream_file::write_only
          | xio::stream_file::create
          | xio::stream_file::truncate);

    char data[4096];
    std::error_code error;
    for (;;)
    {
      std::size_t n = from_file.read_some(xio::buffer(data), error);
      if (error)
        break;
      xio::write(to_file, xio::buffer(data, n), error);
      if (error)
        break;
    }

    if (error && error != xio::error::eof)
    {
      std::cerr << "Error copying file: " << error.message() << "\n";
      return 1;
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
    return 1;
  }

  return 0;
}

#else // defined(ASIO_HAS_FILE)
int main() {}
#endif // defined(ASIO_HAS_FILE)
