#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <filesystem>

// example command line args(P:\Artbooks -e "cbz" -r " - Satoshi Urushihara")

#define LOG(value, args) std::cout << value << std::endl

namespace po = boost::program_options;
namespace fs = std::filesystem;

const size_t MAX_PATH = 260;

struct TargetDetails
{
	fs::path RootDirectory;
	std::string FileExtension;
};

void PrintPerms(fs::perms p)
{
	std::cout << ((p & fs::perms::owner_read) != fs::perms::none ? "r" : "-")
		<< ((p & fs::perms::owner_write) != fs::perms::none ? "w" : "-")
		<< ((p & fs::perms::owner_exec) != fs::perms::none ? "x" : "-")
		<< ((p & fs::perms::group_read) != fs::perms::none ? "r" : "-")
		<< ((p & fs::perms::group_write) != fs::perms::none ? "w" : "-")
		<< ((p & fs::perms::group_exec) != fs::perms::none ? "x" : "-")
		<< ((p & fs::perms::others_read) != fs::perms::none ? "r" : "-")
		<< ((p & fs::perms::others_write) != fs::perms::none ? "w" : "-")
		<< ((p & fs::perms::others_exec) != fs::perms::none ? "x" : "-")
		<< '\n';
}

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

	if (vm.count("help"))
	{
		std::cout << "Command line options:" << '\n';
		std::cout << "-h [--help]                   : display options" << '\n';
		std::cout << "-d [--dir] args               : target directory" << '\n';
		std::cout << "-e [--extension] args         : target file extension" << '\n';
		std::cout << "-r [--remove] args            : remove a sub string from file names" << '\n';
		return 0;
	}

	TargetDetails target;

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

		if (target.FileExtension.empty() || p.path().extension() == target.FileExtension)
		{
			file_paths.push_back(p);
			LOG("Found: " << p.path().string());
		}
	}

	LOG("File Count: " << file_paths.size());

	// Edit Files

	if (vm.count("remove"))
	{
		const auto remove_str = vm["remove"].as<std::string>();

		for (fs::path const& current_path : file_paths)
		{
			if (current_path.string().size() > MAX_PATH)
			{
				LOG("Error: file path too long to rename (260 character limit)! - " << current_path);
				continue;
			}

			// calculate new file name and path
			auto new_path = current_path;

			int index = new_path.string().find(remove_str);

			if (index < 0)
			{
				// file does not contain unwanted string
				continue;
			}
			
			while (index >= 0)
			{
				new_path = new_path.string().erase(index, remove_str.size());

				index = new_path.string().find(remove_str);
			}

			if (new_path.empty() || new_path.filename().empty())
			{
				LOG("Error: Cannot rename a file to have no name or path - " << current_path);
				continue;
			}

			// create new directory
			auto new_directory = new_path;
			new_directory.remove_filename();

			if (!fs::exists(new_directory))
			{
				fs::create_directories(new_directory);
				fs::permissions(new_directory, fs::perms::all);
			}

			// rename file
			try
			{
				LOG("Attempting Rename: " << current_path << " -> " << new_path);
				fs::rename(current_path, new_path);				
			}
			catch (const std::exception& e)
			{
				LOG("Failed: " << e.what());
				break;
			}

			LOG("Success!");

			if (fs::exists(new_path))
			{				
				auto old_path = current_path;
				old_path.remove_filename();
				
				if (fs::exists(old_path))
				{
					try
					{
						fs::remove(old_path);
					}
					catch (std::exception const& e)
					{
						LOG("Error cleaning up: " << e.what());
					}				
				}				
			}
		}
	}
}