#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <filesystem>

#define LOG(value, args) std::cout << value << std::endl

namespace po = boost::program_options;
namespace fs = std::filesystem;

struct TargetData
{
	fs::path RootDirectory;
	std::string FileExtension;
};

int main(int argc, char* argv[])
{
	po::positional_options_description p_desc;
	p_desc.add("dir", -1);

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce help message")
		("dir,d", po::value<std::string>(), "target file directory")
		("extension,e", po::value<std::string>(), "target file extension")
		("remove,r", po::value<std::string>(), "remove sub string from all file names");

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).positional(p_desc).run(), vm);
	po::notify(vm);

	//if (vm.count("help"))
	//{
	//	std::cout << "Command line options:" << '\n';
	//	std::cout << "-h [--help]                   : display options" << '\n';
	//	std::cout << "-d [--dir] args               : target directory" << '\n';
	//	std::cout << "-e [--extension] args         : target file extension" << '\n';
	//	std::cout << "-r [--remove] args            : remove a sub string from file names" << '\n';
	//	return 0;
	//}

	TargetData target;

	// Get Target Directory	

	if (vm.count("dir"))
	{
		target.RootDirectory = vm["dir"].as<std::string>();
	}
	else
	{
		target.RootDirectory = fs::current_path().string();
	}

	if (target.RootDirectory.empty())
	{
		LOG("Fatal error: could not resolve target directory");
		return 1;
	}

	if (!fs::is_directory(target.RootDirectory))
	{
		LOG("Fatal error: " << target.RootDirectory.string() << " is not a valid directory");
		return 1;
	}

	LOG("Target Directory: " << target.RootDirectory.string());

	// Get Extension

	if (vm.count("extension"))
	{
		target.FileExtension = vm["extension"].as<std::string>();

		if (target.FileExtension.rfind('.', 0) != 0) // add '.' to extension
		{
			target.FileExtension.insert(0, 1, '.');
		}
		
		LOG("Target Extension: " << target.FileExtension);
	}
	else
	{
		LOG("No file extension specified...");
	}

	// Get File Names To Edit

	std::vector<fs::path> file_paths;

	LOG("Searching for files...");

	for (auto const& p : fs::recursive_directory_iterator(target.RootDirectory))
	{
		if (!p.is_regular_file())
		{
			continue;
		}

		LOG(p.path().extension());

		if (target.FileExtension.empty() || p.path().extension() == target.FileExtension)
		{
			file_paths.push_back(p);
			LOG(p.path().string());
		}
	}

	LOG("Found " << file_paths.size() << "files");

	// Edit Files

	if (vm.count("remove"))
	{
		const auto remove_str = vm["remove"].as<std::string>();

		for (auto const& file_path : file_paths)
		{
			if (size_t index = file_path.string().find(remove_str))
			{
				const auto new_name = file_path.string().erase(index, remove_str.size());

				fs::rename(file_path, new_name);

				if(fs::exists(new_name))
				{
					//fs::remove(file_path);
				}				
			}
		}
	}
}