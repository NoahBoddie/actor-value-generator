#pragma once

namespace Arthmetic
{
	enum struct ArgumentType : uint8_t
	{
		NullArg,
		String,
		Number,
		Object,
		Arthmetic,//If it's a subroutine, arthmetic. If it's a delegate, arthmetic. 
		
		//Parameter still needs to designate a type. Then again, I guess that would be based on what that parameter is.
		
		//I JUST REALIZED, I can make all the flags, save any, represent resolved. Then, I can have a resolvable flag for
		// parameters. The idea is if something is resolved, then it must be a static value, not a parameter or property.

		//Actually, instead of resolvable, I can use incomplete to designate when parameters aren't default, and when lacking
		// they are filled. Main reason I want this is so I can use properties, who are now very much static objects now.


		Any = 1 << 3,			//When asked for a type, any will report true regardless, while clearing abiguity in parameters that accept all types.
								//The state of any controls if just any type is allowed into the argument. Without it, an exception will be thrown.
		Property = 1 << 4,
								
		Incomplete = 1 << 5,	//Marks something as "incomplete". In arthmetic's case, this is used to show that it's Not managing.
								//IDEA, simply having something unlinked means that whatever it is needs local, global, or object linkage.
								//That way, it's easy to see what value it is. AND, incomplete can be used as a flag for other types.
		Parameter = 1 << 6,		//If it instead points to a local parameter
		Resolved = 1 << 7,
		
		SoftFlags = Incomplete | Resolved,						//Flags that don't change how the data is returned (ie a function or something).
		HarmlessFlags = Resolved | Any, //These flags don't impact what the property actually holds
		AllFlags = Incomplete | Parameter | Resolved | Property,
		RawFlags = AllFlags | Any ,//Raw flags include any, which isn't something that's treated like a flag.
	
		//Compound enumerations
		AnyString = String | Any,
		AnyNumber = Number | Any,
		AnyObject = Object | Any,
		UnlinkedObject = Object | Incomplete
	};

	//Evaluate the relevance of this.
	inline bool IsStandardValue(ArgumentType type)
	{
		type &= ~ArgumentType::SoftFlags;

		switch (type)
		{
		case ArgumentType::String:
		case ArgumentType::Number:
		case ArgumentType::Object:
			//Any values should count.
			return true;
		default:
			return false;
		}
	}
	
	//Put in detail or something.
	[[nodiscard]] inline constexpr std::string_view argT_to_strV(ArgumentType arg_type)
	{
		arg_type &= ~ArgumentType::RawFlags;

		switch (arg_type)
		{
		default:
			return "NullArgument";


		case ArgumentType::String:
			return "String";

		case ArgumentType::Number:
			return "Number";

		case ArgumentType::Object:
			return "Object";

		case ArgumentType::Arthmetic:
			return "ArithmeticNumber";
		}
	}

}

//using namespace Arthmetic;


//*
namespace std
{
	[[nodiscard]] inline constexpr std::string to_string(Arthmetic::ArgumentType arg_type)
	{
		return Arthmetic::argT_to_strV(arg_type).data();
	}

#ifdef __cpp_lib_format
	template <class CharT>
	struct formatter<Arthmetic::ArgumentType, CharT> : formatter<std::string_view, CharT>
	{
		template <class FormatContext>
		auto format(Arthmetic::ArgumentType arg_type, FormatContext& a_ctx)
		{
			return formatter<std::string_view, CharT>::format(Arthmetic::argT_to_strV(arg_type), a_ctx);
		}
	};
#endif
}

namespace fmt
{
	template <>
	struct formatter<Arthmetic::ArgumentType>
	{
		template <class ParseContext>
		constexpr auto parse(ParseContext& a_ctx)
		{
			return a_ctx.begin();
		}

		template <class FormatContext>
		auto format(const Arthmetic::ArgumentType& arg_type, FormatContext& a_ctx)
		{
			return fmt::format_to(a_ctx.out(), "{}", Arthmetic::argT_to_strV(arg_type));
		}
	};
}
//*/