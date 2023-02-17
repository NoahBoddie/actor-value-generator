#pragma once

#include "RoguesGallery/Utility.h"

namespace RGL
{
	//Allow equals copying, I think what I want to be able to have tempflags escape scope via references.
	//Allow it all the things a normal object does, copy constructors, copy assignments, and default constructors specifically.
	// This way, I can condition the creation or initialization of a temp flag.



	template<class FocusType, class EntryType>
	class TempFlag;

	template<class T, class A, class B>
	concept DerivedTempFlag = std::is_base_of<TempFlag<A, B>, T>::value;

#define temp_flag_invalid validity_check = false; return;


	//Make a branch of this where it can accept references. For not much value though, honestly speaking.
	template<class FocusType, class EntryType>
	class TempFlag
	{
#define init_flag 0xF0
#define valid_flag 0x0F
	public:
		using Focus = FocusType;
		using Entry = EntryType;
		using TempFlagType = TempFlag<FocusType, EntryType>;

	protected:
		std::uint8_t _flagValidation;
		//protected:
		FocusType*	_focusPtr = nullptr;
		EntryType	_setValue = DefaultType::value;
		EntryType	_originalValue = DefaultType::value;
	
	private:
		void Validate() { _flagValidation |= IsInitialized() ? _flagValidation : valid_flag; }
		void Invalidate() { _flagValidation &= ~valid_flag; }
		void Initialize() { _flagValidation |= init_flag; }

	public:
		virtual void FlagStart(bool& validity_check) = 0;
		virtual void FlagFinish() = 0;

		constexpr bool IsValid() { return _flagValidation & valid_flag; }
		constexpr bool IsInitialized() { return _flagValidation & init_flag; }

		constexpr FocusType* focus() { return _focusPtr; }
		constexpr EntryType setValue() { return _setValue; }
		constexpr EntryType originalValue() { return _originalValue; }
		//If this causes issues, we can use a sign bit instead.

	private:
		//These 2 are share functions.
		__declspec(noinline) void Init(FocusType* focus_ptr, EntryType set_value)
		{
			if (!focus_ptr) {
				logger::error("error with focus pointer, didn't work dead");
				return;
			}
			_focusPtr = focus_ptr;
			_setValue = set_value;

			if (IsInitialized() == true)
				return;//Already initialized

			bool validity_check = true;

			FlagStart(validity_check);

			if (validity_check) {
				Validate();
			}

			Initialize();
		}

		void Delete()
		{
			if (IsValid() == false)
				return;

			FlagFinish();
		}

		void Transfer(TempFlagType&& other) {
			std::memcpy(this, &other, sizeof other);
			OnTransfer(other);
			other.Invalidate();//Ownership has transfered.
		};

		//In case other data needs transfering.
		virtual void OnTransfer(TempFlagType& other) {}

	public:
		TempFlag() = default;


		//This is copy constructor, we want THIS to be defined. // func(temp_flag)
		
		
		/*//I wish to disable these, and put it on the container to deal with this.
		TempFlag(TempFlagType& other) {
			Transfer(other);
		}
		
		//This is a copy assignment, we likely don't want this either, but it might be used elsewhere. //Foo foo; foo = bar;
		TempFlagType& operator=(TempFlagType other) {
			Transfer(other);
		}

		//This is the same as below, a copy assignment, we likely don't want this.
		TempFlagType& operator=(TempFlagType& a_rhs) {
			Transfer(other);
		}

		//I believe this is the explicit move assignment, we want to delete this
		TempFlagType& operator=(TempFlagType&& other) {
			Transfer(other);
		}
		//*/
		template<class DerivedFlag>
		friend class FlagContainer;
	};

	template<class Type>
	inline void Variable_FlagStart(bool& is_valid, Type& focus, Type& original_value, Type set_value)
	{
		//original_value = set_value;
		original_value = focus;
		focus = set_value;
	}
	template<class Type>
	inline void Variable_FlagFinish(Type& focus, Type& original_value, Type set_value)
	{
		logger::info("origin {}", (uint64_t)original_value);
		focus = original_value;
	}

	template <class FocusType, class EntryType>
	using FlagStartDelegate = void(bool&, FocusType&, EntryType&, EntryType);

	template <class FocusType, class EntryType>
	using FlagFinishDelegate = void(FocusType&, EntryType&, EntryType);

	//Make a branch of this where it can accept references. For not much value though, honestly speaking.
	template<class FocusType, class EntryType, FlagStartDelegate<FocusType, EntryType>* Flag_Start, FlagFinishDelegate<FocusType, EntryType>* Flag_Finish>
	class TemporalFlag
	{
		static_assert(Flag_Start != nullptr, "Flag Start cannot be null.");
		static_assert(Flag_Finish != nullptr, "Flag Finish cannot be null.");

	private:
		static inline constexpr short unsigned int InitFlag() { return 0xF0; }
		static inline constexpr short unsigned int ValidFlag() { return 0x0F; }

#define init_flag 0xF0
#define valid_flag 0x0F
	public:
		using Focus = FocusType;
		using Entry = EntryType;
		using TempFlagType = TemporalFlag<FocusType, EntryType, Flag_Start, Flag_Finish>;
		

	protected:
		std::uint8_t _flagValidation;
		//protected:
		FocusType* _focusPtr = nullptr;
		EntryType	_setValue{};
		EntryType	_originalValue{};

	private:
		void Validate() { _flagValidation |= IsInitialized() ? _flagValidation : valid_flag; }
		void Invalidate() { _flagValidation &= ~valid_flag; }
		void Initialize() { _flagValidation |= init_flag; }

	public:
		__declspec(noinline) void FlagStart(bool& validity_check) { Flag_Start(validity_check, *_focusPtr, _originalValue, _setValue); }
		__declspec(noinline) void FlagFinish() { Flag_Finish(*_focusPtr, _originalValue, _setValue); }

		constexpr bool IsValid() { return _flagValidation & valid_flag; }
		constexpr bool IsInitialized() { return _flagValidation & init_flag; }

		constexpr FocusType* FocusPtr() { return _focusPtr; }
		constexpr EntryType SetValue() { return _setValue; }
		constexpr EntryType OriginalValue() { return _originalValue; }
		//If this causes issues, we can use a sign bit instead.

	private:
		//These 2 are share functions.
		__declspec(noinline) void Init(FocusType* focus_ptr, EntryType set_value)
		{
			if (!focus_ptr) {
				logger::error("error with focus pointer, didn't work dead");
				return;
			}
			_focusPtr = focus_ptr;
			_setValue = set_value;

			if (IsInitialized() == true)
				return;//Already initialized

			bool validity_check = true;

			FlagStart(validity_check);

			if (validity_check) {
				Validate();
			}

			Initialize();
		}

		void Delete()
		{
			if (IsValid() == false)
				return;

			FlagFinish();
		}

		void Transfer(TempFlagType& other) {
			
			std::memcpy(this, &other, sizeof other);
			//OnTransfer(other);//Should it need something, I'll give it a different function. No vtables.
			other.Invalidate();//Ownership has transfered.
		};

	public:
		TemporalFlag() = default;



		void operator()(Focus* focus_ptr, Entry set_value) { Init(focus_ptr, set_value); }
		void operator()(Focus& focus_ref, Entry set_value) { Init(&focus_ref, set_value); }


		TemporalFlag(TempFlagType& other) { Transfer(other); }
		TemporalFlag(TempFlagType&& other) { Transfer(other); }
		TemporalFlag(Focus* focus_ptr, Entry set_value) { Init(focus_ptr, set_value); }
		TemporalFlag(Focus& focus_ref, Entry set_value) { Init(&focus_ref, set_value); }
		~TemporalFlag() { if (IsValid() == true) FlagFinish(); }

		//FlagContainer& operator=(FlagContainer a_rhs) { _tempFlag.Transfer(a_rhs._tempFlag); }
		TempFlagType& operator=(TempFlagType& a_rhs) { Transfer(a_rhs); }
		TempFlagType& operator=(TempFlagType&& a_rhs) { Transfer(a_rhs); }

		operator bool() { return IsValid(); }
	};



	template<class DerivedFlag>
	class FlagContainer final//THis is the chief holder, and I do not want any more versions.
	{
		//using TempFlagType = TempFlag<DerivedFlag::Focus, DerivedFlag::Entry>;
		using ContainerType = FlagContainer<DerivedFlag>;

	private:
		DerivedFlag _tempFlag;

		static_assert(std::is_base_of<TempFlag<DerivedFlag::Focus, DerivedFlag::Entry>, DerivedFlag>::value,
			"Type is not derived from a Temporary Flag.");//Constraint would be better.
	public:

		void operator()(DerivedFlag::Focus* focus_ptr, DerivedFlag::Entry set_value) { _tempFlag.Init(focus_ptr, set_value); }
		void operator()(DerivedFlag::Focus& focus_ref, DerivedFlag::Entry set_value) { _tempFlag.Init(&focus_ref, set_value); }



		FlagContainer(DerivedFlag::Focus* focus_ptr, DerivedFlag::Entry set_value) { _tempFlag.Init(focus_ptr, set_value); }
		FlagContainer(DerivedFlag::Focus& focus_ref, DerivedFlag::Entry set_value) { _tempFlag.Init(&focus_ref, set_value); }
		~FlagContainer() { _tempFlag.Delete(); }
		
		FlagContainer() = default;
		FlagContainer(ContainerType& other) { _tempFlag.Transfer(other._tempFlag); }
		//FlagContainer& operator=(FlagContainer a_rhs) { _tempFlag.Transfer(a_rhs._tempFlag); }
		ContainerType& operator=(ContainerType& a_rhs) { _tempFlag.Transfer(a_rhs._tempFlag); }
		ContainerType& operator=(ContainerType&& a_rhs) { _tempFlag.Transfer(a_rhs._tempFlag); }

		operator bool() { return _tempFlag.IsValid(); }

		//Don't think I need to do this, seeing as there is no feasible constructor for it. Pag.
		//static void* operator new (size_t) = delete;
		//static void* operator new[](size_t) = delete;
		//static void  operator delete  (void*) = delete;
		//static void  operator delete[](void*) = delete;

		constexpr DerivedFlag* operator->() { return &_tempFlag; }
	};



	//Given the functionality of these templates, you could just as easily perform the functionality of these with
	// insertable function types. But I fear tampering with these
	template<RE::ActorValue Actor_Value>
	class ActorValueFlag : public TempFlag<RE::Actor, float>
	{
	public:
		using TempFlag<RE::Actor, float>::TempFlag;
		//private:
		//float _originalValue;

		//With stuff like this it should measure the difference between them.
		//main reason I don't want to use a specific constructor for this is because
		// I want the set up of the last one to be mostly final.
		void FlagStart(bool& validity_check) override
		{
			this->_originalValue = _focusPtr->AsActorValueOwner()->GetActorValue(Actor_Value);
			this->_focusPtr->SetActorValue(Actor_Value, this->_setValue);
		}


		void FlagFinish() override
		{
			this->_focusPtr->SetActorValue(Actor_Value, this->_originalValue);
		}
	};




	//should be made to be a primitive type or integral
	template<class Type>//Is used in situations where the tpe cannot be owned by a flag, like an actor ptr
	class VariableFlag : public TempFlag<Type, Type>//std::conditional_t<std::is_pointer_v<Type>, Type*, Type>>
	{
	public:
		//using TempFlag<Type, Type>::TempFlag;
		//Type _originalValue;


		void FlagStart(bool& validity_check) override
		{
			/*
			if (!__super::_focusPtr){// || !(*this->_focusPtr)) {
				RE::DebugMessageBox("NO FOCUS");
				validity_check = false;
				return;
			}
			//*/
			auto& focus = *__super::_focusPtr;

			//decltype(__super::_originalValue) value = *__super::_focusPtr;
			//logger::info("{}, {}", typeid(focus).name(), (uint64_t)focus);
			//__super::_originalValue = *__super::_focusPtr;
			//*this->_focusPtr = this->_setValue;

			__super::_originalValue = focus;
			focus = __super::_setValue;			
			//RE::DebugNotification(std::format("{} setStart", *this->focus()).c_str(), 0, false);
		}


		void FlagFinish() override
		{
			auto& focus = *__super::_focusPtr;
			//*this->_focusPtr = this->_originalValue;
			//focus = nullptr;
			focus = __super::_originalValue;
			//RE::DebugNotification(std::format("{} setFinish", *this->focus()).c_str(), 0, false);
		}
	};


	//Namespace used for the short names of these various flag types. Move els
	namespace TemporaryFlag
	{
		//template<class Type>
		//using Variable = FlagContainer<VariableFlag<Type>>;

		template<class Type>
		using Variable = TemporalFlag<Type, Type, Variable_FlagStart, Variable_FlagFinish>;

		template<RE::ActorValue Actor_Value>
		using ActorValue = FlagContainer<ActorValueFlag<Actor_Value>>;

	}
}