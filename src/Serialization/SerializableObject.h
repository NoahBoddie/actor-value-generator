#pragma once

#include "Serialization/ISerializer.h"

namespace RGL
{
	class ISerializable;
	class SerialArgument;
	class ISerialClassTrival;

	
	//Make this a template type that takes a bool, merging it with Class Irrelevant. Better solution would be
	// making a binary enum to make it a bit more readable.
	class ISerializable
	{
	public:
		//The type is solely to resolve ambiguity, so it happens simply.
		template<std::derived_from<ISerializable> SerializableObject>
		inline SerializableObject* As(SerializableObject* type) { return static_cast<SerializableObject*>(this); }
		

		//A function for if serialization and deserialization largely handles exactly the same.
		void HandleSerialize(SerialArgument& serializer, bool& success, ISerializable* pre_process) {}

		//I'm thinking of making this a single function with OnDeserialize
		void OnSerialize(SerialArgument& serializer, bool& success, ISerializable* pre_process) {}


		void OnDeserialize(SerialArgument& serializer, bool& success, ISerializable* pre_process) {}
	};
	
	//Marker class to say the class type data shouldn't matter, as long as it remains the same size. Used for stuff like relink
	// pointer where while it's giving the relink pointer object, it can and does get derived, but the data being the same size
	// shouldn't cause undefined behaviour
	class ISerialClassTrival : public ISerializable {};
}