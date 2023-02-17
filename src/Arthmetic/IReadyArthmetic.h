#pragma once

#include "Arthmetic/ArthmeticObject.h"
#include "Arthmetic/TargetType.h"


namespace Arthmetic
{
	class RoutineArgument;
	class IReadyArthmetic;

	//I need a base class that can store some of the stuff I need that is virtual on the part of ArthmeticObject,
	// The general idea behind this would be all helper interfaces can basically inherit virtually from this sort of handshake
	// class.

	//inline std::map<std::string, IReadyArthmetic*> propertyMap;//MOVEME

	//Might need to make this inherit virtually.
	struct IReadyArthmetic : public virtual IObject//public ArthmeticObject//
	{
		//Might make this a void version.
		virtual float Run(Target target) = 0;

		float RunImpl(Target target)
		{
			//if (IsValid() == false) {
			//	return 0;
			//}

			return Run(target);
		}

#pragma region QOL
		void RunImpl(RoutineArgument* arg, float& r, size_t&);
		//WHY THE FUCK DOES THIS HAVE A LINKER ERROR
		float RunImpl(RoutineArgument* arg);

#pragma endregion
	};
}