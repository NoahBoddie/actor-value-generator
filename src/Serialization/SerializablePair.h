#pragma once

#include "Serialization/SerialConstructor.h"
#include "Serialization/SerialArgument.h"


namespace RGL
{

	//Would be helpful having this derive from something like iterator.
	//This can actually just be an ISerializer
	template<class TypeA, class TypeB>
	class SerializablePair : public SerializationHandler
	{
	private: 
		using Pair = std::pair<TypeA, TypeB>;
	protected:
		Pair _pair;
	public:

		void HandleSerialize(SerialArgument& serializer, bool& success) override 
		{ 
			serializer.Serialize(_pair.first);
			serializer.Serialize(_pair.second);
		}

		TypeA& first = _pair.first;
		TypeB& second = _pair.second;

		constexpr Pair& get() noexcept { return _pair; }

		constexpr Pair* operator->() noexcept { return &_pair; }



	};


}