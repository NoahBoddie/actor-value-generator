#pragma once

#include "Serialization/SerialArgument.h"
#include "Serialization/SerializingWrapper.h"

namespace RGL
{
	/*
	template <class WrapType, void(*REPLACE_WRAP_HANDLE)(WrapType&, SerialArgument&)>
	void SerializingWrapper<WrapType, REPLACE_WRAP_HANDLE>::HandleSerialize(SerialArgument& serializer)
	{
		if (REPLACE_WRAP_HANDLE)
			return REPLACE_WRAP_HANDLE(_wrapObject, serializer);
		else
			serializer.Serialize(_wrapObject);
	}
	//*/
	/*
	template <class WrapType, void(*REPLACE_WRAP_HANDLE)(WrapType&, SerialArgument&)>
	void SerializingWrapper<WrapType, REPLACE_WRAP_HANDLE>::HandleSerialize(SerialArgument& serializer)
	{
		if (REPLACE_WRAP_HANDLE)
			return REPLACE_WRAP_HANDLE(_wrapObject, serializer);
		else
			serializer.Serialize(_wrapObject);
	}
	//*/
}