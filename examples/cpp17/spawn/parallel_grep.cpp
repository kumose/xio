//
// parallel_grep.cpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <xio/dispatch.h>
#include <xio/post.h>
#include <xio/strand.h>
#include <xio/thread_pool.h>
#include <fstream>
#include <iostream>
#include <string>

using xio::dispatch;
using xio::post;
using xio::strand;
using xio::thread_pool;

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: parallel_grep <string> <files...>\n";
      return 1;
    }

    thread_pool pool;
    strand<thread_pool::executor_type> output_strand(pool.get_executor());

    const std::string search_string = argv[1];
    for (int argn = 2; argn < argc; ++argn)
    {
      const std::string input_file = argv[argn];
      post(pool,
          [=]
          {
            std::ifstream is(input_file.c_str());
            std::string line;
            while (std::getline(is, line))
            {
              if (line.find(search_string) != std::string::npos)
              {
                dispatch(output_strand,
                    [=]
                    {
                      std::cout << input_file << ':' << line << std::endl;
                    });
              }
            }
          });
    }

    pool.join();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
