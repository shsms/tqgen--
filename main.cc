#include <iostream>
#include <cstdlib>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace std;

struct cli_opts {
    int numstk;
    int seed;
    int interval;
    string date_beg;
    string date_end;
    string start_time;
    string end_time;
    string out_file_pattern;
} opts;

void options(int argc, char* argv[]) {
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
	("help", "produce help message")
	("numstk,n",		po::value<int>		(&opts.numstk)		->default_value(100)		, "Number of stocks")
	("seed,s",		po::value<int>		(&opts.seed)		->default_value(1)		, "Randomization seed")
	("interval,i",		po::value<int>		(&opts.interval)	->default_value(25)		, "Max milliseconds between two entries.  Decreasing this increases number of lines per date.")
	("date_beg,b",		po::value<std::string>	(&opts.date_beg)	->default_value("20150101")	, "Start date in YYYYMMDD format")
	("date_end,e",		po::value<std::string>	(&opts.date_end)	->default_value("20150103")	, "End date in YYYYMMDD format")
	("start_time,s",	po::value<std::string>	(&opts.start_time)	->default_value("0930")		, "Trading start time")
	("end_time,f",		po::value<std::string>	(&opts.end_time)	->default_value("1600")		, "Trading end time")
	("out_file_pattern,o",	po::value<std::string>	(&opts.out_file_pattern)				, "Outfile name pattern with YYYYMMDD that will be replaced by date.")
	;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
	cout << desc << "\n";
	exit(0);
    }

    if (vm.count("compression")) {
	cout << "Compression level was set to " 
	     << vm["compression"].as<int>() << ".\n";
    } else {
	cout << "Compression level was not set.\n";
    }
}

int main(int argc, char* argv[]) {
    options(argc, argv);
}
