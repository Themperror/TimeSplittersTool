#include <string>
#include "Tool/TS2Importer/TS2Importer.h"

int main(int argc, char** argv)
{
	if (argc >= 4 && !strcmp(argv[2], "export"))
	{
		TS2Importer importer;
		importer.Import(argv[3]);
	}
}