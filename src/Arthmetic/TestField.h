


//this is partly a creation function, so it's responsible as well.

int main()
{
	//Setting up the function on map
	functionMap["GetRandomRange"] = GetRandomRange;

	{
		//Setting up routine on map
		// ... modCheck ?
		//gonna set up true and false
		std::vector<std::string> pre_routine_true = { "1#" };
		std::vector<std::string> pre_routine_false = { "0#" };
		//Other constants would be pi very likely.
		//Stuff like this, likely shouldn't be a routine, takes up a decent chunk of space for something
		// that's not constant.
		uint16_t i = 0;

		vector<string>::const_iterator true_it = pre_routine_true.begin();
		vector<string>::const_iterator true_false = pre_routine_false.begin();

		vector<ArthmeticRoutineItem*> true_item = { TestMakeFunction(i, true_it) };
		vector<ArthmeticRoutineItem*> false_item = { TestMakeFunction(i, true_false) };

		ArthmeticRoutine* routine_true = new ArthmeticRoutine(true_item);
		ArthmeticRoutine* routine_false = new ArthmeticRoutine(false_item);

		routineMap["true"] = routine_true;
		routineMap["false"] = routine_false;
	}

	//Currentl this has an issue with it's order of operations. * and / being on the same
	// level. I also think I may want to handle division in short and long devision,
	// long devision being translated like /() and looking like this :/
	string function = "true - 22. - .2 + 2. + (3 * 4) / GetRandomRange(1, 5, 1) * (5 % 6) ^ 7";
	//string function = "1 - 22. - .2 + 2. + (((3 * 4) / 4) * ((5 % 6) ^ 7))";

	//string function = "(5) + GetRandomRange(1, 88)";
	//string function = "6 / 2 * (1 +2)";
	vector<string> broken_down_func = InitialParse(function);

	bool first = true;

	vector<ArthmeticRoutineItem*> function_code(broken_down_func.size());

	uint16_t i = 0;

	vector<string>::const_iterator it = broken_down_func.begin();
	vector<string>::const_iterator end = broken_down_func.end();

	while (it != end) {
		string to_print = *it;

		if (!first)
			cout << ", ";
		else
			cout << "[";

		first = false;

		cout << to_print << "(" << i << ")";

		ArthmeticRoutineItem* new_code = TestMakeFunction(i, it);

		function_code[i] = new_code;

		it++;
		i++;
	}

	function_code.resize(i);

	cout << "]" << std::endl
		 << std::endl;

	ArthmeticRoutine routine(function_code);

	ExtraValueInfo* info = new ExtraValueInfo(false);

	info->updateCalc = &routine;

	std::string ev_name = "TestValue";

	//brute forcing it a bit, but who cares for now.
	ExtraValueInfo::_infoTable[ev_name] = info;
	ExtraValueInfo::FinishManifest();

	RE::Actor* actor = new RE::Actor();

	cout << "Make Storage" << std::endl;

	_valueTable[actor->formID] = new ExtraValueStorage(actor, false);

	float test_value = GetActorValue(actor, ev_name);

	cout << std::endl
		 << std::endl;

	//ArthmeticArgument arg(function_code);

	//float test_value = arg.ProcessFunction();//0;//
	//float test_value = routine.Run();

	//string test = "-2.444#";
	//double value = std::stod(test);
	//float test_2 = -19.2 + 0.0001536;
	//cout
	cout << "Hello World: " << test_value;

	return 0;
}

float GetRandomRange(RE::Actor* target, const std::vector<ArthmeticFunctionParameter>& parameter)
{
	float min = parameter[0].GetNumber();
	float max = parameter[1].GetNumber();

	bool true_random = parameter[2].GetNumber();

	if (true_random) {
		std::srand(std::time(NULL));
	} else {
		std::srand(1);
	}

	//Flawed implementation, do not care.
	int range = max - min + 1;
	int num = rand() % range + min;

	cout << std::endl
		 << "  Special Function: min = " << min << ", max = " << max << ", (" << true_random << "), result is num " << num << ";";

	return num;
};

//functionMap["GetRandomRange"] = GetRandomRange;


std::map<HighHash, FunctionCallback*> functionMap;
std::map<HighHash, Routine*> routineMap;
std::map<HighHash, MethodCallback*> methodMap;

//Methods will derive from ArthmeticObjects, but they will not give a return value basically, just send something to a callback.
// will handle that later.


//When I'm chopping up an interpreted string, it should be stored in a new class.
// This class should store the primary string, and have a vector of subclasses,
// this subclass stores a string_view, and a type. This frees up the string method we were
// using before, it's faster because of how string views work, and reduces need for allocation
// and deallocation. Not to mention, it will likely result in cleaner code.


//for each class that would need to be created I'm going to need a different type.
enum DataType : uint16_t
{
	Invalid, 
	OpenOperator,
	SingleOperator,
	DualOperator,
	Number, 
	String, 
	Object,//By rule all targets and objects are iterchangable.
	Target,//However, target is a specification that follows the function name to specify the type it should do it on.
	RoutineCall,
	FunctionCall,

	//Extract flags: I'll likely make a manual function for this, but basically these flags should be extracted
	Subroutine = 1 << 14,//Until the last entry is found all record data will be included in the subroutine
	LastEntry = 1 << 15//Flag makes object act as the last entry of whatever is being processed.
};

//I'm going to need a new object for targeting. The idea being that you can try to convert it into a different type either using a dynamic cast
// Or a designated cast at the users behest (So I can use skyrim casts). The idea being, if the cast results in a null value, then it will
// throw an exception that exists the function used and send a null value.


struct RecordData
{
	std::string_view view{};
	DataType type{ DataType::Invalid };

	//I want a constant expression of this that can effectively just encapsulate an operator or common function or somehing. 
	// Like numbers next to a parantheses can't be interpreted like that very easy, but if there was a string view already existing of
	// multiplication, that makes it much simpler.



	constexpr RecordData() = default;

	constexpr RecordData(StringIterator begin, StringIterator end, DataType data_type) :
		view(begin, end), type(data_type) {}

};

struct Record
{
	std::string fullData;

	std::vector<RecordData> dataView;

	Record(std::string data) : fullData(data) {}
};

//I throw this instead of returning an empty array now, the exception should be caught. Within it should have the pointer
// to the last data being processed.
struct RecordException {};


using StringIterator = std::string::const_iterator;

//Declarative Region
Record IteratorParse(StringIterator& it, StringIterator end, bool full_string);

RecordData ParseNumber(StringIterator& it, StringIterator begin, StringIterator end, bool has_point = false, DataType extra_flags = DataType::Invalid)
{
	while (it != end) {
		if (!has_point && *it == '.') {
			has_point = true;
			goto inc;
		} else if (std::isdigit(*it) == true) {
			goto inc;
		} else {
			break;
		}
	
inc:
		++it;
	}

	return RecordData(begin, it, DataType::Number | extra_flags);
}

inline RecordData ParseNumber(StringIterator& it, StringIterator end, bool has_point = false, DataType extra_flags = DataType::Invalid)
{
	return ParseNumber(it, it, end, has_point, extra_flags);
}



void ParseArguments(StringIterator& it, StringIterator end, std::vector<RecordData>& out_vector)
{
	//cout << "HIT 1";

	//for now, encountering multiple paras should be an X.
	// I may be able to do arethmatic in these, but one thing at a time.

	//std::vector<std::string> return_strings;

	StringIterator new_begin = it;

	//This is how many "(" we've passed. Once it's zero, we can exit the function, giving the last command ~ instead of #
	int params_open = 1;  //Make not need, if we encounter

	bool quotation_ignoring = false;

	char last_char = '\0';

	//The moment space is detected, that's the end of the function
	//Second, we base this around the first character. If it's a letter, do ParseName.
	// If it's a number we parse number.
	// If the first is t or f we try to compare to t or f, exit out otherwise.

	//bool param_location = 1;//locations are 1 for first, "," cannot be here,
	//bool first_char = true;

	//Later I want to be able to parse this differently
	while (it != end) {
		//Require this to close please
		if (quotation_ignoring) {
			if (*it == '\"' && last_char != '\\') {  //if it was \" it is likely specificly a quote in the string. Not as if that would be allowed.
				quotation_ignoring = false;

				//cout << *new_begin << " / " << *new_end << std::endl;

				//std::string push_string = std::string(new_begin, it) + "$";
				//create and push the string.

				//return_strings.push_back(push_string);
				out_vector.push_back(RecordData(new_begin, it, DataType::String));
				;
			}
			goto inc;
		} else if (*it == '\"') {  //But not check happens here because if a quote hasn't been opened \" doesn't do anything.
			quotation_ignoring = true;
			new_begin = ++it;
			goto set_last;
		}

		switch (*it) {
		//These needs more control on comma placement, for now, I'll just be smart.
		case ',':
			{
				//When you encounter this, send the function off. Make a string, then break it down.
				//string tmp_section = std::string(new_begin, it);
				//This works with arrays
				//a.insert(std::end(a), std::begin(b), std::end(b));

				////cout << std::endl << *new_begin << " / " << *it << std::endl;

				auto result = IteratorParse(new_begin, it, false);

				it = new_begin;
				new_begin++;

				////cout << std::endl << " end at " << *new_begin << std::endl;

				//Size should be allowed to be zero right?
				if (result.size() > 0)
					//return_strings.insert(return_strings.end(), result.begin(), result.end());
					out_vector.insert(out_vector.end(), result.begin(), result.end());

				break;
			}
		case '(':
			params_open++;
			break;
		case ')':
			//I want to combine these, but I'm sorta not sure how

			params_open--;

			if (params_open <= 0) {
				//Might prevent this from running if I read there's nothing relevant. I'll have something do some detection
				// since the last comma
				auto result = IteratorParse(new_begin, it, false);
				//return_strings.insert(return_strings.end(), result.begin(), result.end());
				out_vector.insert(out_vector.end(), result.begin(), result.end());

				goto end;
			}
		}

inc:
		++it;

set_last:
		last_char = *it;
	}

end:

	RecordData& end_param = out_vector.back();
	end_param.type |= DataType::LastEntry;

	//return return_strings;
}

std::vector<RecordData> ParseName(StringIterator& it, StringIterator begin, StringIterator end)  //Used to have allow_nat
{
	//So it's quite important that this branch out, it's currently parse name, but it's actually parse function, specifically function, 
	// not routine.


	std::vector<RecordData> total_function{};

	bool nat_func = false;

	bool quotation_ignoring = false;
	char last_char = '\0';

	auto IsName = [](char character) -> int {
		
		//I want to put some more usable characters in here.
		//Usable charaters
		// :: they are required to be stacked up twice, but not thrice and never to show again
		// . this is supposed to be used only from a target

		return std::isalnum(character) || character == '_';
	};
	
	while (it != end) {
		/*NOTE, this will be moved
        if (quatation_ignoring){
            if (*it == '\"' && last_char != '\')//if it was \" it is likely specificly a quote in the string. Not as if that would be allowed.
                quatation_ignoring = false;
        }
        else if (*it == '\"'){//But not check happens here because if a quote hasn't been opened \" doesn't do anything.
                quatation_ignoring = false;
        }
        //*/

		//This needs to account for ::

		last_char = *it;

		if (*it == '(') {
			//Nat func detected, saving the name, and printing the data
			nat_func_detected = true;
			break;
		}
		else 
		{
			auto parsable = IsName(*it);

			//We want both numbers and letters, since we now know the first is a chacater
			if (parsable == false) {
				break;  //if its not ( or {, don't we want to fail this shit then?
			}
			else if(parseable == -1) 
			{
				//Illegal value used. Should be 
				logger::info("R");
				throw nullptr;			
			}
		}
		

		//inc:
		++it;
	}
	//HERE, depending on what it is this function 'it' currently is, it should fire a different function, and commit the main name.
	
	//If it is a period, we would probably find some way to do the above again.

	//std::string main_name = std::string(new_begin, it);
	RecordData main_name(begin, it, nat_func ? DataType::FunctionCall : DataType::RoutineCall);//For now, just function.


	total_function.push_back(main_name);

	//This is supposed to deal with a multitude of things, one of the main things is supposed to be the selection of a target.
	// But specifying a target with '.' and using a namespace would be very difficule, so I'm gonna ignore that for a while.
	// It also doesn't preserve the zero index very well.
	/*
	switch (last_char) 
	{
	case '(':
	case '{':
	case ' ':
	}
	//*/


	if (nat_func) {
		//[0] += "&";

		++it;  //Make sure to push past "(" to start

		//do function here.
		//cout << *it;
		//std::vector<std::string> result = ParseArguments(it, end);
		//total_function.insert(total_function.end(), result.begin(), result.end());
		//This is kinda scuffed looking ngl.

		ParseArguments(it, end, total_function);
	}

	return total_function;
}

inline std::vector<RecordData> ParseName(StringIterator& it, StringIterator end)
{
	return ParseName(it, it, end);
}

std::vector<RecordData> IteratorParse(StringIterator& it, StringIterator end, bool full_string)
{
	//It's likely that I'll need to store an iterator at the start of the function.



	std::vector<RecordData> return_vector{};

	bool space_detected = false;

	int operators_open = 0;

	RoutineItemType last_type = RoutineItemType::None;

	while (end != it) {
		auto true_it = it;
		
		bool pushed = false;

		RecordData push_value;

		bool hit_spacing = false;

		char load_char = ArthmeticUtility::CheckAlpha(it);

		DataType use_type = DataType::Invalid;

		switch (load_char) {
		case ' ':
		case '\n':
			hit_spacing = true;
			//We ignore spaces, maybe page breaks too
			break;
		case '\0':
			//Handles character processing, uses null char since that should never be encountered
			if (ArthmeticUtility::IsValidCodeChar(RoutineItemType::FunctionValue, last_type) == false) {
				//cout << "ERROR, \"FunctionValue\" item cannot be next to " << PrintRoutineItemType(last_type) << ". >> " << *it;
				return std::vector<std::string>{};
			} else {
				auto result = ParseName(it, end);

				return_vector.insert(return_vector.end(), result.begin(), result.end());
				//This doesn't push, don't push
			}
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			//Handle digit
			if (ArthmeticUtility::IsValidCodeChar(RoutineItemType::Value, last_type) == false) {
				//cout << "ERROR, \"Value\" item cannot be next to " << PrintRoutineItemType(last_type) << ".";
				//return std::vector<std::string>{};
				logger::info("S");
				throw nullptr;
			}

			//push_value = ParseNumber(it, end) + "#";
			push_value = ParseNumber(it, end);
			pushed = true;
			break;

		case '.':
			//Handle possible decimal number, else present error

			if (!space_detected) {
				//cout << "ERROR, invalid decimal found without proper spacing.";
				//return std::vector<std::string>{}
				logger::info("T");
				throw nullptr;
			}

decimal_handle:

			//push_value += '.';
			pushed = true;
			++it;//We push because numbers shouldn't re read the decimal.

			if (std::isdigit(*it) == true) {
				if (ArthmeticUtility::IsValidCodeChar(RoutineItemType::Value, last_type) == false) {
					//cout << "ERROR, \"Value\" item cannot be next to " << ArthmeticUtility::PrintRoutineItemType(last_type) << ".";
					return std::vector<std::string>{};
				}

				//push_value += ParseNumber(it, true_it, end, true) + "#";
				push_value = ParseNumber(it, true_it, end, true);
			} else {
				//cout << "ERROR, invalid decimal with no number found.";
				//return std::vector<std::string>{};
				logger::info("U");
				throw nullptr;
			}
			break;

		case '-':
			//Handle possible negative
			//push_value = '-';
			pushed = true;
			++it;//We push because number proess cant read the dash.

			//If it's a decimal go to decimal handle.
			if (std::isdigit(*it) == true) {
				if (ArthmeticUtility::IsValidCodeChar(RoutineItemType::Value, last_type) == false) {
					//cout << "ERROR, \"Value\" item cannot be next to " << ArthmeticUtility::PrintRoutineItemType(last_type) << ".";
					//return std::vector<std::string>{};
					logger::info("V");
					throw nullptr;
				}

				//push_value += ParseNumber(it, true_it, end, false) + "#";
				push_value = ParseNumber(it, true_it, end, false);
			} else if (*it == '.') {
				goto decimal_handle;
			} else if (ArthmeticUtility::IsValidCodeChar(RoutineItemType::Operator, last_type) == false) {
				//cout << "ERROR, \"Operator\" item cannot be next to " << ArthmeticUtility::PrintRoutineItemType(last_type) << ".";
				//return std::vector<std::string>{};
				logger::info("W");
				throw nullptr;
			}

			break;

		//Currently, these have no rules as to what they can go up against.
		case '(':
			//NOTICE
			//Currently this always goes through, but if it's next to a close para or a value, it should multiply
			operators_open++;
			last_type = RoutineItemType::OpOpen;
			use_type = DataType::OpenOperator;
			goto symbol_handle;

		case ')':
			if (ArthmeticUtility::IsValidCodeChar(RoutineItemType::OpClose, last_type) == false) {
				//cout << "ERROR, \"Operator\" item \"" << *it << "\" cannot be next to " << ArthmeticUtility::PrintRoutineItemType(last_type) << ".";
				//return std::vector<std::string>{};
				logger::info("X");
				throw nullptr;
			} else if (operators_open <= 0) {
				//cout << "ERROR, An operator was closed with no open symbol.";
				//return std::vector<std::string>{};
				logger::info("Y");
				throw nullptr;
			}
			operators_open--;
			use_type = DataType::OpenOperator;
			goto symbol_handle;

		case '+':
		case '/':
		case '*':
		case '%':
		case '^':
			if (ArthmeticUtility::IsValidCodeChar(RoutineItemType::Operator, last_type) == false) {
				//cout << "ERROR, \"Operator\" item \"" << *it << "\" cannot be next to " << ArthmeticUtility::PrintRoutineItemType(last_type) << ".";
				//return std::vector<std::string>{};
				logger::info("Z");
				throw nullptr;
			}
			use_type = DataType::DualOperator;


//Symbol handle, simply pushes symbol
symbol_handle:
			//push_value = *it;
			pushed = true;
			push_value = RecordData(it, it++, use_type);
			break;
		case '{':
			//Functional handle, Not implemented, merely ignore.

		default:
			//Input is invalid, do not use. Also pull the value that offends.
			//cout << "ERROR, invalid value \"" << *it << "\" found.";
			//return std::vector<std::string>{};
			logger::info("1");
			throw nullptr;
		}

		//We do this after so that I can effectively clear space if anything else at all other than a space value is used,
		// but if space would function the same if it was in the spacing code.
		space_detected = hit_spacing;

		if (push_value != "")
			return_vector.push_back(push_value);
		//These 2 could be linked
		if (!pushed)
			++it;
	}

	if (operators_open != 0) {
		//cout << "ERROR, An operator was left open by a count of " << operators_open << ".";
		//return std::vector<std::string>{};
		logger::info("2");
		throw nullptr;
	} else if (full_string && last_type == RoutineItemType::Operator) {
		//cout << "ERROR, An operator standard operator cannot be the last item in the function.";
		//return std::vector<std::string>{};
		logger::info("3");
		throw nullptr;
	}

	return return_vector;
}

Record* InitialParse(const std::string& function_code)
{
	//Has to send back a pointer because of string views. Should test functionality with unique_ptrs when I get the chance.
	// Or find some functionality of a string like object that passes ownership.
	//Basically I need what unique pointer does in exchanging ownership, but something that has the conviences of string classes.
	Record* new_record = new Record(function_code);

	new_record->fullData.begin();

	StringIterator it = new_record->fullData.begin();//function_code.begin();
	StringIterator end = new_record->fullData.end();//function_code.end();
	
	new_record->dataView = IteratorParse(it, end, true);
	
	return new_record;
}

//Note, you can make more global properties, you'll have to declare them as a property however, which must consist of relatively constant values.


struct ParameterSetting
{
	ArgumentType type;
	std::string name;

	union
	{
		float defaultFloat;
		char* defaultString;
		ARTH_OBJECT_TYPE* defaultObject;
	};
};

template <class Type, class... ExtraDataArgs>
static void PushExtraData(Type* obj, ExtraDataArgs... args)
{

	//This has to be used manually, like
	// PushExtraData(this, ...);
	// So not automatic
	ExtraDataHandler* test_handle{};

	if (ExtraDataHandler::GetSingleton(handler) == false)
		return;

	//Push back...

	ExtraDataHandler& handler = *test_handle;

	using TypeExtraData = Type::ExtraData;

	auto& queue = handler[obj];

	if (queue.HasData<Type>() == true)
	{
		logger::error("ExtraData already exists.");
		return;
	}


	if constexpr (!std::is_same_v<Type, ArthmeticObject::ExtraData>)
	{
		//If you don't see the default extra data, add that first, as it's the key to validation.
		if (queue.HasData<ArthmeticObject::ExtraData>() == false)
		{
			ArthmeticObject* a_obj = obj;
			PushExtraData(a_obj);
		}
	}

	TypeExtraData* extra_data = new TypeExtraData(args...);

}


//Formulas need Parameters and Targets

//RoutineItems need an just an argument

//ReadyArthmetic just neeeds Target

//Properties require neither.

using ParameterSettingList = public std::vector<ParameterSetting>;

struct DelegateArgument;

enum ObjectFlags
{
	Invalid,
	Validated,
	ErrorFlag,//Used when an error is spoken to not print it over and over again. May make more.
};


struct ArthmeticObject
{
	ObjectFlags flags = ObjectFlags::Invalid;
	
	constexpr ObjectFlags IsValid() { !!(flags | ObjectFlags::Validated); }

	//I've decided the data flags would be very useful as well in the construction.

	virtual void LoadFromView(RecordIterator& data_view) = 0;

	void LoadFromViewImpl(RecordIterator& data_view)
	{
		LoadFromView(data_view);

		//FinalizeExtra data- When finalized, it's the last data that gets throw.
		// If an arth object specifies, it will come validated. Operators, const values both come valid. Basically, anything that pushes
		// extra data has to be validated, so it's still just delegates that need validation.
		//I think some directives will need validation.
	}
};



struct RoutineItem
{
	//Most of the things in arthmetic base will be getting moved to RoutineItem

	virtual void Run(RoutineArgument* argument, float& return_value, size_t& index) = 0;

	void RunImpl(RoutineArgument* argument, float& return_value, size_t& index)
	{
		if (IsValid() == false) {
			return_value = 0;
			return;
		}

		Run(argument, return_value, index);
	}
};
struct ARTH_TARGET_TYPE;


//A resolved parameter is created from 3 different things, the default value from the setting,
struct ResolvedParameter
{
	union
	{
		uint64_t _value{};
		char* _string;//When outputted, put into an std::string

		float _number;

		ARTH_OBJECT_TYPE* _object;
	};
	
	//Must be created from setting or delegate parameter
};



struct TargetList
{

	//This is what's used externally, because if I introduce new types I think the different versions would break shit.
	//I think these will consist of 2 different types.

	//The first would be the dynamic targets, then the static targets. Would make it easier to handle I think. and would prevent an index
	// error. Will work on that when targeting becomes more important.
	
	//Additionally, make a wrapper object for accessing the target and object types. This way, I can send my personal exception and bail the function

	std::unique_ptr<ARTH_TARGET_TYPE*[]> targets = nullptr;
	size_t length;

	TargetList(ArgTargetParams& target_params)
	{
		targets = decltype(targets)(new ARTH_TARGET_TYPE * [] {target_params});
		length = target_params.size();
	}
};



struct ParameterList
{

	//This is what's used externally, because if I introduce new types I think the different versions would break shit.
	//I think these will consist of 2 different types.

	//The first would be the dynamic targets, then the static targets. Would make it easier to handle I think. and would prevent an index
	// error. Will work on that when targeting becomes more important.

	const ResolvedParameter* params = nullptr;
	size_t length = 0;

	//Access should be arranged with getter functions, in case not enough has been fulfilled.
	// Also, delegate parameters are not what should be being used. a weak carrier object called ResolvedParameters
	// What should go in it are the fully resolved parameters of a function. Resolved parameters make function calls,
	// and resolve routines, reducing items down to just their values.

	//This is supposed to take 2 lists.
	ParameterList(std::vector<DelegateArgument>* delegate_params)
	{
		if (delegate_param)
		{
			targets = delegate_param->data();
			length = delegate_param->size();
		}
	}
};


struct IReadyArthmetic
{
	//Might make this a void version.
	virtual float Run(const ArgTargetParams& targets) = 0;

	float RunImpl(const ArgTargetParams& targets)
	{
		if (IsValid() == false) {
			return 0;
		}

		return Run(targets);
	}
};


struct IDirective : public ArthmeticObject
{
	//I formulas push no index, as they are self contained.


	std::unique_ptr<ParameterSettingList> _paramSettingList{ nullptr };

	//I've removed default value and the return value from run because that implementation can change between formulas.

	

	virtual void Run(const ArgTargetParams&, std::vector<DelegateArgument>*, float&) = 0;

	virtual void RunImpl(const ArgTargetParams& targets, std::vector<DelegateArgument>* params, float& result)
	{

		if (IsValid() == false) {
			return_value = 0;
			return;
		}

		Run(targets, params, result);
	}
};

struct IFormula : public IDirective
{
	//If the function gets an error thrown by a missing parameter or something like that, this is the default value used.
	//Namely, if your choosen value is null
	//I think if the result is NAN it will also use this as well.
	float defaultValue = 0;
	


	inline float Run(const ArgTargetParams& target_params, std::vector<DelegateArgument>* formula_params)
	{
		//Not really needed, but symbolizes a change in function.
		float result = 0;

		RunImpl(target_params, formula_params, result);

		return result;
	}
};

using FunctionCallback = float(TargetList, ParameterList);



struct FunctionInterface : public IFormula
{
	FunctionCallback* _callback = nullptr;

	void Run(const ArgTargetParams& targets, std::vector<DelegateArgument>* parameters, float& result) override
	{
		if (!_callback) {
			//Log error, do return value
			result = defaultValue;
		}

		TargetList t_list(targets);
		ParameterList p_list(&parameters);

		try
		{
			result = _callback(t_list, p_list);
		}
		catch (nullptr)
		{//Temporary, but this is the exception that helps an easy bail.
			result = defaultValue;
		}
	}

};



struct IRoutine : public IFormula
{
	//This is supposed to hold code.
	std::vector<RoutineItem*> _code{};
};

struct Coroutine : public IRoutine
{
	//Made with parameters in mind.
};



struct Subroutine : public IRoutine, public IReadyArthmetic
{
	//Made with no parameters in mind.
};


//May later centralize this from a base, but unsure of when I'll do such a thing.
struct FormulaDelegate : public RoutineItem, public IReadyArthmetic
{
	//This would probably just be a centralizer, likely will do nothing.
	
	float defaultValue = NAN;

	//The main point of this is you don't have to fulfill all the items.
	std::unordered_map<size_t, DelegateArgument>* params = nullptr;//Something like this


	IFormula* _formula;
	//Would no longer need this.
	//virtual DelegateType DelegateType() { return DelegateType::None; }

	//constexpr IDelegate(uint16_t pos) :
	//	RoutineItem(pos, RoutineItemType::FunctionValue) {}
};


//Similar to a routine argument, when parameters are sent for this I'd like an object to handle the arguments. Basically you'll be requesting
// parameters from this, and if there are no parameters in the submitted ones it wil check the default



template <class ClassType, enum EnumType> //requires (std::is_enum_v<EnumType>)
class ClassFactory
{
	//This will have to have some light implementation from the class routineitems (and other classes) must derive from.
	using RegisterFunction = ClassType*(*)();

	inline static std::unordered_map<EnumType, RegisterFunction> creationFactory{};

	ClassFactory() = default;

	//This should be private, wouldn't want someone thinking they had to inherit this.
	ClassFactory(EnumType enum_type, RegisterFunction registration)
	{
		creationFactory[enum_type] = registration;
	}
};

template <class ClassType, enum EnumType, class RelativeType, EnumType Value>//for relative type std::derived_from<ClassType>
class IndirectFactoryComponent : private ClassFactory<ClassType, EnumType>
{
private:
	static ClassFactory<ClassType, EnumType> _initializer = ClassFactory(Value, Create);

	ClassType* Create()
	{
		RelativeType* result = new RelativeType();
		return result;
	}
};
//Make a shorter version, template wise for specific use cases.
template <std::derived_from<RoutineItem> ItemType, RoutineItemType Value>
using  RoutineItemFactoryComponent = IndirectFactoryComponent<RoutineItem, RoutineItemType, ItemType, Value>;



enum struct TestEnum
{
	kBase, 
	kTarget
};

using Target = void;			//Would be form
using TargetType = TestEnum;	//Would be form type. Used to have something require a specific form type to proceed. (Can even make a function for?
using TargetContext = void;		//Would be extra data


struct UniqueTarget
{
	//Use this instead of target, fuctional selectors could be used to get specific targets, like worn objects or most significant
	// healthed extradata or whatever. IE, treat this like a pointer.
	Target* focus;
	TargetContext* context;//Might give this a unique class specifically too to hold onto more extra data.

	UniqueTarget() = default;
	UniqueTarget(Target* _f) : focus(_f) {}
	UniqueTarget(Target* _f, TargetContext* _c) : focus(_f) {}

	//Should effectively be an alias for Target*
	operator Target*() { return focus; }

	Target* operator->() const
	{
		return focus;
	}
};

struct TargetSelector
{
	
	
	//Should have some way to declare this undeclared.
	//Basically, this is supposed to be determined via declaration like this rather than an override.
	TargetType GetType();

	virtual Target* GetTarget(TargetSelector*) = 0;
	
	
	Target* GetTargetSafe(TargetSelector* selector)
	{
		//This version of get target, while a member function doesn't actually require the member to exist. And if the member does not
		// it will just return the target from the target used.

		if (!this){
			return selector ? selector->GetTarget(nullptr) : nullptr;
		}

		return GetTarget(selector);
	}

	//Not always will there be context, and if there's some way to get context that could be good.
	// Main point however is that items can be sent with extra information.
	virtual TargetContext* GetContext() { return nullptr; }
	


	//THIS, replaces a the object type in Object
};


TargetType TargetSelector::GetType()
{
	return TargetType::kTarget;
}

class BasicSelector : TargetSelector
{
	//This is the basic version of the selector, most things can use this, but not all things would have to.
	// RoutineArgument is primarily what is going to be handling this.
	UniqueTarget focus;

	Target* GetTarget(TargetSelector* selector = nullptr) override { return focus; }
};

//Under something like the above, youd have something like KeywordSelectors, that would just be manually created and managed 
// constant targets.


//Not actually needed, by itself has no reason to exist.
class SelectSelector : public TargetSelector
{
	TargetSelector* _selector;

	Target* GetTarget(TargetSelector* selector) override { return _selector->GetTarget(); }
};


//Unsure what should be used yet.
using SelectFunction = Target*(*)(Target*);

class FunctionalSelector : public TargetSelector
{
	TargetSelector* _select;
	SelectFunction _func;

	Target* GetTarget(TargetSelector* selector) override 
	{ 
		//How this works is it resolves the thing above it, and then performs a function on that.
		// new functions should be able to be added via mod api.

		Target* result = _select->GetTargetSafe(selector);

		return _func(result);
	}
};




class ParameterSelector : public TargetSelector
{
	//While not all selectors would need to, routine args would undoubtably need to have something loaded into their selector in order to function.
	// Perhaps all selectors take another selector, making proper space for routine argument without needing one.
	//Alright, so here is how this is gonna work. It sends a regular old TargetSelector with each one. It can't be IN it all the time so you know.
	// So what gets sent will still be the routine argument, which is what is currently the target. BUT, for a parameter selector it will
	// transfer it into being a routineArgument. This means I don't need to use it always, but it can be used for specific types.

	std::int32_t parameterIndex = -1;

	//Target* GetTarget() override { return selector->GetTarget(); }



};

enum class PropertyState
{
	Undefined,
	Parameter,
	Property
};


struct PropertyValue : public RoutineItem//public RoutineItemFactoryComponent<ArthmeticValue, RoutineItemType::Value>
{
	PropertyState GetPropertyState()
	{
		if (IsValid() == false)
			return PropertyState::Undefined;
		else if (_raw & address_bytes)
			return PropertyState::Property;
		else
			return PropertyState::Parameter;
	}

	union
	{
		uint64_t _raw = 0;

		mutable char* _propertyName;
		int32_t _propertyIndex;
		IReadyArthmetic* _propertyObject;
	};

	static constexpr uint64_t address_bytes = 0xFFFFFFFF00000000;

	int32_t GetPropertyIndex()
	{
		if (GetPropertyState() != PropertyState::Parameter)
			return -1;

		return _propertyIndex;
	}


	IReadyArthmetic* GetPropertyObject()
	{
		if (GetPropertyState() != PropertyState::Property)
			return nullptr;

		return _propertyObject;
	}

	//It says, RoutineArgument, but what it actually means is target selector that you can sift through.
	float GetPropertyValue(RoutineArgument* argument)
	{
		switch (GetPropertyState())
		{
			case PropertyState::Parameter:
				return !argument || !argument->_args ? 0 : (*argument->_args)[_propertyIndex].GetNumberParam(argument);

			case PropertyState::Property:
				return _propertyObject->RunImpl(argument->GetTargets());

			case PropertyState::Undefined:
				logger::error("undefined property state");
				break;

			default:
				logger::error("Unknown property state")
		}

		return 0;
		
	}

	void Run(RoutineArgument* argument, float& return_value, size_t& index) override 
	{
		return_value = GetPropertyValue(argument);
	}


	void LoadFromView(RecordIterator& data) override
	{

	}

	LinkerFlags GetLinkerFlags() override { return LinkerFlags::Property; }


	RoutineItemType GetItemType() override { return RoutineItemType::Value; }


	void FreeData() const
	{
		if (IsValid() == true || !_propertyName)
			return;

		logger::debug("freeing property '{}'", _propertyName);

		free(_propertyName);
	}


	~PropertyValue() { FreeData(); }
};




struct FakeArthObj
{
	//Point of this is after the main stuff does it's load, it will push the iterator, and check if it's something this cares 
	// to know about. if not, does nothing.
	virtual void OnPostLoadFromView(RecordIterator& it) {}
};

//The base object and the one that carries the target selector, handling it's creation.
// Not required to use target selectors so be wary of that. Merely will check for Target creation after the fact.
struct SelectorObject : public FakeArthObj
{
	void OnPostLoadFromView(RecordIterator& it) override
	{
		it++;

		while (it->IsTargeter)
			MakeTargetSelector(it);
	}
};








namespace std
{
	template<class T>
	struct vector;


	template<class T, size_t Size>
	struct array;

	class string;
}

struct RecordData;

//ENUM(ParsingType)
enum ParsingType
{
	
	Name,
	Number,
	Symbol,
	Pass,
	Total = Pass
};

struct Parser;


std::vector<std::string> reserved_names{};
struct StringItCollection
{
	StringIterator& it;
	StringIterator begin;
	StringIterator end;

	StringItCollection(StringIterator& a_it, StringIterator a_begin, StringIterator a_end) :
		it(a_it),
		begin(a_begin),
		end(a_end)
	{}
};

class ParsingContext
{
	//This is part of an idea where it holds the type and the later object must specify the type in order to use it. Will avoid if I can.
	//Type id = TypeInfo<void>;
	
	//ParsingContext() = delete;
	
	//I would like to determine a priority with these, With the default parser falling last regardless
	// of any other.

	virtual bool ShouldInterpret(Parser*, ParsingType, StringItCollection) = 0;
	virtual bool ShouldAllowInterpret(Parser*, ParsingContext*, ParsingType, StringItCollection) = 0;

	//there's an issue in this where loose records could be returned, or regular records could be returned.
	// The issue being ownership
	//So what I could do is no return types, instead I have an object that takes the new records to make. Instead of giving it new
	// records, I give it record components. This way, it has the ability when I upload it to make RecordData or LooseRecords
	//OR, I just use record data, and that record data will be turned into a series of loose Records if the situation calls for it.


	virtual std::vector<RecordData> InterpretName(Parser*, StringItCollection) = 0;
	virtual std::vector<RecordData> InterpretNumber(Parser*, StringItCollection) = 0;
	virtual std::vector<RecordData> InterpretSymbol(Parser*, StringItCollection) = 0;

	//Something used by the last context to determine the last character should be based.
	virtual void IsPassChar() = 0;

};


std::array<std::vector<ParsingContext*>, ParsingType::Total> ParsingMap;

struct DefaultContext
{
	//This handles much of the expected stuff
};

//Some things can only be a primary context, some things can





struct Parser
{
	ParsingContext* primaryContext;
	std::array<ParsingContext*, 2> previousContext;


	//This only accounts for the first character
	ParsingType GetParseType(StringIterator& it)
	{
		char load_char = ArthmeticUtility::CheckAlpha(it);

		StringIterator next = it + 1;

		switch (load_char)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '.':
			return ParsingType::Number;

		case '-':
		case '(':
		case ')':
		case '|':
		case '&':
		case '^':
		case '!':
		case '<':
		case '>':
		case '=':
		case '+':
		case '/':
		case '*':
		case '%':
		case '\'':
			return ParsingType::Symbol;

		case ':':
			if (*next != ':') {
				return ParsingType::Symbol;
			}

			++it;//Should skip the next one, we know what it is.
		case '_':
		case '\0':
			return ParsingType::Name;

		case ' ':
		case '\n':
			//I really would like to make a function to handle whether the following whitespaces will count.
			return ParsingType::Pass;
		default:
			ARTHMETIC_LOGGER(error, "symbol '{}' not expected.", *it);
			throw nullptr;
		}
	}

	
	std::vector<RecordData> ParseData(ParsingContext* current_context, std::pair< ParsingType, ParsingType> types, StringItCollection collection)
	{
		auto type = types.first;
		auto last_type = types.second;

		if (type == ParsingType::Pass)
			//do what?
			return std::vector<RecordData>();

		std::vector<ParsingContext*>& contexts = std::vector<ParsingContext*>[type];

		for (auto& context : contexts)
		{
			if (context->ShouldInterpret(this, type, collection) == false)
				continue;


			if (current_context->ShouldAllowInterpret(this, type, collection) == false) {
				//Print x isn't an allowed interpretation something rather other.
				continue;
			}

			switch (type)
			{
			case ParsingType::Name:
				return context->InterpretName(this, collection);
			case ParsingType::Symbol:
				return context->InterpretSymbol(this, collection);
			case ParsingType::Number:
				return context->InterpretNumber(this, collection);
			}

		}
		//Instead, this is to throw an error about how untenable this is
		return std::vector<RecordData>();
	}

	//This should use an std::conditional_t so I can return lose or non loose. depending on the call.
	inline std::vector<RecordData> ParseRange(ParsingContext* context, StringIterator& it, StringIterator end)
	{
		//It's likely that I'll need to store an iterator at the start of the function.

		std::vector<RecordData> return_vector{};

		bool space_detected = false;

		int operators_open = 0;//Should be handled by a specific context.

		RoutineItemType last_type = RoutineItemType::None;

		ParsingType last_type = ParsingType::Pass;

		StringIterator true_it = it;
		
		StringItCollection collect{ it, it, end };


		//One of these it's has to be stored so that it can be looped 
		static_assert(false, "CALLER");
		
		while (end != it) 
		{	
			//Can probably move on from using true_it with this
			//collect.begin = true_it;
			
			bool pushed = false;

			//RecordData push_value;

			bool hit_spacing = false;

			//Regarding GetParseType, I would like the existence of the processing of stuff like 
			// :: and . to only have one flag per interpret step to be a product of the primary context. 
			// may not be able to swing that however.
			ParsingType type = GetParseType(it);

			if (type == last_type) {
				//This should do some other stuff, but basically move on, keep cycling.
				//If the new type is pass I may have it do something else.
				
				goto push;
			}
			else if (last_type == ParsingType::Pass){
				collect.begin = it;
				goto push;
			}

			//We'll want to start parsing if the type has changed, and it the last type wasn't whitespace.


			//Should the thing actually be interpreted, the last_type should be set to pass, just to prevent some rogue error
			
			auto add = ParseData(context, std::make_pair(type, last_type), collect);

			////true_it = it;
			////collect.begin = it;
		
			//I think this should be done regardless.
			//if (add.size() != 0) {
				type = ParsingType::Pass;
			//}


		push:
			
			if (it == true_it)
			{
				it++;
				last_type = type;
			}
			else
			{
				//Unsure if I actually should do this or not.
				//last_type = GetParseType(it);
				last_type = type;
			}
			//Other things it should do if it has moved, last type should be reevaluated with get parse type.
			

			continue;
			//What determines if this actually fires the function below?
			// If the parsing type changes, it will process that.

			//The deal here will likely be we don't need a switch, instead the function takes the parsing name, and searches the associated list.
			// if it's pass it checks the validity of the whitespace
			//So what would it need? Parameter wise?
			//this, context, pair<ParsingType>, StringItCollection -> std::vector<RecordData>(rather,whichever type is used).
			//Not sure of what we use this for again, context will be for whitespace, StrIt is obvious, and the vector is the result
			switch (type)
			{
			case ParsingType::Name:
				//Psuedo:
				// Use both this and the current Context to call upon the ParsingContextMap, and try to parse something.
				// needs some way to get strings back, probably will use an exception of some sort.
			case ParsingType::Symbol:

			case ParsingType::Number:
			}
		}


		while (end != it) {
			auto true_it = it;

			bool pushed = false;

			RecordData push_value;

			bool hit_spacing = false;

			char load_char = ArthmeticUtility::CheckAlpha(it);

			//DataType use_type = DataType::Invalid;

			logger::info("{}", *it);


			//If you ever want this to recontextualize what happens with parameters, I could make an outside function that handles
			// some of the load by having reference functions. it could only be called if it's a nested call basically.
			switch (load_char) {
			case ' ':
			case '\n':
				hit_spacing = true;
				//We ignore spaces, maybe page breaks too
				break;
			case '\0':
				//Handles character processing, uses null char since that should never be encountered
				if (IsValidCodeChar(RoutineItemType::FunctionValue, last_type) == false) {
					//cout << "ERROR, \"FunctionValue\" item cannot be next to " << PrintRoutineItemType(last_type) << ". >> " << *it;
					//return std::vector<std::string>{};
					logger::info("B");
					throw nullptr;
				}
				else {
					auto result = ParseName(it, end);

					if (true_it != it)
						pushed = true;//Only do so if it moved, if it didn't, we're likely going to repeat a lot.

					return_vector.insert(return_vector.end(), result.begin(), result.end());
					//This doesn't push, don't push
				}
				break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				//Handle digit

				ARTHMETIC_LOGGER(info, "Handle digit ({}, \'{}\').", PrintRoutineItemType(last_type), load_char);

				if (IsValidCodeChar(RoutineItemType::Value, last_type) == false) {
					//cout << "ERROR, \"Value\" item cannot be next to " << PrintRoutineItemType(last_type) << ".";
					//return std::vector<std::string>{};
					ARTHMETIC_LOGGER(info, "ERROR, \"Value\" item cannot be next to {} (char \'{}\').", PrintRoutineItemType(last_type), load_char);
					throw nullptr;
				}

				//push_value = ParseNumber(it, end) + "#";
				push_value = ParseNumber(it, end);
				pushed = true;
				break;

			case '.':
				//Handle possible decimal number, else present error

				if (!space_detected) {
					//cout << "ERROR, invalid decimal found without proper spacing.";
					//return std::vector<std::string>{};
					logger::info("C2");
					throw nullptr;
				}

			decimal_handle:

				//push_value += '.';
				pushed = true;
				++it;  //We push because numbers shouldn't re read the decimal.

				if (std::isdigit(*it) != 0) {
					if (IsValidCodeChar(RoutineItemType::Value, last_type) == false) {
						//cout << "ERROR, \"Value\" item cannot be next to " << ArthmeticUtility::PrintRoutineItemType(last_type) << ".";
						//return std::vector<std::string>{};
						logger::info("D");
						throw nullptr;
					}

					//push_value += ParseNumber(it, true_it, end, true) + "#";
					push_value = ParseNumber(it, true_it, end, true);
				}
				else
				{
					//cout << "ERROR, invalid decimal with no number found.";
					//return std::vector<std::string>{};
					logger::info("E");
					throw nullptr;
				}
				break;

			case '-':
				//Handle possible negative
				//push_value = '-';
				pushed = true;
				++it;  //We push because number proess cant read the dash.

				//If it's a decimal go to decimal handle.
				if (std::isdigit(*it) != 0) {
					if (IsValidCodeChar(RoutineItemType::Value, last_type) == false) {
						//cout << "ERROR, \"Value\" item cannot be next to " << ArthmeticUtility::PrintRoutineItemType(last_type) << ".";
						//return std::vector<std::string>{};
						logger::info("F");
						throw nullptr;
					}

					//push_value += ParseNumber(it, true_it, end, false) + "#";
					push_value = ParseNumber(it, true_it, end, false);
				}
				else if (*it == '.') {
					goto decimal_handle;
				}
				else if (IsValidCodeChar(RoutineItemType::Operator, last_type) == false) {
					//cout << "ERROR, \"Operator\" item cannot be next to " << ArthmeticUtility::PrintRoutineItemType(last_type) << ".";
					//return std::vector<std::string>{};
					logger::info("G");
					throw nullptr;
				}
				else
				{

					//logger::info("last type is operator {}, {}/{}", last_type == RoutineItemType::Operator, *true_it, *it);
					push_value = RecordData(true_it, it, DataType::Operator);
				}


				break;

				//Currently, these have no rules as to what they can go up against.
			case '(':
				//NOTICE
				//Currently this always goes through, but if it's next to a close para or a value, it should multiply
				operators_open++;
				last_type = RoutineItemType::OpOpen;
				//use_type = DataType::Operator;
				goto symbol_handle;

			case ')':
				if (IsValidCodeChar(RoutineItemType::OpClose, last_type) == false) {
					//cout << "ERROR, \"Operator\" item \"" << *it << "\" cannot be next to " << ArthmeticUtility::PrintRoutineItemType(last_type) << ".";
					//return std::vector<std::string>{};
					logger::info("H");
					throw nullptr;
				}
				else if (operators_open <= 0) {
					//cout << "ERROR, An operator was closed with no open symbol.";
					//return std::vector<std::string>{};
					logger::info("I");
					throw nullptr;
				}
				operators_open--;
				//use_type = DataType::Operator;
				goto symbol_handle;


			case '|'://Contains: BitwiseOR(|), LogicialOR (||)
				//If the next one is another OR symbol
				pushed = *(++it) != '|';
				goto symbol_check;

			case '&'://Contains: BitwiseAND,LogicalAND
				//If the next one is another AND symbol
				pushed = *(++it) != '&';
				goto symbol_check;

			case '^':
				pushed = *(++it) != '^';
				goto symbol_check;

			case '!'://Contains: logicalNOT(unimplemented), NotEqualTo
				pushed = *(++it) != '=';
				goto symbol_check;

			case '<'://Contains: GreaterThan, GreaterOrEqual, RightShift
				++it;
				pushed = *it != '=' && *it != '<';
				goto symbol_check;
			case '>'://Contains: LesserThan, LesserOrEqual, LeftShift
				++it;
				pushed = *it != '=' && *it != '>';
				goto symbol_check;

			case '='://Contains: EqualTo
				if (*(++it) != '=') {
					ARTHMETIC_LOGGER(error, "'=' is not a valid symbol, did you mean '=='?");
					throw nullptr;
				}

				goto symbol_check;

			case '+':
			case '/':
			case '*':
			case '%':

			symbol_check:
				if (IsValidCodeChar(RoutineItemType::Operator, last_type) == false) {
					//cout << "ERROR, \"Operator\" item \"" << *it << "\" cannot be next to " << ArthmeticUtility::PrintRoutineItemType(last_type) << ".";
					//return std::vector<std::string>{};
					logger::info("J");
					throw nullptr;
				}
				//use_type = DataType::Operator;

//Symbol handle, simply pushes symbol
			symbol_handle:
				//push_value = *it;

				//if it didn't push, we push it forward one. If it did push, we don't.
				push_value = RecordData(true_it, it += !pushed, DataType::Operator);
				pushed = true;
				logger::info("make symbol \'{}\'", push_value.view);
				break;
			case '{':
				//Functional handle, Not implemented, merely ignore.

			default:
				//Input is invalid, do not use. Also pull the value that offends.
				//cout << "ERROR, invalid value \"" << *it << "\" found.";
				//return std::vector<std::string>{};
				logger::info("K");
				throw nullptr;
			}

			//We do this after so that I can effectively clear space if anything else at all other than a space value is used,
			// but if space would function the same if it was in the spacing code.
			space_detected = hit_spacing;

			//May need to do something else with this portion.
			if (push_value.view != "")
				return_vector.push_back(push_value);
			//These 2 could be linked
			if (!pushed)
				++it;
		}

		//Both of these should be handled by the operator parsing.
		if (operators_open != 0) {
			ARTHMETIC_LOGGER(error, "ERROR, An operator was left open by a count of {}.", operators_open);
			//return std::vector<std::string>{};
			throw nullptr;
		}
		else if (full_string && last_type == RoutineItemType::Operator) {
			//cout << "ERROR, An operator standard operator cannot be the last item in the function.";
			//return std::vector<std::string>{};
			ARTHMETIC_LOGGER(info, "ERROR, An operator standard operator cannot be the last item in the function.");
			throw nullptr;
		}

		logger::info("~end of it parse");

		return return_vector;
	}

};

using NativeFormula = float(*)(Target, const ArgumentList&);



using LinkNatFunc = int(*)(std::string_view, int, NativeFormula);


namespace ArithmeticAPI
{
	//This should use a macro
	constexpr auto SOURCE = L"ActorValueExtension.dll";

	//The idea is that when it's source, it's a delegate argument, when it's not its "argument" that's the API version
	// I can likely make argument just like, some object that in source doesn't exist, and is just delegate Argument, but should it be API
	// it basically subs the delegate argument.
	using TEMP_ARG = std::conditional<true, DelegateArgument, Argument>;

	enum Version
	{
		Version1,


		Current = Version1
	};

	struct InterfaceVersion1
	{
		inline static constexpr auto VERSION = Version::Version1;

		virtual ~InterfaceVersion1() = default;

		virtual Version GetVersion() = 0;

		//These can have the interesting ability of switching between resolved argument and delegate argument depending on source
		// or not.


		//Target will have to be defined here if there's no source, just for the transfers sake. It doesn't nothing special anyhow.
		virtual Target GetObject(TEMP_ARG*) = 0;

		virtual float GetNumber(TEMP_ARG*) = 0;

		virtual char* GetString_Impl(TEMP_ARG*) = 0;

		virtual void LinkNativeFunction(std::string_view, int, NativeFormula) = 0;
	};

	using CurrentInterface = InterfaceVersion1;

	CurrentInterface* g_Interface{ nullptr };

	//I think I would possibly make this request g_Interface in non-source?
	CurrentInterface* InferfaceSingleton();

	namespace detail
	{
		//CurrentInterface




		static __declspec(dllexport) void* RequestInterfaceImpl(Version version)
		{
			CurrentInterface* result = InferfaceSingleton();

			switch (version)
			{
			case Version::Version1:
				return dynamic_cast<InterfaceVersion1*>(result);
			default:
				//Should show an error or something like that.
				return nullptr;
			}

			return nullptr;
		}
	}

	//Use arth logger(or api logger I guess) only if defined.

	//This should cast itself to different versions depending on what is requested.
	void* RequestInterface(Version version)
	{
		typedef void* (__stdcall* RequestFunction)(Version);

		static RequestFunction request_interface = nullptr;

		HINSTANCE API = GetModuleHandle(SOURCE);
		//if (!a_load || a_load->GetPluginInfo("po3_KeywordItemDistributor.dll") == nullptr) {
		if (API == nullptr) {
			ARTHMETIC_LOGGER(critical, "{} not found, API will remain non functional." API_SOURCE);
			return;
		}

		//Before this, there should be a request, so if I introduce new versions, I know what level of things it should get.
		request_interface = (RequestFunction)GetProcAddress(API, "RequestInterfaceImpl");

		//If source, get the created version

		g_Interface = request_interface(version);

		return g_Interface;
	}
	//use a derived from
	template <class Interface>
	inline  Interface* RequestInterface()
	{
		void* interface = RequestInterface(Interface::VERSION);

		return reinterpret_cast<Interface*>(interface);
	}

}


namespace Arithmetic
{


#include <vector>


	//Put the actual API here.




	struct Argument
	{
		//So the member functions are for look
		//The API functions are what the members call,
		//And the IMPL function are what actually gets stored and called by other plugins.
		//Overall, Argument's existence only matters as an address, nothing more.
		
		Target GetObject()
		{
			return ArithmeticAPI::g_Interface->GetObject(this);
		}

		float GetNumber()
		{
			return ArithmeticAPI::g_Interface->GetObject(this);
		}
		std::string GetString()
		{
			return ArithmeticAPI::g_Interface->GetObject(this);
		}

	private:


		//There is indeed no way to actually create this object, as it doesn't exist. It is only
		// to be used as a pointer, as it is a mask over DelegateArguments. This is then the version that 
		// function interfaces will send.
		//There is an idea of Argument, an abstraction. It simple is not there.
		Argument() = delete;
		Argument(const Argument&) = delete;
		Argument(Argument&&) = delete;

		Argument(Argument&&) = delete;
		Argument(Argument&&) = delete;
		~Argument() = delete;
	};

	//This object will actually need extra work to send, as in it will need to get the address of every single delegate arugment,
	// then that gets stuffed into this vector after a reinterpret cast.
	using ArgumentList = std::vector<Argument*>;

}



using ExportFunction = void(*)(RE::Actor*, std::string, RE::ACTOR_VALUE_MODIFIER, float);


namespace ActorValueGeneratorAPI
{
	//Defines what plugin is actually using the Lexicon API
#define ARITHMETIC_API_SOURCE L"ActorValueExtension.dll"

	enum Version
	{
		Version1,


		Current = Version1
	};

	struct InterfaceVersion1
	{
		inline static constexpr auto VERSION = Version::Version1;

		virtual ~InterfaceVersion1() = default;

		virtual Version GetVersion() = 0;

		//These can have the interesting ability of switching between resolved argument and delegate argument depending on source
		// or not.


		//Target will have to be defined here if there's no source, just for the transfers sake. It doesn't nothing special anyhow.
		void RegisterExportFunction(std::string, ExportFunction) = 0;

		void CheckActorValue(RE::ActorValue&, const char*) = 0;

	};

	using CurrentInterface = InterfaceVersion1;

	CurrentInterface* g_Interface{ nullptr };

	//I think I would possibly make this request g_Interface in non-source?
	CurrentInterface* InferfaceSingleton();

	namespace detail
	{
		//CurrentInterface



		//extern "C" __declspec(dllexport) 
		static __declspec(dllexport) void* RequestInterfaceImpl(Version version)
		{
			CurrentInterface* result = InferfaceSingleton();

			switch (version)
			{
			case Version::Version1:
				return dynamic_cast<InterfaceVersion1*>(result);
			default:
				//Should show an error or something like that.
				return nullptr;
			}

			return nullptr;
		}
	}

	//Use arth logger(or api logger I guess) only if defined.

	//This should cast itself to different versions depending on what is requested.
	void* RequestInterface(Version version)
	{
		typedef void* (__stdcall* RequestFunction)(Version);

		static RequestFunction request_interface = nullptr;

		HINSTANCE API = GetModuleHandle(L"ActorValueExtension.dll");
		//if (!a_load || a_load->GetPluginInfo("po3_KeywordItemDistributor.dll") == nullptr) {
		if (API == nullptr) {
			ARTHMETIC_LOGGER(critical, "{} not found, API will remain non functional." API_SOURCE);
			return;
		}

		//Before this, there should be a request, so if I introduce new versions, I know what level of things it should get.
		request_interface = (RequestFunction)GetProcAddress(API, "RequestInterfaceImpl");

		//If source, get the created version

		g_Interface = request_interface(version);

		return g_Interface;
	}
	//use a derived from
	template <class Interface = CurrentInterface>
	inline  Interface* RequestInterface()
	{
		void* interface = RequestInterface(Interface::VERSION);

		return reinterpret_cast<Interface*>(interface);
	}

}

namespace AVG
{
	struct DynamicValueID
	{
		RE::ActorValue actorValue = RE::ActorValue::kNone;

		DynamicValueID() = default;

		constexpr DynamicValueID(RE::ActorValue a_rhs) : actorValue(a_rhs) {}

		template <class StringType> requires (std::is_same_v<StringType, std::string_view> || std::is_same_v<StringType, std::string>)
			DynamicValueID(StringType a_rhs)
		{
			ActorValueGeneratorAPI::g_InterFace->CheckActorValue(actorValue, a_rhs);
		}


		constexpr DynamicValueID& operator=(RE::ActorValue a_rhs)
		{
			actorValue = a_rhs;
			return *this;
		}


		template <class StringType> requires (std::is_same_v<StringType, std::string_view> || std::is_same_v<StringType, std::string>)
			DynamicValueID& operator=(StringType a_rhs)
		{
			ActorValueGeneratorAPI::g_AVGInterFace->CheckActorValue(actorValue, a_rhs);
			return *this;
		}
	};


	template <StringLiteral Av_Name>
	struct ConstValueID
	{
		static constexpr auto ActorValueName = Av_Name.value;

		operator RE::ActorValue() const
		{
			ActorValueGeneratorAPI::g_AVGInterFace->CheckActorValue(_actorValueID, ActorValueName);
			return _actorValueID;
		}

	private:
		static inline RE::ActorValue _actorValueID = RE::ActorValue::kNone;
	};
}

