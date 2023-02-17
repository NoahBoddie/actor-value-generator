#pragma once

#include "Arthmetic/ArthmeticObject.h"
#include "Arthmetic/RoutineItemType.h"


namespace Arthmetic
{
	class RoutineArgument;
	class RoutineItem;

	using RoutineItemFactory = ClassFactory<RoutineItem, RoutineItemType>;

	template <class DerivedType, RoutineItemType Value>
	using RoutineItemFactoryComponent = IndirectFactoryComponent<RoutineItem, RoutineItemType, DerivedType, Value>;


#define DeclareRoutineItemFactory(mc_class_name, mc_routine_type) \
    mc_class_name : public RoutineItemFactoryComponent<mc_class_name, RoutineItemType::##mc_routine_type>

	

	struct RoutineItem : public ArthmeticObject
	{
		//To be const
		uint16_t _position{};

		RoutineItemType _type{ RoutineItemType::None };
		//Within this function it should know it's place, so submit it

		//Returns false if it is unable to run

		RoutineItem* GetBack(RoutineArgument* argument);
		RoutineItem* GetFront(RoutineArgument* argument);

		//Maybe should give NaN instead
		float GetBackValue(RoutineArgument* argument);
		float GetFrontValue(RoutineArgument* argument);



		virtual void Run(RoutineArgument* argument, float& return_value, size_t& index) = 0;

		void RunImpl(RoutineArgument* argument, float& return_value, size_t& index)
		{
			//if (IsValid() == false) {
			//	return_value = 0;
			//	return;
			//}
			logger::debug("RoutineType {} {}", PrintRoutineItemType(GetItemType()), _position);
			Run(argument, return_value, index);
		}


		inline void RunImpl(RoutineArgument* arg, float& return_value)
		{
			size_t index{};
			RunImpl(arg, return_value, index);
		}
		inline float RunImpl(RoutineArgument* arg)
		{
			float return_value{};
			RunImpl(arg, return_value);
			return return_value;
		}

		int32_t GetPosition() { return _position; }
		virtual OperatorType GetOperator() { return OperatorType::Invalid; }
		virtual RoutineItemType GetItemType() = 0;//Removing this for now.


		static RoutineItem* Create(RecordIterator& data, uint16_t pos, RoutineItemType type)//Include record.
		{
			RoutineItem* item = RoutineItemFactory::Create(type);

			item->_position = pos;

			item->LoadFromViewImpl(data);

			return item;
		}
		
		template<std::derived_from<RoutineItem> DerivedItem>
		inline static DerivedItem* CreateAs(RecordIterator& data, uint16_t pos, RoutineItemType type)//Include record.
		{
			RoutineItem* item = Create(data, pos, type);

			DerivedItem* result = dynamic_cast<DerivedItem*>(item);

			//This needs to be handled better
			if (!result) {
				delete item;
			}

			return result;
		}

		//Routine

	protected:
		RoutineItem() = default;

		//RoutineItem(uint16_t pos, RoutineItemType type) :
		//	_position(pos), _type(type) {}
	};

}