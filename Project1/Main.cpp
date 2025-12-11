#include "Hook.h"
#include "HotkeyConfig.h"
#include "Util.h"
#include <strsafe.h>
#include <system_error>
//#include <tchar.h>
#include <thread>
#include <vector>
#include <windows.h>


constexpr auto THREADS = 1;

void HookThread();

int wmain(int argc, wchar_t* argv[])
{
	std::vector<std::thread> threads;
	threads.reserve(THREADS);

	try
	{
		for (int i = 0; i < THREADS; ++i)
		{
			threads.emplace_back(HookThread);
		}
	}
	catch (const std::system_error&)
	{
		ErrorHandler(TEXT("std::thread creation"));
		ExitProcess(3);
	}

	for (auto& t : threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}


	return 0;
}