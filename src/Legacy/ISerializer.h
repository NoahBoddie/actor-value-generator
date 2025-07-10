#pragma once

namespace RGL
{
	//Another empty general class to display a classes desire to perform special actions when serializing
	class ISerializer {};

	//I could use 'std::is_polymorphic' to tell if something I'm serializing something with a vtable. This could be
	// something to catch
	
	//Additionally, I could use a concept to search for the function expected from ISerializer. I made it this way
	// so it doesn't HAVE to use virtual functions, but it still won't serialize it's whole size

}
