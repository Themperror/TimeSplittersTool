#include <string>
#include "Tool/TS2Importer/TS2Importer.h"

int main(int argc, char** argv)
{
	if (argc >= 4 && !strcmp(argv[1], "extractpak"))
	{
		TS2Importer importer;
		importer.PAKExtract(argv[2], argv[3]);
	}
	else if (argc > 1)
	{
		Utility::Print("CMDLine Usage:\nextractpak \"ExtractedIsoDir\" \"OutputDir\"");
	}
}