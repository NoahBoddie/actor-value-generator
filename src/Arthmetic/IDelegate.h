#pragma once

#include "Arthmetic/DelegateType.h"
#include "Arthmetic/RoutineItem.h"
#include "Arthmetic/DelegateArgument.h"
#include "Arthmetic/IReadyArthmetic.h"


#include "Arthmetic/Subroutine.h"
#include "Arthmetic/ArthmeticConstructor.h"

namespace Arthmetic
{

	//Should rename IDelegate
	struct IDelegate : public RoutineItem, public IReadyArthmetic
	{
		//This would probably just be a centralizer, likely will do nothing.

		//TargetType _target = TargetType::Self;  //Put me in super class

		std::unique_ptr<TargetSelector> _targeter{ nullptr };

		float defaultValue = 0;
		
		//These are the old parameters, this is getting phased out too.
		std::vector<DelegateArgument> params;

		//Something rather other like this, basically a key map of all parameters.
		//Since they have some defaults
		std::unordered_map<size_t, DelegateArgument>* _params = nullptr;

		virtual DelegateType DelegateType() { return DelegateType::None; }

		
		//RoutineItemType GetItemType() override { return RoutineItemType::FunctionValue; }

		void OnDeclareOwner(std::deque<ArthmeticObject*>& owner_stack) override
		{
			logger::info("Ran the delegate at pos {}", _position);
		}


	protected:
		//Make 2 versions of this, one deals to delegate, and the other just deals out delegate parameters.
		inline bool MakeParameters(RecordIterator& it)
		{
			//any new functions this has to make (later) will be on it's own index. Not here.

			//This will have to work for now, but I'd like a different way to handle this.
			std::string code{ it->view };

			bool end = !!(it->type & DataType::LastEntry);

			DataType type = it->GetType();

			bool deeper = !!(it->type & DataType::SubArthmetic);

			if (deeper)
			{
				RecordIterator end = it;

				//RecordIterator true_ending();

				//I was gonna gate this against true end, but you know, I think this will 
				// crash anyways so you know.

				while (!(end->type & DataType::LastEntry)) ++end;

				logger::info("last found at {}? {} {:B}", end->view, !(end->type & DataType::LastEntry), (int)end->type);
				end++;//Regardless, we need to iterate once, because the end is the one we don't do, so first and last entries don't cut it.

				//auto distance = std::distance(it->view.begin(), end->view.end());
				//if (distance)
				//	logger::info("experiment begin: \"{}\"...", std::string_view(it->view.data(), distance));
				//else
				logger::info("experiment begin: \"{}/{}/{}\"...", it->view, (it + 1)->view, end->view);

				std::vector<RoutineItem*> items = CreateRoutineItems(it, end);

				//for now, we just make a subroutine. I'll make it cut later.
				IReadyArthmetic* generated_routine = new Subroutine(items);

				auto param = DelegateArgument(generated_routine);

				//If something can make it through this test without sending back an error, said thing sure to be in some part
				// a const able call.

				logger::info("Testing the routine...");
				Target target{};

				float result = generated_routine->RunImpl(target);//param.GetNumberParam();

				logger::info("result first is {}", result);

				result = generated_routine->RunImpl(target);//param.GetNumberParam();

				logger::info("result second is {}", result);

				result = param.GetNumberParam();

				logger::info("result third is {}", result);
				//At this point, flag for irrelevancy.


				params.push_back(param);

				//Should return false all the time right? Because of the next has to be
				// pass?
				return false;
			}

			switch (type)
			{
			case DataType::Number:
			{
				float value = std::stof(code);//May have to find different way to do this.
				//cout << "(Loading " << value << ")";
				params.push_back(DelegateArgument(value));

				logger::info("O a {}", end);

				break;
			}
			case DataType::String:
			{
				//Its a string
				//int sub_count = end ? 2 : 1;//This is trying to curb the " I think. or the parantheses

				//code = code.substr(0, code.size() - sub_count);

				params.push_back(DelegateArgument(code));

				logger::info("O b {}", end);

				break;

			}
			case DataType::ParameterCall:
			{
				//This type of call will dominate this functionality.
				params.push_back(DelegateArgument(it));
				break;
			}

			case DataType::Object:
			{
				//This type of call will dominate this functionality.
				params.push_back(DelegateArgument(it));
				break;
			}
			case DataType::Pass:
				logger::info("Pass type found, returning.");
				//But should I just return?
				break;
			default:
				//cout << "{" << code << "} could not be interpreted." << std::endl;
				//Crashes due to parameter call, THANK GOD I didn't implement it yet.
				logger::info("O {:B}", (int)it->type);
				throw nullptr;//Currently unknown.
			}
			logger::info("O end");
			return end;
		}

		LinkerFlags GetLinkerFlags() const override 
		{ 
			//return LinkerFlags::External;

			//I sorta forgot that by nature this will have to set up with external objects. Whoops.
			LinkerFlags result_flags = LinkerFlags::External;
			
			constexpr LinkerFlags cond_flag = LinkerFlags::Object | LinkerFlags::External;
			
			for (auto& arg : params)
			{
				//This should likely be using can be's rather than anything else.

				if (arg.IsParameter() == true)
					result_flags |= LinkerFlags::External;
				else if (arg.IsParameter() == false && arg.IsObject() == true)//If it's a property, this isn't the thing that needs to confirm.
					result_flags |= LinkerFlags::Object;

				if (result_flags == cond_flag)
					break;
			}
			
			return result_flags; 
		}

		//IDelegate(uint16_t pos) :
		//	RoutineItem(pos, RoutineItemType::FunctionValue) {}
	};
}