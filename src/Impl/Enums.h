#pragma once

namespace AVG
{
	ENUM(PrimaryRecordType, uint8_t)
	{
		//I know something gets serialized here, I just don't remember for the fuck of me what the fuck it was.
		ExtraValueInfo,
		ExtraValueStorage,
		PlayerStorage,
	};

}