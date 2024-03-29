#include <string>
#include "Tool/TS2Importer/TS2Importer.h"

int main(int argc, char** argv)
{
	if (argc >= 4 && !strcmp(argv[1], "extractpak"))
	{
		TS2Importer importer;
		importer.PAKExtract(argv[2], argv[3]);
	}
	else if (argc >= 4 && !strcmp(argv[1], "converttex"))
	{
		TS2Importer importer;
		importer.TexConvert(argv[2], argv[3]);
	}
	else if (argc >= 4 && !strcmp(argv[1], "convertmodel"))
	{
		TS2Importer importer;
		importer.ModelConvert(argv[2], argv[3]);
	}
	else if (argc > 1)
	{
		Utility::Print("CMDLine Usage:\nextractpak \"ExtractedIsoDir\" \"OutputDir\"");
	}
	
}