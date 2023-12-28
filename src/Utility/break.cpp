#include "break.h"
#include "common.h"


void Utility::Break()
{
#if _DEBUG
	if (IsDebuggerPresent())
	{
		DebugBreak();
	}
#endif
}