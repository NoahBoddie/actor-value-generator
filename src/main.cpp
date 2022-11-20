
#include <stddef.h>

#include <toml++/toml.h>

#include "ActorValueExtendedList.h"
#include "EventSingleton.h"
#include "Hooks.h"

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
        const auto level = spdlog::level::info;
#endif


        log->set_level(level);
        log->flush_on(level);

        spdlog::set_default_logger(std::move(log));
        //spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
        spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);
    }

    
    void InitializeSerialization() {
        //log::trace("Initializing cosave serialization...");
        //auto* serde = GetSerializationInterface();
        //serde->SetUniqueID(_byteswap_ulong('AVGN'));
        //serde->SetSaveCallback(Sample::HitCounterManager::OnGameSaved);
        //serde->SetRevertCallback(Sample::HitCounterManager::OnRevert);
        //serde->SetLoadCallback(Sample::HitCounterManager::OnGameLoaded);
        //log::trace("Cosave serialization initialized.");
    }


    void InitializePapyrus() {
        log::trace("Initializing Papyrus binding...");
		
        
        return;
        
        /*
        if (GetPapyrusInterface()->Register(Sample::RegisterHitCounter)) {
            log::debug("Papyrus functions bound.");
        } else {
            stl::report_and_fail("Failure to register Papyrus bindings.");
        }
        //*/
    }

       //temporary, move later.
	union FloatOrInteger
	{
		float _float;
		RE::ActorValue _integer;
	};

	class BGSEntryPointFunctionDataTwoValue : public RE::BGSEntryPointFunctionData
	{
	public:
		inline static constexpr auto RTTI = RE::RTTI_BGSEntryPointFunctionDataTwoValue;

		~BGSEntryPointFunctionDataTwoValue() override;  // 00

		// override (BGSEntryPointFunctionData)
		FunctionType GetType() const override;               // 01 - { return kOneValue; }
		bool LoadFunctionData(RE::TESFile* a_mod) override;  // 02

		// members
		FloatOrInteger data1;  // 08 - DATA
		FloatOrInteger data2;  // 0C
	};
	static_assert(sizeof(BGSEntryPointFunctionDataTwoValue) == 0x10);

	void InitializeHooking()
	{
		RE::TESDataHandler* data_handler = RE::TESDataHandler::GetSingleton();

		if (data_handler) {
			//RE::TESGlobal* test_1 = data_handler->LookupForm<RE::TESGlobal>(0x000810, "AVG_CrashTest.esp");
			//RE::TESGlobal* test_2 = data_handler->LookupForm<RE::TESGlobal>(0x000801, "AVG_EditorTest.esp");
            //logger::info("test 1: {}, test 2: {}, equal: {}", test_1 ? test_1->formID : 0, test_2 ? test_2->formID : 0, test_1 == test_2);

            the_setting = data_handler->LookupForm<RE::EffectSetting>(0x000802, "AVG_CrashTest.esp");

			if (the_setting) {
				logger::info("effect set.");
				the_setting->data.primaryAV = static_cast<RE::ActorValue>(165);
				the_setting->data.secondaryAV = static_cast<RE::ActorValue>(165);
			}

			RE::BGSPerk* the_perk = data_handler->LookupForm<RE::BGSPerk>(0x000805, "AVG_CrashTest.esp");

			if (the_perk) {
				RE::BGSEntryPointPerkEntry* entry_point = skyrim_cast<RE::BGSEntryPointPerkEntry*>(the_perk->perkEntries[1]);

				if (!entry_point) {
					logger::error("fail A");
					goto end;
				}

				if (!entry_point->functionData) {
					logger::error("fail B");
					goto end;
				}

				BGSEntryPointFunctionDataTwoValue* function_data = skyrim_cast<BGSEntryPointFunctionDataTwoValue*>(entry_point->functionData);

				if (!function_data) {
					logger::error("fail C");
					goto end;
				}
				function_data->data1._float = 165;
				logger::info("first entry: {}, second entry {}", function_data->data1._float, function_data->data2._float);
			}
		}
        end:

			uint32_t total = static_cast<uint32_t>(RE::ActorValue::kTotal);

			ActorValueExtendedList::Create(257 - total);

			//log::trace("Initializing trampoline...");
			//auto& trampoline = GetTrampoline();
			//trampoline.create(64);
			//log::trace("Trampoline initialized.");

			//Sample::InitializeHook(trampoline);
	}

    
    //should use "Data/SKSE/Plugins/ActorValueData/TestEV_AVG.toml" as it's configuration.
	static bool temp_LoadExtraValue(const std::string a_path)
	{
		try {
            //I wonder if this closes.
			const auto table = toml::parse_file(a_path);

			for (auto& [key, entry] : table) {
				std::string ev_name = key.data();  //This may not be null terminated, may want to switch over to string views at some point.
				int type_name = static_cast<int>(entry.type());
                logger::info("{} is type {}", ev_name, type_name);
                
                if (entry.is_table() == false) {
					logger::error("Not table {}", ev_name);
                    continue;  //We don't currently process anything but tables.
				}
				std::string type = table[key]["type"].value_or("INVALID");//temporary, replace with better code plz

				if (type == "INVALID") {
					logger::error("No string for type {}", ev_name);
					continue;
				}


				//Would like a hash switch here if string
				if (type == "Adaptive") {
					auto recovery_data = table[key]["recovery"];

                    float rate;
					float delay;
                    bool no_rec = false;
					
                    if (recovery_data.is_table() == false) {//!!recovery_data, this is a double negative, not an operator.
						logger::info("No rec data");
                        no_rec = true;
					}
                    else {
						rate = recovery_data["rate"].value_or(0.f);
						delay = recovery_data["delay"].value_or(0.f);

                        logger::info("Rec data rate: {}, delay: {}", rate, delay);
                        no_rec = rate == 0;
                    }

                    if (no_rec)
						new AdaptiveValueInfo(ev_name);
					else
						new AdaptiveValueInfo(ev_name, delay, rate);
				}
                else
                {
					logger::error("Not adaptive {}", ev_name);
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
			logger::error("parse fail");
			return false;
		}

		return true;
	}

void PersonalLoad()
{
    //Should register itself. Note, I really would like this to be a function instead of this on the outside doing this.
	
    //std::string ev_name = "HitsTaken";
    //new AdaptiveValueInfo(ev_name);

    temp_LoadExtraValue("Data/SKSE/Plugins/ActorValueData/TestEV_AVG.toml");

    ExtraValueInfo::FinishManifest();
	Hooks::Install();
	EventSingleton::Install();
}



    /**
     * Register to listen for messages.
     *
     * <p>
     * SKSE has a messaging system to allow for loosely coupled messaging. This means you don't need to know about or
     * link with a message sender to receive their messages. SKSE itself will send messages for common Skyrim lifecycle
     * events, such as when SKSE plugins are done loading, or when all ESP plugins are loaded.
     * </p>
     *
     * <p>
     * Here we register a listener for SKSE itself (because we have not specified a message source). Plugins can send
     * their own messages that other plugins can listen for as well, although that is not demonstrated in this example
     * and is not common.
     * </p>
     *
     * <p>
     * The data included in the message is provided as only a void pointer. It's type depends entirely on the type of
     * message, and some messages have no data (<code>dataLen</code> will be zero).
     * </p>
     */
    void InitializeMessaging() {
        if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
            switch (message->type) {
                // Skyrim lifecycle events.
                case MessagingInterface::kPostLoad: // Called after all plugins have finished running SKSEPlugin_Load.
				    PersonalLoad();
                    // It is now safe to do multithreaded operations, or operations against other plugins.
                case MessagingInterface::kPostPostLoad: // Called after all kPostLoad message handlers have run.
                case MessagingInterface::kInputLoaded: // Called when all game data has been found.
                    break;
                case MessagingInterface::kDataLoaded: // All ESM/ESL/ESP plugins have loaded, main menu is now active.
                    // It is now safe to access form data.
                    InitializeHooking();
                    break;

                // Skyrim game events.
                case MessagingInterface::kNewGame: // Player starts a new game from main menu.
                case MessagingInterface::kPreLoadGame: // Player selected a game to load, but it hasn't loaded yet.
                    // Data will be the name of the loaded save.
                case MessagingInterface::kPostLoadGame: // Player's selected save game has finished loading.
                    // Data will be a boolean indicating whether the load was successful.
                case MessagingInterface::kSaveGame: // The player has saved a game.
                    // Data will be the save name.
                case MessagingInterface::kDeleteGame: // The player deleted a saved game from within the load menu.
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



SKSEPluginLoad(const LoadInterface* skse) {
    InitializeLogging();

    auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);


    Init(skse);
    InitializeMessaging();
    InitializeSerialization();
    InitializePapyrus();

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}
