#pragma once

#include "Arthmetic/Record.h"
#include "Arthmetic/ArthmeticObject.h"
#include "Arthmetic/RoutineItemType.h"
#include "Arthmetic/ArthmeticUtility.h"

namespace Arthmetic
{
	constexpr char k_quotationChar = '\'';

	//Whole sale renaming is in order here.

	enum struct ParseIgnoreClause
	{
		None, 
		Quotation,
		ObjectDeclare,
	};

	//Declarative Region
	inline std::vector<RecordData> IteratorParse(StringIterator& it, StringIterator end, bool full_string);

	inline RecordData ParseNumber(StringIterator& it, StringIterator begin, StringIterator end, bool has_point = false, DataType extra_flags = DataType::Invalid)
	{

		//logger::info("{} aaaaaaaaaaaaaa", *it);
		//logger::info("{} bbbbbbbbbbbbbb", *begin);
		//logger::info("{} cccccccccccccc", *end);
		while (it != end) {
			if (!has_point && *it == '.') {
				has_point = true;
				goto inc;
			} else if (std::isdigit(*it) != 0) {
				goto inc;
			} 
			else {
				break;
			}

		inc:
			++it;
		}
		//logger::info("finish number: {}, same ? {} {}", std::string_view(begin, end), begin == it, it == end);
		return RecordData(begin, it, DataType::Number | extra_flags);
	}

	inline RecordData ParseNumber(StringIterator& it, StringIterator end, bool has_point = false, DataType extra_flags = DataType::Invalid)
	{
		return ParseNumber(it, it, end, has_point, extra_flags);
	}

	inline void ParseArguments(StringIterator& it, StringIterator end, std::vector<RecordData>& out_vector)
	{
		//cout << "HIT 1";

		//for now, encountering multiple paras should be an X.
		// I may be able to do arethmatic in these, but one thing at a time.

		//std::vector<std::string> return_strings;

		StringIterator new_begin = it;

		//This is how many "(" we've passed. Once it's zero, we can exit the function, giving the last command ~ instead of #
		int params_open = 1;  //Make not need, if we encounter

		ParseIgnoreClause ignoring = ParseIgnoreClause::None;

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
			switch (ignoring)
			{
			case ParseIgnoreClause::Quotation:
				if (*it == k_quotationChar && last_char != '\\') {  //if it was \" it is likely specificly a quote in the string. Not as if that would be allowed.
					ignoring = ParseIgnoreClause::None;

					//cout << *new_begin << " / " << *new_end << std::endl;
					ARTHMETIC_LOGGER(info, "{}", std::string_view(new_begin, it));
					//std::string push_string = std::string(new_begin, it) + "$";
					//create and push the string.

					//return_strings.push_back(push_string);
					out_vector.push_back(RecordData(new_begin, it, DataType::String));

					new_begin = it + 1;
				}
				goto inc;

			case ParseIgnoreClause::ObjectDeclare:
				if (*it == '}' && last_char != '\\') {  //if it was \" it is likely specificly a quote in the string. Not as if that would be allowed.
					ignoring = ParseIgnoreClause::None;

					//cout << *new_begin << " / " << *new_end << std::endl;
					ARTHMETIC_LOGGER(info, "{}", std::string_view(new_begin, it));
					//std::string push_string = std::string(new_begin, it) + "$";
					//create and push the string.

					//return_strings.push_back(push_string);
					out_vector.push_back(RecordData(new_begin, it, DataType::Object));

					new_begin = it + 1;
				}
				goto inc;

			default:
				if (*it == k_quotationChar) {  //But not check happens here because if a quote hasn't been opened \" doesn't do anything.
					//logger::info("quote detected");
					ignoring = ParseIgnoreClause::Quotation;
					new_begin = ++it;
					goto set_last;
				}
				else if (*it == '{')
				{
					//logger::info("bracket detected");
					ignoring = ParseIgnoreClause::ObjectDeclare;
					new_begin = ++it;
					goto set_last;
				}

				break;//Checks for parse break.
			}
			
	
			//logger::info("tell {}", *it);

			switch (*it) {
			//These needs more control on comma placement, for now, I'll just be smart.
			case ',':
				{
					//When you encounter this, send the function off. Make a string, then break it down.
					//string tmp_section = std::string(new_begin, it);
					//This works with arrays
					//a.insert(std::end(a), std::begin(b), std::end(b));

					ARTHMETIC_LOGGER(info, "'{}'...", std::string_view(new_begin, it));

					auto result = IteratorParse(new_begin, it, false);

					if (result.size() != 0)
					{
						//If the size is greater than one, the record created is marked to be made it's own subroutine.
						//Note, subroutines and it's ilk are made to be portable. With this being said, subroutines are currently the size
						// of a vector. Which is about 3 pointers + 1 more because of alignment.
						// This increases the size of a delegate parameter by quite a bit from what I assume is currently about 2 pointers.
						// It's really a question of how much space do I think it's gonna take up.
						//It's because of this (and because I don't think a vector can be in a union) that I'll make a managed pointer to it.
						// 
						if (result.size() > 1 || IsStandardValue(result[0].type) == false)
						{//If larger than one, it's def a subroutine, and if it has an abnormal type its def an arth value.
							//If they're the same, it's cool. Really don't matter in the ones it would.

							auto& front = result.front();
							auto& back = result.back();
							front.type |= DataType::SubArthmetic;
							back.type |= DataType::LastEntry;
							
							//logger::info("front {} // {:B}", front.view, (int)front.type);
							//logger::info("back {} // {:B}", back.view, (int)back.type);
						}
					}

					it = new_begin;
					new_begin++;

					////cout << std::endl << " end at " << *new_begin << std::endl;

					//Size should be allowed to be zero right?
					if (result.size() > 0)
					{
						//return_strings.insert(return_strings.end(), result.begin(), result.end());
						out_vector.insert(out_vector.end(), result.begin(), result.end());
					}

					break;
				}
			case '(':
				params_open++;
				break;
			case ')':
				//I want to combine these, but I'm sorta not sure how

				params_open--;

				if (params_open <= 0) {
					if (params_open == 0)
					{
						//Might prevent this from running if I read there's nothing relevant. I'll have something do some detection
						// since the last comma
						auto result = IteratorParse(new_begin, it, false);
						

						if (result.size() != 0)
						{
							if (result.size() > 1 || IsStandardValue(result[0].type) == false)
							{//If larger than one, it's def a subroutine, and if it has an abnormal type its def an arth value.
								//If they're the same, it's cool. Really don't matter in the ones it would.
								
								auto& front = result.front();
								auto& back = result.back();
								front.type |= DataType::SubArthmetic;
								back.type |= DataType::LastEntry;

								//logger::info("front {} // {:B}", front.view,(int)front.type);
								//logger::info("back {} // {:B}", back.view, (int)back.type);
							}
						}

						out_vector.insert(out_vector.end(), result.begin(), result.end());

						auto& front = out_vector.front();
						auto& back = out_vector.back();

						//logger::info("add {}", out_vector.size());
						//logger::info("front {} // {:B}", front.view, (int)front.type);
						//logger::info("back {} // {:B}", back.view, (int)back.type);
						out_vector.push_back(RecordData(it, it + 1, DataType::Pass | DataType::LastEntry));

						//WATCH ME, it seems like I'm the only want that doesn't want to be processed when we make it back to the main
						// parse, but I'm unsure how well that holds.
						it++;
					}

					goto end;
				}
			}

inc:
			++it;

set_last:
			last_char = *it;
		}

end:
		
		//Don't do if empty.
		RecordData& end_param = out_vector.back();
		//end_param.type |= DataType::LastEntry;
		
		//logger::info("type: {:B}", (int)end_param.type);

		//return return_strings;
	}

	inline std::vector<RecordData> ParseName(StringIterator& it, StringIterator begin, StringIterator end)  //Used to have allow_nat
	{
		//So it's quite important that this branch out, it's currently parse name, but it's actually parse function, specifically function,
		// not routine.

		std::vector<RecordData> total_function{};

		bool nat_func = false;

		bool quotation_ignoring = false;
		char last_char = '\0';

		auto IsName = [](char character) -> bool {
			//I want to put some more usable characters in here.
			//Usable charaters
			// :: they are required to be stacked up twice, but not thrice and never to show again
			// . this is supposed to be used only from a target

			return std::isalnum(character) || character == '_';
		};

		while (it != end) {
			//This needs to account for ::

			last_char = *it;

			if (*it == '(') {
				//Nat func detected, saving the name, and printing the data
				nat_func = true;
				break;
			} else {
				auto parsable = IsName(*it);

				//We want both numbers and letters, since we now know the first is a chacater
				if (parsable == false) {
					break;  //if its not ( or {, don't we want to fail this shit then?
				} else if (parsable == -1) {
					//Illegal value used. Should be
					ARTHMETIC_LOGGER(critical, "Character '{}' isn't parsable.", *it);
					throw nullptr;
				}
			}

			//inc:
			++it;
		}
		//HERE, depending on what it is this function 'it' currently is, it should fire a different function, and commit the main name.

		//If it is a period, we would probably find some way to do the above again.

		//std::string main_name = std::string(new_begin, it);
		RecordData main_name(begin, it, nat_func ? DataType::FormulaCall : DataType::ParameterCall);//For now, just function.

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

	inline std::vector<RecordData> IteratorParse(StringIterator& it, StringIterator end, bool full_string)
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

			//DataType use_type = DataType::Invalid;

			//logger::info("{}", *it);


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
					ARTHMETIC_LOGGER(critical, "\"FunctionValue\" item '{}' cannot be next to {}.", *it, PrintRoutineItemType(last_type));
					throw nullptr;
				} else {
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
				
				//ARTHMETIC_LOGGER(info, "Handle digit ({}, \'{}\').", PrintRoutineItemType(last_type), load_char);

				if (IsValidCodeChar(RoutineItemType::Value, last_type) == false) {
					//cout << "ERROR, \"Value\" item cannot be next to " << PrintRoutineItemType(last_type) << ".";
					//return std::vector<std::string>{};
					ARTHMETIC_LOGGER(critical , "ERROR, \"Value\" item cannot be next to {} (char \'{}\').", PrintRoutineItemType(last_type), load_char);
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
					ARTHMETIC_LOGGER(critical, "invalid decimal found without proper spacing.");
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
						ARTHMETIC_LOGGER(critical, "\"Value\" item '{}' cannot be next to {}.", *it, PrintRoutineItemType(last_type));
						throw nullptr;
					}

					//push_value += ParseNumber(it, true_it, end, true) + "#";
					push_value = ParseNumber(it, true_it, end, true);
				} 
				else 
				{
					//cout << "ERROR, invalid decimal with no number found.";
					//return std::vector<std::string>{};
					ARTHMETIC_LOGGER(critical, "Invalid decimal with no number found.");
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
						ARTHMETIC_LOGGER(critical, "\"Value\" item '{}' cannot be next to {}.", *it, PrintRoutineItemType(last_type));
						throw nullptr;
					}

					//push_value += ParseNumber(it, true_it, end, false) + "#";
					push_value = ParseNumber(it, true_it, end, false);
				} else if (*it == '.') {
					goto decimal_handle;
				} else if (IsValidCodeChar(RoutineItemType::Operator, last_type) == false) {
					//cout << "ERROR, \"Operator\" item cannot be next to " << ArthmeticUtility::PrintRoutineItemType(last_type) << ".";
					//return std::vector<std::string>{};
					ARTHMETIC_LOGGER(critical, "\"Operator\" item '{}' cannot be next to {}.", *it, PrintRoutineItemType(last_type));
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
					ARTHMETIC_LOGGER(critical, "\"Operator\" item '{}' cannot be next to {}.", *it, PrintRoutineItemType(last_type));
					throw nullptr;
				} else if (operators_open <= 0) {
					//cout << "ERROR, An operator was closed with no open symbol.";
					//return std::vector<std::string>{};
					ARTHMETIC_LOGGER(critical, "An operator was closed with no open symbol.");
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
				if (*(++it) != '='){
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
					ARTHMETIC_LOGGER(critical, "\"Operator\" item '{}' cannot be next to {}.", *it, PrintRoutineItemType(last_type));
					throw nullptr;
				}
				//use_type = DataType::Operator;

//Symbol handle, simply pushes symbol
symbol_handle:
				//push_value = *it;

				//if it didn't push, we push it forward one. If it did push, we don't.
				push_value = RecordData(true_it, it += !pushed, DataType::Operator);
				pushed = true;
				ARTHMETIC_LOGGER(debug, "make symbol \'{}\'", push_value.view);
				break;
			case '{':
				//Functional handle, Not implemented, merely ignore.

			default:
				//Input is invalid, do not use. Also pull the value that offends.
				//cout << "ERROR, invalid value \"" << *it << "\" found.";
				//return std::vector<std::string>{};
				ARTHMETIC_LOGGER(critical, "Invalid value '{}' found.", *it);
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

		if (operators_open != 0) {
			ARTHMETIC_LOGGER(error, "An operator was left open by a count of {}.", operators_open);
			//return std::vector<std::string>{};
			throw nullptr;
		} else if (full_string && last_type == RoutineItemType::Operator) {
			//cout << "ERROR, An operator standard operator cannot be the last item in the function.";
			//return std::vector<std::string>{};
			ARTHMETIC_LOGGER(error, "An operator standard operator cannot be the last item in the function.");
			throw nullptr;
		}

		ARTHMETIC_LOGGER(debug, "~end of it parse");

		return return_vector;
	}

	inline Record InitialParse(const std::string& function_code)
	{
		//Has to send back a pointer because of string views. Should test functionality with unique_ptrs when I get the chance.
		// Or find some functionality of a string like object that passes ownership.
		//Basically I need what unique pointer does in exchanging ownership, but something that has the conviences of string classes.
		Record new_record(function_code);

		StringIterator it = new_record.fullData.begin();  //function_code.begin();
		StringIterator end = new_record.fullData.end();   //function_code.end();

		new_record.dataView = IteratorParse(it, end, true);

		return new_record;
	}



	////////////////
#ifdef ENABLE_TEST_FIELD

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

		std::string_view GetView() const { return std::string_view(begin, it); }

		bool IsValidEnd() const { return end == StringIterator(); }

		StringItCollection(StringIterator& a_it, StringIterator a_begin, StringIterator a_end) :
			it(a_it),
			begin(a_begin),
			end(a_end)
		{}
	};


	enum struct ParseQueryResult
	{
		No, 
		Yes,
		Cancel
	};

	class ParsingContext
	{

#define INVALID_INTERPRETATION(mc_parse_type) \
		ARTHMETIC_LOGGER(error, "Cannot interpret {}", #mc_parse_type); throw nullptr

	public:
		//This is part of an idea where it holds the type and the later object must specify the type in order to use it. Will avoid if I can.
		//Type id = TypeInfo<void>;

		//ParsingContext() = delete;

		//I would like to determine a priority with these, With the default parser falling last regardless
		// of any other.

		virtual ParseQueryResult ShouldInterpret(Parser*, ParsingType, StringItCollection) = 0;
		virtual ParseQueryResult ShouldAllowInterpret(Parser*, ParsingContext*, ParsingType, StringItCollection) { return ParseQueryResult::Yes; }

		//there's an issue in this where loose records could be returned, or regular records could be returned.
		// The issue being ownership
		//So what I could do is no return types, instead I have an object that takes the new records to make. Instead of giving it new
		// records, I give it record components. This way, it has the ability when I upload it to make RecordData or LooseRecords
		//OR, I just use record data, and that record data will be turned into a series of loose Records if the situation calls for it.

		virtual std::vector<RecordData> InterpretName(Parser*, StringItCollection) { INVALID_INTERPRETATION(Name); }
		virtual std::vector<RecordData> InterpretNumber(Parser*, StringItCollection) { INVALID_INTERPRETATION(Number); }
		virtual std::vector<RecordData> InterpretSymbol(Parser*, StringItCollection) { INVALID_INTERPRETATION(Symbol); }

		//Something used by the last context to determine the last character should be based.
		//virtual void IsPassChar() = 0;

	};


	std::array<std::vector<ParsingContext*>, ParsingType::Total> parsingMap;

	struct ErrorContext : public ParsingContext
	{
		//This exists solely to have report errors.
		ParseQueryResult ShouldInterpret(Parser*, ParsingType, StringItCollection) override { return ParseQueryResult::Yes; }
	};

	inline ErrorContext _errorContext;

	//Some things can only be a primary context, some things can


	struct NumberContext : public ParsingContext
	{

		ParseQueryResult ShouldInterpret(Parser* parser, ParsingType type, StringItCollection collect) override
		{
			return ParseQueryResult::Yes;
		}
		
		ParseQueryResult ShouldAllowInterpret(Parser* parser, ParsingContext* query_context, ParsingType type, StringItCollection collect) override
		{
			return ParseQueryResult::Yes;
		}

		std::vector<RecordData> InterpretName(Parser* parser, StringItCollection collect) 
		{ 
			INVALID_INTERPRETATION(Name); 
		}
		
		
		std::vector<RecordData> InterpretNumber(Parser* parser, StringItCollection collect) override
		{
			INVALID_INTERPRETATION(Number); 
		}
		
		std::vector<RecordData> InterpretSymbol(Parser* parser, StringItCollection collect) override
		{ 
			INVALID_INTERPRETATION(Symbol); 
		}

	};


	struct VariableContext : public ParsingContext
	{

		ParseQueryResult ShouldInterpret(Parser* parser, ParsingType type, StringItCollection collect) override
		{
			return ParseQueryResult::Yes;
		}

		ParseQueryResult ShouldAllowInterpret(Parser* parser, ParsingContext* query_context, ParsingType type, StringItCollection collect) override
		{
			return ParseQueryResult::Yes;
		}

		std::vector<RecordData> InterpretName(Parser* parser, StringItCollection collect)
		{
			INVALID_INTERPRETATION(Name);
		}


		std::vector<RecordData> InterpretNumber(Parser* parser, StringItCollection collect) override
		{
			INVALID_INTERPRETATION(Number);
		}

		std::vector<RecordData> InterpretSymbol(Parser* parser, StringItCollection collect) override
		{
			INVALID_INTERPRETATION(Symbol);
		}
	};


	struct ObjectContext : public ParsingContext
	{
		//Falls under symbol, as it is the explicit declaration of an object.
		// May merge with number in due time as "LiteralContext"


		ParseQueryResult ShouldInterpret(Parser* parser, ParsingType type, StringItCollection collect) override
		{
			return ParseQueryResult::No;
		}

		std::vector<RecordData> InterpretSymbol(Parser* parser, StringItCollection collect) override
		{
			INVALID_INTERPRETATION(Symbol);
		}
	};


	struct OperatorContext : public ParsingContext
	{
		//Symbol, and some name. Symbol is low priority, Name is high priority


		ParseQueryResult ShouldInterpret(Parser* parser, ParsingType type, StringItCollection collect) override
		{
			if (type == ParsingType::Name)
				return ParseQueryResult::No;

			switch (*collect.begin) 
			{
			case '-':
			case '(':
			case ')':
			case '|'://Contains: BitwiseOR(|), LogicialOR (||)
			case '&'://Contains: BitwiseAND,LogicalAND
			case '^':
			case '!'://Contains: logicalNOT(unimplemented), NotEqualTo
			case '<'://Contains: GreaterThan, GreaterOrEqual, RightShift
			case '>'://Contains: LesserThan, LesserOrEqual, LeftShift
			case '='://Contains: EqualTo
			case '+':
			case '/':
			case '*':
			case '%':
			//case '{':
				if (collect.IsValidEnd() == false) {
					//Log an interesting tale.
					return ParseQueryResult::Cancel;
				}
				else {
					return ParseQueryResult::Yes;
				}

			default:
				return ParseQueryResult::No;
			}
		}

		std::vector<RecordData> InterpretName(Parser* parser, StringItCollection collect)
		{
			INVALID_INTERPRETATION(Name);
		}

		std::vector<RecordData> InterpretSymbol(Parser* parser, StringItCollection collect) override
		{
			return { RecordData(collect.GetView(), DataType::Operator) };
		}
	};


	struct NumberContext : public ParsingContext
	{
		//Falls under number (obvs) and symbol as high priority, temporarily. Also falls under name so I can register for INFINITY or MAX and stuff.


		ParseQueryResult ShouldInterpret(Parser* parser, ParsingType type, StringItCollection collect) override
		{
			if (type != ParsingType::Number)
			{
				if (*collect.begin != '-' || *collect.begin != '.')
					return ParseQueryResult::No;

				if (!collect.IsValidEnd() || std::isdigit(*collect.end) == false)
					return ParseQueryResult::No;
			}

			return ParseQueryResult::Yes;
		}


		std::vector<RecordData> InterpretName(Parser* parser, StringItCollection collect)
		{
			INVALID_INTERPRETATION(Name);
		}



		inline RecordData ParseNumber(StringIterator& it, StringIterator begin, StringIterator end)
		{
			bool has_point = false; 

			while (it != end) {
				if (!has_point && *it == '.') {
					has_point = true;
					goto inc;
				}
				else if (std::isdigit(*it) != 0) {
					goto inc;
				}
				else {
					break;
				}

			inc:
				++it;
			}

			return RecordData(begin, it, DataType::Number);
		}

		std::vector<RecordData> InterpretNumber(Parser* parser, StringItCollection collect) override
		{
			return { ParseNumber(collect.it, collect.begin, collect.end) };
		}

		std::vector<RecordData> InterpretSymbol(Parser* parser, StringItCollection collect) override
		{
			auto it_begin = collect.begin;

			it_begin += *collect.begin == '-' ? 1 : 0;

			return { ParseNumber(it_begin, collect.begin, collect.end) };
		}
	};

	//May make this derive from another kind of context that sets it apart from these other types.
	struct RoutineContext : public ParsingContext
	{
		//Falls under number (obvs) and symbol as high priority, temporarily. Also falls under name so I can register for INFINITY or MAX and stuff.


		ParseQueryResult ShouldInterpret(Parser* parser, ParsingType type, StringItCollection collect) override
		{
			return ParseQueryResult::Yes;
		}

		ParseQueryResult ShouldAllowInterpret(Parser* parser, ParsingContext* query_context, ParsingType type, StringItCollection collect) override
		{
			return ParseQueryResult::Yes;
		}

		//Has no interpretations.
	};


	union ParseData
	{
		float _float;
		uint32_t _integer;
	};

	struct Parser
	{
		ParsingContext* primaryContext;
		std::array<ParsingContext*, 2> previousContext;

		
		//This is used for stuff like open operator, stuff that will allow last checks
		// I may not need this depending on how I go about such a thing.
		std::map<std::string_view, std::pair<ParsingContext*, ParseData>> dataMap;

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
			//LAST is the more important type.
			auto type = types.first;
			auto last_type = types.second;

			if (last_type == ParsingType::Pass)
				//do what?
				return std::vector<RecordData>();

			std::vector<ParsingContext*>& contexts = parsingMap[type];

			for (auto& context : contexts)
			{
				ParseQueryResult query = context->ShouldInterpret(this, last_type, collection);



				if (query == ParseQueryResult::Cancel)
					throw nullptr;//Should bail, I couldn't care right now, previous should report.
				else if (query == ParseQueryResult::No)
					continue;

				query = current_context->ShouldAllowInterpret(this, context, last_type, collection);

				//Print x isn't an allowed interpretation something rather other.
				if (query == ParseQueryResult::Cancel)
					throw nullptr;//Should bail, I couldn't care right now, previous should report.
				else if (query == ParseQueryResult::No)
					continue;


				switch (last_type)
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

			//RoutineItemType last_type = RoutineItemType::None;

			ParsingType last_type = ParsingType::Pass;

			StringIterator true_it = it;

			StringItCollection collect{ it, it, end };


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

					if (it == true_it)
						it++;
					last_type = type;
					continue;
				}
				else if (last_type == ParsingType::Pass) {
					collect.begin = it;

					if (it == true_it)
						it++;
					last_type = type;
					continue;
				}

				//We'll want to start parsing if the type has changed, and it the last type wasn't whitespace.


				//Should the thing actually be interpreted, the last_type should be set to pass, just to prevent some rogue error

				auto add = ParseData(context, std::make_pair(type, last_type), collect);

				if (add.size() != 0) {
					return_vector.insert(return_vector.end(), add.begin(), add.end());
				}

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
					last_type = GetParseType(it);
					//last_type = type;
				}
				//Other things it should do if it has moved, last type should be reevaluated with get parse type.
			}
		}


		inline Record InitialParse(const std::string& function_code)
		{
			
			Record new_record(function_code);

			StringIterator it = new_record.fullData.begin();  //function_code.begin();
			StringIterator end = new_record.fullData.end();   //function_code.end();

			new_record.dataView = ParseRange(primaryContext, it, end);

			return new_record;
		}

	};

#endif
}