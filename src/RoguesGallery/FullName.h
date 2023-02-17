#pragma once

namespace NEL
{
	using NameHash = uint64_t;

	struct FullName
	{
		//I would actually say instead of this, maybe use one whole string and have
		// a divider. Quick and dirty for now I guess.
		std::string scopeName{};
		std::string shortName{};

		NameHash Hash(bool include_scope)
		{
			//Real rough, but eh

			uint8_t hash_bytes[8]{};

			hash_bytes[0] = scopeName[0];
			hash_bytes[1] = scopeName[1];
			hash_bytes[2] = shortName[0];
			hash_bytes[3] = shortName[1];

			std::string comp_name = scope_name + short_name;

			auto name_hash = hash(comp_name);
			
			std::memcpy(&hash_bytes[4], &name_hash, 4);//I forget if it's the other way or not.
			
			return reinterpret_cast<uint64_t>(hash_bytes);
		}

		std::string print()
		{
			if (scopeName == "")
				return "<Loose::>" + shortName;

			return scopeName + "::" + shortName;
		}

		constexpr operator bool()
		{
			//scope name isn't so important
			return shortName != "";
		}
	};
}


