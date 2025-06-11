
#include <stddef.h>

#include <toml++/toml.h>

#include "ActorValueExtendedList.h"
#include "EventSingleton.h"
#include "Hooks.h"

#include "ValueAliasHandler.h"

#include "Serialization/SerializationTypePlayground.h"

#include "ExtraValue.h"

#include "FileParser.h"

#include "LegacyFunctions.h"

using namespace RE::BSScript;
using namespace AVG;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;



namespace {
    void InitializeLogging() {
        auto path = log_directory();
        if (!path) {
            report_and_fail("Unable to lookup SKSE logs directory.");
        }
        *path /= PluginDeclaration::GetSingleton()->GetName();
        *path += L".log";

        std::shared_ptr<spdlog::logger> log;
        if (IsDebuggerPresent()) {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        } else {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
        }
        
        
#ifndef NDEBUG
        const auto level = spdlog::level::trace;
#else
		const auto level = GetKeyState(VK_RCONTROL) & 0x800 ? spdlog::level::debug : spdlog::level::info;
#endif

        log->set_level(level);
        log->flush_on(level);

        spdlog::set_default_logger(std::move(log));
        //spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
        spdlog::set_pattern("%s(%#): [%^%l%$] %v"s);
    }

    
    void InitializeSerialization() {
        //log::trace("Initializing cosave serialization...");
        //auto* serde = GetSerializationInterface();
        //serde->SetUniqueID(_byteswap_ulong('AVGN'));
        //serde->SetSaveCallback(Sample::HitCounterManager::OnGameSaved);
        //serde->SetRevertCallback(Sample::HitCounterManager::OnRevert);
        //serde->SetLoadCallback(Sample::HitCounterManager::OnGameLoaded);
        //log::trace("Cosave serialization initialized.");

		RGL::MainSerializer::Initialize('AVG');
    }


	void InitializeHooking()
	{
		auto size = ExtraValueInfo::GetCount();

		//ActorValueExtendedList::Create(257 - total);
		ActorValueExtendedList::Create(size + 1);
	}

	//SHAMELESSLY Ripped directly from po3's clib utils
	inline std::vector<std::string> get_configs(std::string_view a_folder, std::string_view a_suffix, std::string_view a_extension = ".ini"sv)
	{
		std::vector<std::string> configs{};

		for (const auto iterator = std::filesystem::directory_iterator(a_folder); const auto & entry : iterator) {
			if (entry.exists()) {
				if (const auto& path = entry.path(); !path.empty() && path.extension() == a_extension) {
					if (const auto& fileName = entry.path().string(); fileName.rfind(a_suffix) != std::string::npos) {
						configs.push_back(fileName);
					}
				}
			}
		}

		std::ranges::sort(configs);

		return configs;
	}

    

	static bool LoadFile(const std::string a_path, bool is_legacy)
	{
		try {
            //I wonder if this closes.
			const auto table = toml::parse_file(a_path);

			if (is_legacy)
			{
				using EntryRef = std::pair< const toml::key*, const toml::node*>;

				std::vector<EntryRef> order{ table.size() };

				//for (auto& [key, entry] : table) {
				//	order.push_back(std::make_pair(&key, &entry));					
				//}

				std::transform(table.begin(), table.end(), order.begin(), [](auto&& pair)
					{
						return std::make_pair(&pair.first, &pair.second);
					});

				std::sort(order.begin(), order.end(), [](EntryRef& a, EntryRef& b) -> bool
					{
						switch (Hash<HashFlags::Insensitive>(a.first->data()))
						{
						case"Properties"_ih:
							return true;
						}

						//*
						auto test_a = a.second->as_table();
						auto test_b = b.second->as_table();

						if (!test_b)
						{
							return test_a;
						}
						else if (!test_a)
						{
							return false;
						}
						else
						{
							auto table_a = *test_a;
							auto table_b = *test_b;

							//constexpr std::string_view routine = "Routine";

							bool func_a = stricmp("Routine", table_a["type"].value_or("")) == 0;
							bool func_b = stricmp("Routine", table_b["type"].value_or("")) == 0;


							if (!func_a)
							{
								return false;
							}
							else if (!test_b)
							{
								return func_a;
							}
						}

						return false;
						//*/
						if (auto test = b.second->as_table(); test)
						{
							auto& table = *test;

							std::string type = table["type"].value_or("Invalid");


							switch (RGL::Hash<RGL::HashFlags::Insensitive>(type))
							{
							case "Routine"_ih:
								return false;
							}
						}
						
						return true;
					});

				for (auto [key, entry] : order) {
					std::string name = key->data();  //This may not be null terminated, may want to switch over to string views at some point.
					logger::info("Starting: {}-------------", name);
					HandleFileInput(name, *entry, is_legacy);
				}
			}
			else {
				for (auto& [key, entry] : table) {
					std::string name = key.data();  //This may not be null terminated, may want to switch over to string views at some point.
					logger::info("Starting: {}-------------", name);
					HandleFileInput(name, entry, is_legacy);
				}
			}
			

		} catch (const toml::parse_error& e) {
			//For now, I'm just gonna take the L
			/*
			std::ostringstream ss;
			ss
				<< "Error parsing file \'" << *e.source().path
				<< "\':\n"
				<< e.description()
				<< "\n  (" << e.source().begin << ")\n";
			logger::error(fmt::runtime(ss.str()));
			//*/

			logger::error("Parsing failed: {}", e.description());
			return false;
		}

		return true;
	}

	
	void PersonalLoad()
	{
		std::set<std::string> ignoreList;

		constexpr std::string_view core_path = "Data/SKSE/Lexicon/Resources/ActorValueGenerator";



		
		std::vector<std::pair<std::string, std::string>> configs = LEX::SearchFiles(core_path, ".toml");

		auto pred = [](auto&& lhs, auto&& rhs) -> bool { return lhs > rhs; };

		//std::sort(std::begin(configs), std::end(configs), pred);

		//Should register itself. Note, I really would like this to be a function instead of this on the outside doing this.
	
		//std::string ev_name = "HitsTaken";
		//new AdaptiveValueInfo(ev_name);
		logger::info("Config Count: {}", configs.size());

		for (auto& [path, file_name] : configs){
			logger::info("Parsing: {}-------------", file_name);
			LoadFile(path + "/"+ file_name, false);

			file_name.insert(file_name.size() - 5, "_AVG");

			logger::info("Ignoring future {}", file_name);
		}


		//std::vector<std::string, std::string> configs = LEX::SearchFiles(core_path, "_AVG", ".toml");
		constexpr std::string_view legacy_path = "Data/SKSE/Plugins/ActorValueData";

		configs = LEX::SearchFiles(legacy_path, "_AVG.toml", ignoreList);

		logger::info("Legacy Count: {}", configs.size());

		//std::sort(std::begin(configs), std::end(configs), pred);

		for (auto& [path, file_name] : configs) {
			logger::info("Parsing: {}-------------", file_name);
			LoadFile(path + "/" + file_name, true);
		}


		logger::info("Finishing the manifest. . .");
		AVG::Legacy::FormulaRegister();
		ExtraValueInfo::FinishManifest();
		HandleIncludeLists();
		Hooks::Install();
		EventSingleton::Install();
	}




	/*
	struct DefaultClient : public LEX::ProjectClient
	{
		//Currently has an issue where failure is not descriptive if the location it failed in.

		void RecieveMessage(uint64_t severity, std::string_view message, ProjectClient* sender) override
		{
			//no reciever on this end.
		}

		bool HandleFormat(Script* script, std::string_view format, std::string_view name, std::string_view content) override
		{
			//By default has no formatter.
			return false;
		}


		virtual TypeOffset HandleExtraOffsetArgs(std::string_view category, std::string_view* data, size_t length) { return -1; }

		TypeOffset GetOffsetFromArgs(std::string_view category, std::string_view* data, size_t length) override final
		{
			return 
		}


		static DefaultClient* GetInstance()
		{
			//This should be ensured not to be called before initialize is called.

			if (Initializer::Finished() && !_client)
				_client = new DefaultClient;

			return _client;

		}

		static void SetInstance(DefaultClient* client)
		{
			if (!_client)
				_client = client;
		}

	private:
		inline static DefaultClient* _client = nullptr;
	};
	//*/


	





	void InitializeLexiconMessaging()
	{
		using LEX::LinkFlag;

		//If I can, I'd like to make an initialize script based around this.
		if (!LEX::LinkMessenger::instance->RegisterForLink([](LinkFlag flag) {


			switch (flag) {
				// Skyrim lifecycle events.
			case LinkFlag::Loaded: // Called after all plugins have finished running SKSEPlugin_Load.
				FormExtraValueHandler::Register();
				//ArithmeticAPI::RequestInterface();
				//ActorValueGeneratorAPI::RequestInterface();
				{

					logger::info("Starting...");
					LEX::IProject* avg = nullptr;

					if (LEX::ProjectManager::instance->CreateProject("ActorValueGenerator", nullptr, avg) != LEX::APIResult::Success)
					{
						logger::info("AVG has experienced failure");
					}

					commons = avg->GetCommons();

					if (avg) {
						//IProject* project, std::string_view name, std::string_view path, std::string_view content, IScript*& out, std::span<std::string_view> options = {}
						//if (LEX::ProjectManager::instance->CreateScript(avg, "__Legacy__", "", "", &legacy, std::vector<std::string_view>{"incremental"}) != LEX::APIResult::Success) {
						std::vector<std::string_view>incremental{ "incremental" };
						if (LEX::ProjectManager::instance->CreateScript(avg, "__Legacy__", "", "", legacy, incremental) != LEX::APIResult::Success) {
							logger::info("ETU* Legacy has failured to create");
						}
					}

					logger::info("Ending?... {} {}", !!avg, !!legacy);
				}
				break;


			case LinkFlag::Definition: // Called when all game data has been found.
				PersonalLoad();
				break;

			case LinkFlag::External: // All ESM/ESL/ESP plugins have loaded, main menu is now active.
				if (legacy->AppendContent("int test_legacy = 69;") == false)
				{
					logger::info("ETU* failure");
				}
				else {
					logger::info("ETU* legacy >> {}", LEX::Formula<int>::Run("test_legacy * 1", legacy, 420));
				}


				FormExtraValueHandler::Initialize();
				InitializeHooking();
				{
					//auto testForm = LEX::Formula<float(RE::Actor::*)()>::Create("30 + (HasKeyword(props::PlayerKeyword) * 20) + GetActorValue2('StrengthAdaptive', props::All)");

					//auto value = testForm(RE::PlayerCharacter::GetSingleton());

					commons = LEX::ProjectManager::instance->GetScriptFromPath("ActorValueGenerator::Commons");
					//legacy = LEX::ProjectManager::instance->GetScriptFromPath("ActorValueGenerator::Commons");

					logger::info("Project {}", LEX::Formula<float>::Run("GetPlayer().ProjectTest()", commons));



				}

				break;

			}
			})) {
			stl::report_and_fail("Unable to register Lexicon linkage messenger.");
			throw nullptr;
		}
	}


    void InitializeMessaging() {
        
		
		if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
            switch (message->type) {
                // Skyrim lifecycle events.
                case MessagingInterface::kPostLoad: // Called after all plugins have finished running SKSEPlugin_Load.
					InitializeLexiconMessaging();
					break;
				
            }
        })) {
            stl::report_and_fail("Unable to register message listener.");
        }
    }
}

/**
 * This if the main callback for initializing your SKSE plugin, called just before Skyrim runs its main function.
 *
 * <p>
 * This is your main entry point to your plugin, where you should initialize everything you need. Many things can't be
 * done yet here, since Skyrim has not initialized and the Windows loader lock is not released (so don't do any
 * multithreading). But you can register to listen for messages for later stages of Skyrim startup to perform such
 * tasks.
 * </p>
 */

//*

	//using namespace Arthmetic;





SKSEPluginLoad(const LoadInterface* skse) {
#ifdef _DEBUG
	//Need a way to only have this happen when holding down a key
	if (GetKeyState(VK_RCONTROL) & 0x800) {
		constexpr auto text1 = L"Request for debugger detected. If you wish to attach one and press Ok, do so now if not please press Cancel.";
		constexpr auto text2 = L"Debugger still not detected. If you wish to continue without one please press Cancel.";
		constexpr auto caption = L"Debugger Required";
		
		int input = 0;

		do
		{
			input = MessageBox(NULL, !input ? text1 : text2, caption, MB_OKCANCEL);
		} 
		while (!IsDebuggerPresent() && input != IDCANCEL);
	}
#endif
	
	InitializeLogging();

	auto plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
	
    log::info("{} {} is loading...", plugin->GetName(), version);
	


    Init(skse);
	
    InitializeMessaging();
	
    InitializeSerialization();
	
	
    log::info("{} has finished loading.", plugin->GetName());

	return true;
}




struct ExtraSkillForm
{
	std::unique_ptr<RE::ActorValue[]> data;

	ExtraSkillForm(size_t size)
	{
		//I don't know how to do this shit inside of the constructor decl

		//new RE::ActorValue[5]{;
		data = std::unique_ptr<RE::ActorValue[]>{ new RE::ActorValue[size] };

		for (int i = 0; i < size; i++)
		{
			data[i] = RE::ActorValue::kNone;
		}
	}
};

size_t GetExtraSkillLimit(RE::FormType form)
{
	//Basically, this determines the point where something needs to be considered an extra skill value, being unable to be stored properly
	
	//Correction, this isn't needed though. Doing this would ultimately just lead to more annoyances, so I think instead I should just let the values remain void
	// and move the true value elsewhere.
	switch (form)
	{
		case RE::FormType::NPC:
			break;
	}

	return 0;
}