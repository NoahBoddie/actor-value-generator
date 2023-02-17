#pragma once

namespace Arthmetic
{
	//This class here won't need to include arthmetic object, just declare it
	// This is because ArthObject needs to include this, and even if I resolve that, it gets rather messy, so I'm going to opt out of that.
	struct ArthmeticObject;

	struct ExtraData;

	class ExtraDataQueue;



	


	struct ExtraData
	{
	private:
		template <std::derived_from<ExtraData> DerivedData, class... ExtraDataArgs>
		static void Create(ExtraDataArgs... args)
		{
		}

		//I like the concept of having the queue owner here, that way if failure happens I can make it so the first object
		// can store important information and failures and such.
		//At the very end, I would like that information to remain if things have names and such. Doooooooon't really think I'll care
		// too much about all of the extra data, so maybe just point to some header piece of extra data yeah?
	protected:
		ExtraDataQueue* queue_owner;


	public:
		//I'll have no type function here, because I don't think I have need of anything else but delegates doing this.
		virtual void Process(ArthmeticObject* obj, bool& result) = 0;
	};




	//Use unqiue ptrs please.
	class ExtraDataQueue
	{
		//I can switch the lot of this around so it doesn't have to seperate it's shit like this.
		// Also, what this really wants is a stack. So maybe the list will stay
	public:
		std::list<std::pair<std::unique_ptr<ExtraData>, type_info*>> data_list{};

		template <class Type>
		bool HasData()
		{
			type_info* info = &typeid(Type);
			return std::find_if(data_list.begin(), data_list.end(), [=](auto& it) {return it.second == info; }) != data_list.end();
		}

		template <class Type>
		void AddData(Type* data)
		{
			if (HasData<Type>() == true)
				return;

			data_list.push_back(std::make_pair(std::make_unique(data), &typeid(Type));

			data->queue_owner = this;
		}

		std::unique_ptr<ExtraData> Pop()
		{
			if (data_list.empty() == true)
				return std::unique_ptr<ExtraData>(nullptr);
			
			std::unique_ptr<ExtraData> extra_data = std::move(data_list.back().first);
			data_list.pop_back();

			return extra_data;

		}
	};



	//This is basically to resolve some ambiguity later.
	using ExtraDataBase = ExtraData;

	struct ExtraDataHandler : public std::unordered_map<ArthmeticObject*, ExtraDataQueue>
	{
		using std::unordered_map<ArthmeticObject*, ExtraDataQueue>::unordered_map;
		

		static ExtraDataHandler* GetSingleton()
		{
			static ExtraDataHandler* singleton = new ExtraDataHandler();

			return singleton;
		}


		static bool GetSingleton(ExtraDataHandler*& out_singleton)
		{
			out_singleton = GetSingleton();
			return out_singleton;
		}


		static void Destroy()
		{
			auto singleton = GetSingleton();
			delete singleton;
		}

		bool HasObject(ArthmeticObject* obj)
		{
			return find(obj) != cend();

		}

		static void Finalize()
		{
			auto* singleton = GetSingleton();

			if (!singleton){
				ARTHMETIC_LOGGER(warn, "ExtraDataHandler already resolved.");
				return;
			}

			for (auto& [object, queue] : *singleton)
			{
				bool success = true;

				//I'd actually like to progressively pop off the back for this.

				auto begin = queue.data_list.rbegin();

				while (success && queue.data_list.rend() != begin)
				{
					std::unique_ptr<ExtraData>& extra_data = begin->first;

					extra_data->Process(object, success);

					begin++;
				}

				//while (queue.data_list.empty() == false)
				//	std::unique_ptr<ExtraData> queue.Pop
			}
		}

		//void Push(ArthmeticObject* arthmetic_object, ExtraData* data)

	};

	//This is the last extra data in queue, or rather the first in. The use of this will validate the use of the arth object.
	// Note, this should be moved to ArthObject.
	

	//struct ExtraDataVistor{};//Not sure the purpose of.

	//Extra data should be made within the load from view function, what will happen is all Arthmetic objects have the ability
	// to fire a function called push extra data, that will push an extra data onto a map, associated with an ArthmeticObject.

	//Then, when finalize is called, the pointer to the ArthmeticObject is given to the extra data, to handle it as it sees fit.
	// It will try to convert it to the object it thinks it is and should bail if not. But if it is, it will do what it needs to do.
	// For the case of delegates, it's plugging the the final names into where it's supposed to go.


	//Extra data will have a simple function create it, and itself will have a function interpret that string view.
	// Additional things it will need is a namespace however, how I determine that, will likely be confined to records and record data.
	//I believe I can hold the name space in records and have all objects have a pointer of their record.
	// It'll be annoying to set up, but I believe it to be the best idea.

	//Or better idea, At the end of all record strings is the namespace it's using. How do I seperate it however?

	//This becomes easier if the parse functions are a part of record.


	//But for now, namespaces are unimportant.
}

/*
//This is the basis of how embeded extra data works.
// If I can make some sort of requirement for how this bit works when there is no extra data, that would be cool.

//This can be a greate use to tell if member functions have been overriden, but only from templates. I'll use something like this as well
// to see if I should make an extra data. LoadFromView should also have some hand in this sort of thing.
// 
static_assert(&TestB::TestThis == &TestA::TestThis, "Blah");

struct TestA
{
	void TestThis() {}
	struct TestClass
	{

	};
};


struct TestB : public TestA
{
};


template <typename Type>
void TestFunction(Type* ptr)
{
	typename Type::TestClass test_obj;

}

int main()
{
	TestA a;
	TestB b;

	TestFunction(&a);

	TestFunction(&b);

	return 0;
}
//*/