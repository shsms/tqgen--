#include <boost/program_options.hpp>
#include <cstdlib>
#include <iostream>

namespace po = boost::program_options;

struct cli_args {
  int numstk;
  int seed;
  int interval;
  std::string date_beg;
  std::string date_end;
  std::string start_time;
  std::string end_time;
  std::string out_file_pattern;
} args;

void parse_args(int argc, char *argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message");
  desc.add_options()("numstk,n",
                     po::value<int>(&args.numstk)->default_value(100),
                     "Number of stocks");
  desc.add_options()("seed,s", po::value<int>(&args.seed)->default_value(1),
                     "Randomization seed");
  desc.add_options()("interval,i",
                     po::value<int>(&args.interval)->default_value(25),
                     "Max milliseconds between two entries.  Decreasing this "
                     "increases number of lines per date.");
  desc.add_options()(
      "date_beg,b",
      po::value<std::string>(&args.date_beg)->default_value("20150101"),
      "Start date in YYYYMMDD format");
  desc.add_options()(
      "date_end,e",
      po::value<std::string>(&args.date_end)->default_value("20150103"),
      "End date in YYYYMMDD format");
  desc.add_options()(
      "start_time,s",
      po::value<std::string>(&args.start_time)->default_value("0930"),
      "Trading start time");
  desc.add_options()(
      "end_time,f",
      po::value<std::string>(&args.end_time)->default_value("1600"),
      "Trading end time");
  desc.add_options()(
      "out_file_pattern,o", po::value<std::string>(&args.out_file_pattern),
      "Outfile name pattern with YYYYMMDD that will be replaced by date.");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    exit(0);
  }
}
