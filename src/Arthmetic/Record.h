#pragma once

namespace Arthmetic
{

	//for each class that would need to be created I'm going to need a different type.
	enum struct DataType : uint32_t
	{
		Invalid,
		Pass,		//Used to prevent doubling up on last entries.
		Operator,
		Number,
		String,
		Object,  //By rule all targets and objects are iterchangable.
		Target,  //However, target is a specification that follows the function name to specify the type it should do it on.
		FormulaCall,
		ParameterCall,//slash property, but parameters take priority

		//Extract flags: I'll likely make a manual function for this, but basically these flags should be extracted
		SubArthmetic = 1 << 14,  //Doesn't exactly have to be a subroutine
		LastEntry = 1 << 15,    //Flag makes object act as the last entry of whatever is being processed.
		//DeclareParam = 1 << 16,	//Used to indicate that a record. Don't think I can use.
		AllFlags = SubArthmetic | LastEntry// | DeclareParam
	};

	//I'm going to need a new object for targeting. The idea being that you can try to convert it into a different type either using a dynamic cast
	// Or a designated cast at the users behest (So I can use skyrim casts). The idea being, if the cast results in a null value, then it will
	// throw an exception that exists the function used and send a null value.


	inline bool IsStandardValue(DataType type)
	{
		//This is actually kinda useless, all delegates are IReadyArthmetics, so you can literally just use that even if it's a function call.

		switch (type)
		{
		case DataType::FormulaCall://Currently, this is the only thing I can think of.
		//case DataType::ParameterCall://This will need to be resolved in the moment, since the name can possibly not actually be a property.
			return false;
		default:
			return true;
		}
	}





	struct RecordData
	{
		std::string_view view{};
		DataType type{ DataType::Invalid };

		//I want a constant expression of this that can effectively just encapsulate an operator or common function or somehing.
		// Like numbers next to a parantheses can't be interpreted like that very easy, but if there was a string view already existing of
		// multiplication, that makes it much simpler.

		DataType GetType() const { return type & ~DataType::AllFlags; }

		DataType GetFlags() const { return type & DataType::AllFlags; }

		constexpr RecordData() = default;
		

		constexpr RecordData(std::string_view new_view, DataType data_type) :
			view(new_view), type(data_type) {}

		constexpr RecordData(StringIterator begin, StringIterator end, DataType data_type) :
			view(begin, end), type(data_type) {}
	};

	using RecordIterator = std::vector<RecordData>::const_iterator;


	struct Record
	{


		std::unique_ptr<const char[]> dataPtr{ nullptr };
		
		//Somewhere in here, where raw data begins, the name is finished. I'd like that too in a string view, easy to see. Or perhaps
		// it will be easy to see. Full data will start right after "::", so all I have to do is get fullData.begin, and dataPtr as begin
		// and I can make the string view of what I need.
		//Nvm, just store the length and create a string view that starts from pointer.

		std::string_view fullData;
		std::vector<RecordData> dataView;


		//This needs something to contain external identifiers with it's records. IE, so it can know what some strings mean.

		constexpr RecordIterator begin() { return dataView.begin(); }
		constexpr RecordIterator end() { return dataView.end(); }

		constexpr RecordIterator begin() const { return dataView.begin(); }
		constexpr RecordIterator end() const { return dataView.end(); }

		//Record(std::string data) :
		//	fullData(data) {}

		std::string_view GetScopeName() { return "None"; }

		Record(std::string data) :
			dataPtr(strdup(data.data())),
			fullData(dataPtr.get(), data.length()) {}



		Record(const char* data, size_t length) :
			dataPtr(strdup(data)),
			fullData(dataPtr.get(), length) {}

	};

	//I throw this instead of returning an empty array now, the exception should be caught. Within it should have the pointer
	// to the last data being processed.
	struct RecordException
	{};

}