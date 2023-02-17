#pragma once

#include "Serialization/SerialIterator.h"
#include "Serialization/SerializingWrapper.h"

namespace RGL
{
	//Both of these should come with the serializable version at some point.
	template <class WrapType>
	using WrapperHandle = void(*)(WrapType&, SerialArgument&, bool&);

	//SerializableList object. Self explanatory.
	template<class Entry>
	class SerializableList : public SerialIterator<std::list<Entry>, Entry>
	{
		//std::list<Entry> _list;

	public:
		void EmplaceEntry(Entry& entry) override
		{
			this->_iteratable.push_back(entry);
		}

		void HandleEntry(Entry& entry, SerialArgument& serializer, bool& success) override
		{
			serializer.Serialize(entry);//This much should be all that's necessary.
			//note, to the above though, if this where a map, I would serialize the left, the the right
			// which in deserialization would deserialize to the left part, then the right part.




			//These objects will not do the serializing themselves, instead, they use the serializer and use serialize. THIS WAY,
			// the specific objects will serialize as properly.
			// Issue is, how do I get that back?

			//Map, length
			// key,
			// list, length
			// ...
			// 
			//The question is effectively, how do we get back lists, and how do we get back nested types, like lists in lists or something like that.
			//The point is, I've been addressing the serialization of something like a dictionary as a single list in a straight line

			//Not to mention, this object is incapable if of even handling entries for loading because the for each would be empty.
			// I could change how the iteratable is handled, something like that. Rather, the idea when an iteratable object is detected
			//  I just have a function forcibly turn it into a vector (I'd turn it into an array but then no for looping) Maybe use std::array? prob not.
			//  Then once it's done like that, I iterate over it?
			//  NO! I can stick it back into the iterating object. But it will need a different function, that's much more tailored.
			// <!> This function will need to resize, and/or will need to place objects in their either key slots, or in their indexed slots.
			// Indexed slots aren't hard but making it compatible with mapped slots will be a pain.
			// In terms of how to store lists, it will have to be relavent value


			//Utility::DebugNotification(std::format("Hit {}", entry).c_str());
		}
	};


	//SerialVector, a serialization handled vector.
	template<class Entry, class SerializeClass = DefaultSerialize>
	class SerialVector : public SerialIterator<std::vector<Entry>, Entry, SerializeClass>
	{
		//std::list<Entry> _list;

	public:
		void EmplaceEntry(Entry& entry) override
		{
			this->_iteratable.push_back(entry);
		}

		void HandleEntry(Entry& entry, SerialArgument& serializer, bool& success) override
		{
			if constexpr (std::is_same_v<SerializeClass, DefaultSerialize>) {
				logger::debug("(de)serializing {}", typeid(Entry).name());
				serializer.Serialize(entry);
			}
		}

		Entry& operator[](std::size_t pos)
		{
			auto& _vector = this->get();
			return _vector[pos];
		}
		
		const Entry& operator[](std::size_t pos) const
		{
			auto& _vector = this->get();
			return _vector[pos];
		}
		

		SerialVector() = default;

		SerialVector(std::size_t count)
		{
			this->_iteratable = std::vector<Entry>(count);
		}
	};


}