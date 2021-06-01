// Copyright 2021 <elizavetamaikova>

#ifndef INCLUDE_CRAWLER_HPP_
#define INCLUDE_CRAWLER_HPP_

#include <gumbo.h>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/config.hpp>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <string>
#include <queue>

#include "Controller.hpp"
namespace http = boost::beast::http;

const char for_check_http[] = "http://";

struct links
{
  std::string url = "";
  int depth;
};

struct http_parse
{
  std::string http_str = "";
  int depth;
};


class Crawler
{
 public:
  Crawler();
  static void connect(class Controller &cr, std::string &reference, int depth,
                      std::mutex &mut);
  static void search_for_links(class Controller &cr, GumboNode* node,
                               int new_depth, int max_depth, std::mutex &mut);
  ~Crawler();
};

#endif // INCLUDE_CRAWLER_HPP_