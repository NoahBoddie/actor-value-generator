#pragma once

#include "Arthmetic/ArthmeticUtility.h"

#include "Arthmetic/Record.h"
#include "Arthmetic/OperatorType.h"
#include "Arthmetic/RoutineItemType.h"



namespace Arthmetic
{
	class RoutineArgument;
	class ArthmeticObject;

#define IS_TOP_OWNER() (owner_stack.top() == this)

	enum struct ObjectType
	{
		None,
		Value,
		Subroutine,
		Coroutine,
		FunctionIterface,
		FormulaDelegate,
		PropertyDelegate,
		DualOperator,
		OpenOperator,
	};

	enum struct ObjectFlags : uint8_t
	{
		Invalid				= 0,
		VailidateFailure	= 0b001, //Setting this will fail it to validate, even if it validates later.
		ValidateSuccess		= 0b010, //Flag to designate primary validation step achieved
		ValidateComplete	= 0b110, //Flag for validation completion, comprised of success for easier comparison.
		
		ValidationFlags		= 0b111, //The sum total of all flags used for validation.
		
		
		ErrorFlag			= 1 << 3,//Used when an error is spoken to not print it over and over again. May make more.
		LoadedFromView		= 1 << 4, //flag to say that load from view has finished once.
	};

	enum struct LinkerFlags : uint8_t
	{
		None,
		Object = 1 << 0,	//Used for objects who's reference of may not be visable on the spot.
		External = 1 << 1,	//Used for the reference of properties or formulas and as such must be handled after load..


		Total
	};




	struct IObject
	{
		virtual bool IsValid() const = 0;
		//A lot of delcaration is going to have to be moved here, including load from view
		// of course, all just declarative. I know this is a mite shit, but it'll have to do.


		//This is used to tell if an object is a certain type or not. Useful for delegate that need to know if it's actually storing a subroutine
		// or if the subroutine was shortened down to it's results, or if it was shortened down to a single call.
		virtual ObjectType GetObjectType() = 0;

		virtual ~IObject() = default;
	};

	struct ArthmeticObject : public virtual IObject
	{
	private:

		inline static std::map<LinkerFlags, std::list<ArthmeticObject*>> linkerMap;
		inline static std::map <ArthmeticObject*, uint32_t> linkerCount;


		void AttemptCompleteValidation() const 
		{ 
			if (IsForcedInvalid() == true || IsPartiallyValid() == false)
				return;

			flags |= ObjectFlags::ValidateComplete; 
		}

	protected:

		void AttemptValidation() const { flags |= ObjectFlags::ValidateSuccess; }

		void AttemptFullValidation() const
		{
			//Basically, if it has no link, it will attempt to jump the whole process all together
			if (GetLinkerFlags() == LinkerFlags::None) {
				AttemptValidation();
				AttemptCompleteValidation();
			}
		}

	public:
		static void FinalizeLinkage(LinkerFlags flags)
		{
			for (int i = 1 << 0; i < (uint32_t)LinkerFlags::Total; i <<= 1) {
				LinkerFlags flag = flags & static_cast<LinkerFlags>(i);

				if (!flag)
					continue;

				auto& link_list = linkerMap[flag];

				for (auto& link_target : link_list) {
					link_target->HandleLink(flag);

					if (--linkerCount[link_target] == 0) {
						link_target->AttemptCompleteValidation();
						linkerCount.erase(link_target);
					}
				}

				logger::trace("Finalized linkage: {}", (int)flag);

				linkerMap.erase(flag);
			}


		}





		mutable ObjectFlags flags = ObjectFlags::Invalid;

		bool IsValid() const override { return (flags & ObjectFlags::ValidationFlags) == ObjectFlags::ValidateComplete; }
		
		//Checks if the object had success, regardless if the object itself is still invalid.
		bool IsPartiallyValid() const { return !!(flags & ObjectFlags::ValidateSuccess); }
		
		//Checks if the object is forcibly invalid due to error.
		bool IsForcedInvalid() const { return !!(flags & ObjectFlags::VailidateFailure); }

		virtual ObjectType GetObjectType() override { return ObjectType::None; }


		virtual LinkerFlags GetLinkerFlags() const { return LinkerFlags::None; }

		virtual void OnLinkage(LinkerFlags link_type, bool& success) {}

		void HandleLink(LinkerFlags link_type)
		{
			bool result = true;

			OnLinkage(link_type, result);


			//Its also possible the impl version of the call can do this for me.
			if (result) {
				flags |= ObjectFlags::ValidateSuccess;
			}
			else{
				flags |= ObjectFlags::VailidateFailure;
			}
		}


		void LogError(std::string log)
		{
			//Logs issue then flags the error flag. Basically supposed to be used when a core error is detected, rendering the object no longer
			// usable, but can still fire off.
		}


		//Requires validating

		//I've decided the data flags would be very useful as well in the construction.

		//I seek to do that, later. But not now.
		virtual void LoadFromView(RecordIterator& data_view) 
		{
			ARTHMETIC_LOGGER(info, "{}", data_view->view);
		}// = 0;

		bool LoadFromViewImpl(RecordIterator& data_view)
		{
			LoadFromView(data_view);

			//At a later point, load from view should take over constructors, with the only exception being for 
			// routine items that make an explicit point of being created with knowing their position.


			//Query, check if it even has an extra data. If not validate immediately.

			flags |= ObjectFlags::LoadedFromView;

			//Else, it will have to wait until later to be validated.


			//This return value is supposed to record if the iterator has been moved or not.
			return false;


			//FinalizeExtra data- When finalized, it's the last data that gets throw.
			// If an arth object specifies, it will come validated. Operators, const values both come valid. Basically, anything that pushes
			// extra data has to be validated, so it's still just delegates that need validation.
			//I think some directives will need validation.

			//Actually, I think the first time extra data gets pushed back that's when validation becomes important.
			//Or maybe, I'll do it here, it'll be the first one in, and the last one out.
			//This is so I can have a special extra data exist just for RoutineItems
		}

		//So there are multiple functions related to ownership that must be created.
		
		// First, a set ownership function. I'm thinking this function is non-virtual, takes a pointer.
		//  if the pointer is null, this object is the first in queue (THOUGH, getting that queue between functions then needs transfer huh?)
		//  This function will load the pointer onto the cue, and if the pointer is null, will load the current arthmetic object onto the queue
		//  instead. Then, it will fire the next function.
		//   -Note, this function should have another version in which you don't have to load a queue, WHICH IS THE EXTERNAL VERSION
		//   


		//Then, we have a function that serves as an event. On declare ownership. This sends a reference of the queue that's been loaded.
		// since that probably takes less space on the stack. Within this there should be some easy way to tell if there's no owner above you.

	private:
		void CheckLinkValidation()
		{
			LinkerFlags links = GetLinkerFlags();

			if (!!links) {
				for (int i = 1 << 0; i < (uint32_t)LinkerFlags::Total; i <<= 1) {
					LinkerFlags flag = links & static_cast<LinkerFlags>(i);

					if (!!flag) {
						linkerMap[flag].push_back(this);
						linkerCount[this]++;
					}
				}
			}
			else {
				AttemptValidation();
				AttemptCompleteValidation();
			}
		}

		void DeclareAsOwner(std::deque<ArthmeticObject*>& owner_stack, ArthmeticObject* new_owner)
		{
			owner_stack.push_back(new_owner);

			OnDeclaredAsOwner(owner_stack);
			OnDeclareOwner(owner_stack);

			CheckLinkValidation();

			owner_stack.pop_back();
		}
			
	public://protected://Unsure why it won't let me use this as protected
		void DeclareOwner(std::deque<ArthmeticObject*>& owner_stack, ArthmeticObject* new_owner)
		{
			owner_stack.push_back(new_owner);
			
			OnDeclareOwner(owner_stack);
			
			CheckLinkValidation();

			owner_stack.pop_back();
		}


		//This dones't require implementation, just situationally does.
		//I want to make this const, but I'm just going to have to trust myself.

		virtual void OnDeclareOwner(std::deque<ArthmeticObject*>& owner_stack) {}

		virtual void OnDeclaredAsOwner(std::deque<ArthmeticObject*>& owner_stack) {}
public:

		void DeclareOwner()
		{
			//Why this shit screaming?
			std::deque<ArthmeticObject*> owner_stack{};
			DeclareAsOwner(owner_stack, this);
		};

		//Note on the deal of when we start setting ownership.
		// As for who starts the ownership chain, there's a public version of the MakeSubroutineOrReadyArth and MakeCoroutine functions, 
		// the hidden one has an option to send an ownership event once it's finished being created.



		//create some flag on this that says it's trival. 
		// This is usable for generated subroutines that are temporary, and not gotten
		// from and stored in a list.
		
		ArthmeticObject() = default;
		ArthmeticObject(const ArthmeticObject&) = delete;
		ArthmeticObject(const ArthmeticObject&&) = delete;
		ArthmeticObject& operator= (ArthmeticObject) = delete;
		ArthmeticObject& operator= (const ArthmeticObject&) = delete;
		ArthmeticObject& operator= (const ArthmeticObject&&) = delete;
	};


	//Thinking of hard coding this in templates which one should be done
	
}

#undef IS_TOP_OWNER