#include <Crawler.hpp>
#include <Controller.hpp>
#include <boost/program_options.hpp>

using namespace boost::program_options;

int main(int argc, char *argv[]) {
  std::string url = "";  // = "http://kremlin.ru/multimedia/video";
  int depth;
  int network_threads;
  int parser_threads;
  std::string link_to_file = "";

  options_description desc("General options");
  std::string task_type;

  desc.add_options()
      ("url", value(&url), "url to download")
      ("depth", value(&depth), "recursion depth")
      ("network_threads", value(&network_threads), "network_threads")
      ("parser_threads", value(&parser_threads), "parser_threads")
      ("output", value(&link_to_file), "output file");

  variables_map vm;
  try {
    parsed_options parsed = command_line_parser(argc, argv).options(desc).
        allow_unregistered().run();
    store(parsed, vm);
    url = vm["url"].as<std::string>();
    depth = vm["depth"].as<int>();
    network_threads = vm["network_threads"].as<int>();
    parser_threads = vm["parser_threads"].as<int>();
    link_to_file = vm["output"].as<std::string>();
  }
  catch(std::exception& ex) {
    std::cout << desc << std::endl;
  }

  Controller contr(link_to_file, url, depth,
                   network_threads, parser_threads);

  std::vector<std::thread> threads;
  threads.reserve(2);
  threads.emplace_back(&Controller::producer, &contr);
  threads.emplace_back(&Controller::consumer, &contr);
  for (auto &thr : threads)
  {
    thr.join();
  }

  contr.out_img();
}