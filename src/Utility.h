#pragma once


struct Utility
{
	inline static float* g_deltaTime = (float*)REL::ID(523660).address();
	inline static float* g_runTime = (float*)REL::ID(523662).address();

	static float GetDeltaTime() { return *g_deltaTime; }
	static float GetRunTime() { return *g_deltaTime; }

	//Reuse this to use numbers instead of just looking for exact value, gives more info that way.
	static bool CharCmpI(char& c1, char& c2)
	{
		//May only have to do one.
		return c1 == c2 || std::toupper(c1) == std::toupper(c2);
	}

	static bool StrCmpI(std::string str1, std::string str2)
	{
	return str1.size() == str2.size() &&
		std::equal(str1.begin(), str1.end(), str2.begin(), CharCmpI);
	}
};

