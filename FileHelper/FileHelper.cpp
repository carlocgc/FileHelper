#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include "iostream"

int main(int argc, char *argv[])
{
	namespace po = boost::program_options;
	
	po::options_description desc("Allowed options");
	desc.add_options()
	("help", "produce help message")
	("remove,rm", po::value<std::string>(), "remove a sub string from all file names: string to remove");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	std::string input;
	std::getline(std::cin, input);	
}