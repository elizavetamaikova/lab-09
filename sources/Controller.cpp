// Copyright 2021 <elizavetamaikova>

#include "Controller.hpp"

Controller::Controller(std::string link_f, std::string ref_start,
                       int depth, int net_thr, int par_thr)
{
  output_ref = link_f;
  link_to_start = ref_start;
  max_depth = depth;
  network_threads = net_thr;
  parser_threads = par_thr;
  first_download();
}

void Controller::out_img()
{
  std::ofstream fout(output_ref);
  while (!images.empty())
  {
    fout << "img: "<< images.front() << std::endl;
    images.pop();
  }
  fout.close();
}

void Controller::first_download()
{
  std::mutex mut;
  struct http_parse first;
  Crawler::connect(std::ref(*this), link_to_start, 1,
                   std::ref(mut)); // скачал стр
  first.http_str = http_for_parse.front().http_str;
  first.depth = http_for_parse.front().depth;
  http_for_parse.pop();
  char *data_for_parse = const_cast<char *>(first.http_str.c_str());
  GumboOutput *output = gumbo_parse(data_for_parse);
  Crawler::search_for_links(std::ref(*this), std::ref(output->root),
                            first.depth + 1, max_depth,
                            std::ref(mut));
}

void Controller::producer()
{
  ThreadPool tools(network_threads);
  int k = 0;
  while (k < 3)
  {
    if (!collection_links.empty())
    {
      k = 0;
      if (mut_for_download.try_lock())
      {
        std::string http_page = collection_links.front().url;
        int depth = collection_links.front().depth;
        collection_links.pop();
        mut_for_download.unlock();
        tools.enqueue(&Crawler::connect, std::ref(*this), http_page,
                      depth, std::ref(mut_for_download));
      }
    } else {
      k += 1;
      sleep(5);
    }
  }
}

void Controller::consumer()
{
  int k = 0;
  ThreadPool tools(parser_threads);
  while (k < 3)
  {
    if (!http_for_parse.empty())
    {
      k = 0;
      if (mut_for_parse.try_lock())
      {
        std::string link = http_for_parse.front().http_str;
        int depth = http_for_parse.front().depth;
        http_for_parse.pop();
        mut_for_parse.unlock();
        char *data_for_parse = const_cast<char *>(link.c_str());
        GumboOutput *output = gumbo_parse(data_for_parse);
        tools.enqueue(&Crawler::search_for_links, std::ref(*this),
                      std::ref(output->root), depth + 1, max_depth,
                      std::ref(mut_for_parse));
      }
    } else {
      k += 1;
      sleep(5);
    }
  }
}

Controller::~Controller() {}
