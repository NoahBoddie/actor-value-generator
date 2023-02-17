#pragma once

#include "Serialization/ISerializer.h"

namespace RGL
{


	template<bool value>
	class IOpenSerializer {};

	using OpenSerializer = IOpenSerializer<true>;


	template <class Type> constexpr inline bool is_open_serializer = std::derived_from<Type, OpenSerializer> == true;


	class SerialArgument;

	class SerializationHandler : public ISerializer
	{
		//This shit is basically used to iterface with that dogshit serialConstructor,
		//Cause it's a template, we have no fucking way to get what its entry type is, hence, this weird shit.
	public:
		virtual void HandleSerialize(SerialArgument& serializer, bool& success) = 0;

		virtual void Revert() {}

		template<std::derived_from<SerializationHandler> Serializer, class OverarchingType, class... Args> 
			requires(std::is_enum<OverarchingType>::value)
		static Serializer& CreatePrimarySerializer(OverarchingType type, Args... cargs);


		//template<class Serializer, class OverarchingType, class... Args>
		//static Serializer& CreatePrimarySerializerIncmpl(OverarchingType type, Args... cargs)
		//{
		//	return CreatePrimarySerializer<Serializer>, type, cargs...);
		//}
	};


	

	//The type of entry should be what each is. So for a map it pairs. And it would handle be using the
	// first as the indexer, and the second as the value.
	//DEPRECATED
	/*
	template<class Entry, void(*REPLACE_ENTRY_HANDLE)(Entry&, SerialArgument&) = nullptr>
	class SerialConstructor : public SerializationHandler
	{
		//This is a friend class to the serializer thingy mabob. Perhaps not in the technical sense, more like
		// it functions as a helper. When an object can't be serialized as is, this object handles the contruction.


	public:
		//This is the function that's base used, handle entry is used after that, OR handle iterator
		// is used if this is a serialIterator
		//virtual void HandleSerialize(SerialArgument& serializer, bool& success) = 0;

		//Note, even in the failure to do anything this thing must still put something so in being brought back up
		// it doesn't happen out of order.
		//If we are serializing

	public://might be hidden.
		//This function should have a bool out, that way I don't HAVE to say it succeeded,
		// but I can say something didn't succeed, and effectively toss the entire entry
		virtual void HandleEntry(Entry& entry, SerialArgument& serializer, bool& success) = 0;
	};
	//*/
}