#include <iostream>
#include <string.h>
#include "cartridge.h"
#include "emulator.h"

namespace nes
{
	enum OptionCode
	{
		OPTION_HELP = (1 << 0)
	};

	struct Options
	{
		const char*  rom_path = nullptr;
		int          option_code = 0;
		int          scale = 2;

		bool ParseOptions(int argc, char *argv[]);
	};

	bool Options::ParseOptions(int argc, char **argv)
	{
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			{
				this->option_code |= OptionCode::OPTION_HELP;
				return true;
			}
			else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scale") == 0)
			{
				if (++i < argc)
				{
					this->scale = std::stoi(argv[i]);
				}
				else
				{
					std::cout << "Missing argument for option 's'." << std::endl;
					return false;
				}
				if (this->scale <= 0)
				{
					std::cout << "Error argument for option 's', the argument must be a positive integer." << std::endl;
					return false;
				}
			}
			else if (argv[i][0] == '-')
			{
				std::cout << "Unrecognized option." << std::endl;
				return false;
			}
			else
			{
				this->rom_path = argv[i];
			}
		}
		if (!this->rom_path)
		{
			std::cout << "Argument Required : Rom Path." << std::endl;
			return false;
		}
		return true;
	}
}

int main(int argc, char *argv[])
{
	nes::Options options;
	if (!options.ParseOptions(argc, argv))
	{
		std::cout << "Parse options error! Use -h or --help to get help." << std::endl;
		return 0;
	}
	if (options.option_code & nes::OPTION_HELP)
	{
		std::cout << "NesEmulator can help you emulate .nes file.\n"
				  << "Usage: NesEmulator [options] rom_path\n\n"
				  << "Options: \n"
				  << "-h, --help                 show help\n"
				  << "-s, --scale                set scale, must be positive integer, default is 2\n"
				  << std::endl;
		return 0;
	}
	
	nes::Cartridge cartridge;
	if (!cartridge.LoadFromFile(options.rom_path))
	{
		std::cout << "Failed to load .nes from " << options.rom_path << std::endl;
		return 0;
	}

	nes::Emulator emulator;
	emulator.PutInCartridge(&cartridge);

	return 0;
}
