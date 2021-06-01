// Copyright 2021 <elizavetamaikova>

#include <Crawler.hpp>

Crawler::Crawler() {}

void Crawler::search_for_links(class Controller &cr, GumboNode *node,
                               int new_depth, int max_depth, std::mutex &mut)
{
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* href;
  GumboAttribute* src;

  if (node->v.element.tag == GUMBO_TAG_A &&
      (href = gumbo_get_attribute(&node->v.element.attributes, "href"))
      && new_depth <= max_depth)
  {
    std::string ur = href->value;
    std::string check_http = ur.substr(0, 7);
    if (check_http == for_check_http)
    {
      struct links new_link;
      new_link.depth = new_depth;
      new_link.url = href->value;
      bool t = true;
      while (t)
      {
        if (mut.try_lock())
        {
          t = false;
          cr.collection_links.push(new_link);
          std::cout << href->value << std::endl;
          mut.unlock();
        }
      }
    }
  }

  if (node->v.element.tag == GUMBO_TAG_IMG &&
      (src = gumbo_get_attribute(&node->v.element.attributes, "src")))
  {
    bool t = true;
    while (t)
    {
      if (mut.try_lock())
      {
        t = false;
        std::cout << "img: " << src->value << std::endl;
        cr.images.push(src->value);
        mut.unlock();
      }
    }
  }

  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i)
  {
    search_for_links(std::ref(cr),
                     static_cast<GumboNode*>(children->data[i]), new_depth,
                     max_depth, std::ref(mut));
  }
}

void Crawler::connect(class Controller &cr, std::string &reference, int depth,
                      std::mutex &mut)
{
  std::string data;
  std::string url = reference.substr(7);
  std::string host = url.substr(0, url.find("/"));
  std::string target = url.substr(url.find("/"));
  std::string port = "80";

  boost::asio::io_context ioc;
  boost::asio::ip::tcp::resolver resolver{ioc};
  boost::asio::ip::tcp::socket socket{ioc};

  auto const results = resolver.resolve(host, port);
  boost::asio::connect(socket, results.begin(), results.end());
  int version = 11;

  http::request<http::string_body> req{http::verb::get, target, version};
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  http::write(socket, req);
  boost::beast::flat_buffer buffer;

  http::response<http::dynamic_body> res;
  http::read(socket, buffer, res);
  data = boost::beast::buffers_to_string(res.body().data());
  struct http_parse str_http;
  str_http.http_str = data;
  str_http.depth = depth;
  bool t = true;
  while (t)
  {
    if (mut.try_lock())
    {
      t = false;
      cr.http_for_parse.push(str_http);
      mut.unlock();
    }
  }
}

Crawler::~Crawler() {}