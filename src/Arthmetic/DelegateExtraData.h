#pragma once

namespace Arthmetic
{

	struct DelegateExtraData : public ArthmeticExtraData
	{
		DelegateType delegateType{ DelegateType::None };

		FullName delegateName;

		void ProcessArthmetic(Arthmetic* arth_obj) override
		{
			//Note, this is inherently sloppy as shit. Shouldn't be used verbatum

			switch (delegateType) {
			case DelegateType::Routine:
				RoutineDelegate* rout_del = dynamic_cast<RoutineDelegate*>(arth_obj);
				rout_del->_callback = routineMap[delegateName.Hash()];

				if (!rout_del->_callback)
					logger::error("No callback was found for routine at {} for {}", 0, 1);  //Should use delegate name, the owner.
				break;

			case DelegateType::Function:
				{
					FunctionDelegate* func_del = dynamic_cast<FunctionDelegate*>(arth_obj);
					func_del->_callback = functionMap[delegateName.Hash()];

					if (!func_del->_callback)
						logger::error("No callback was found for function at {} for {}", 0, 1);
				}
				break;
			}
		}
	};

}