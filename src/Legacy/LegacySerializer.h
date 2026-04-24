#pragma once

#include "SerialArgument.h"
#include "SerializationTypePlayground.h"
#include "SerializableList.h"
#include "SerializableMap.h"
#include "ExtraValueInfo.h"
#include "ExtraValueStorage.h"
#include "Tome/SerialManager.h"

namespace AVG::Legacy
{

#ifdef TURN_THE_FUCK_OFF

	//////////////////////////////////////////////////////////
	//Manifest
	//////////////////////////////////////////////////////////
	struct SerializeEntry
	{
		void operator()(ExtraValueInfo*& entry, SerialArgument& serializer, bool& success)
		{
			constexpr std::string_view invalid = "INVALID";

			SerialString name{};

			bool is_saving = serializer.IsSerializing();

			if (is_saving)
			{
				if (!entry || entry->IsFunctional() == true)
					name = std::string{ invalid };
				else
					name = std::string{ entry->GetName() };
			}


			serializer.Serialize(name);

			logger::debug("{} '{}'", is_saving ? "saving" : "loading", name);

			if (!is_saving && name != invalid) {
				entry = ExtraValueInfo::GetValueInfoByName(name);
			}
		}
	};

	static void SerializeList(SerialArgument& serializer, bool& success)
	{
		bool is_saving = serializer.IsSerializing();

		serializer.Serialize(is_saving ? _endTypeIndex[0] : _prevEndTypeIndex[0]);
		serializer.Serialize(is_saving ? _endTypeIndex[1] : _prevEndTypeIndex[1]);
		serializer.Serialize(is_saving ? _extraValueList : _previousExtraValueList);
	}

	static inline auto& list_serialize = SerializationHandler::CreatePrimarySerializer<SerialCallback>(PrimaryRecordType::ExtraValueInfo, SerializeList);

	static inline SerialVector<ExtraValueInfo*, SerializeEntry> _extraValueList;

	//////////////////////////////////////////////////////////
	//ExtraValues
	//////////////////////////////////////////////////////////


	struct SerializeClass
	{
		void operator()(std::pair<const SerialFormID, ExtraValueStorage*>& entry, SerialArgument& serializer, bool& success)
		{
			WriteLock guard{ accessLock };

			success = serializer.Serialize(entry.first);//Needs to be a particular type of object, serializable formID


			bool is_deserializing = serializer.IsDeserializing();

			if (success && is_deserializing == true) {
				RE::Actor* actor = nullptr;
				if (success) {
					actor = RE::TESForm::LookupByID<RE::Actor>(entry.first);

					if (!actor) {
						logger::error("Actor FormID {:08X} invalid, dumping.", static_cast<RE::FormID>(entry.first));
						success = false;
					}
					else {
						logger::info("Actor {}(FormID:{:08X}) successful, creating and deserializing.", actor->GetName(), static_cast<RE::FormID>(entry.first));
						entry.second = new ExtraValueStorage(actor, true);
					}
				}
				else
				{
					logger::error("Failure deserializing form ID.");
				}
			}

			//If the pointer is null or the success is false, it will dump the data, and return unsuccessful.
			success = serializer.DumpIfFailure(entry.second, success);

			if (success)
				logger::info("serialized: {:08X} at {:08X}", static_cast<RE::FormID>(entry.first), (uint64_t)entry.second);
			else
				logger::error("failed to de/serialize");
		}

		void operator()(std::map<SerialFormID, ExtraValueStorage*>&)
		{
			logger::debug("Reverting extra value storage");
			RemoveAllStorages();
		}
	};

	using EVStorageMap = SerializableMap<SerialFormID, ExtraValueStorage*, SerializeClass, SerializeClass>;

	inline static EVStorageMap& _valueTable = SerializationHandler::CreatePrimarySerializer<EVStorageMap>(PrimaryRecordType::ExtraValueStorage);


	//////////////////////////////////////////////////////////
	//Player Storage
	//////////////////////////////////////////////////////////

	PlayerStorage& PlayerStorage::_singleton = SerializationHandler::CreatePrimarySerializer<PlayerStorage>(PrimaryRecordType::PlayerStorage);

#endif
	
	template <typename T>
	struct PrimaryHandle : public SerializationHandler//, public IEncapsulateSerialDepthBase, public IChildDepthPrimaryBase
	{
		PrimaryHandle(T& handle) : target{ handle }
		{

		}
		T& target;

	public:
		//I actually think this could get away with just an ISerializer because I won't need a pointer to this.
		void HandleSerialize(SerialArgument& arg, bool& success) override
		{
			success = arg.Serialize(target);
		}

		void Revert() override
		{
		}
	};




	struct Handler
	{

		struct SerializeEntry
		{
			void operator()(ExtraValueInfo*& entry, SerialArgument& serializer, bool& success)
			{
				constexpr std::string_view invalid = "INVALID";

				SerialString name{};

				bool is_saving = serializer.IsSerializing();

				if (is_saving)
				{
					if (!entry || entry->IsFunctional() == true)
						name = std::string{ invalid };
					else
						name = std::string{ entry->GetName() };
				}


				serializer.Serialize(name);

				logger::debug("{} '{}'", is_saving ? "saving" : "loading", name);

				if (name != invalid) {
					entry = ExtraValueInfo::GetValueInfoByName(name);
				}
			}
		};


		using legManifestList = SerialVector<ExtraValueInfo*, SerializeEntry>;


		static void SerializeList(SerialArgument& serializer, bool& success)
		{
			legManifestList prevEVList;


			serializer.Serialize(ExtraValueInfo::_prevEndTypeIndex[0]);
			serializer.Serialize(ExtraValueInfo::_prevEndTypeIndex[1]);
			serializer.Serialize(prevEVList);

			ExtraValueInfo::_previousExtraValueList = prevEVList.get();
		}

		static void SerializePlayer(SerialArgument& serializer, bool& success)
		{
			legManifestList prevEVList;


			serializer.Serialize(ExtraValueInfo::_prevEndTypeIndex[0]);
			serializer.Serialize(ExtraValueInfo::_prevEndTypeIndex[1]);
			serializer.Serialize(prevEVList);

			ExtraValueInfo::_previousExtraValueList = prevEVList.get();
		}



		//Think I'll create these as an instance
		struct HandleEVStorage
		{
			
			void operator()(ExtraValueStorage& storage, SerialArgument& arg, bool& success)
			{
				arg.Serialize(storage._tickValue, 1 << 8 * 3);

				SerialVector<ExtraValueData> value_dump;
				SerialVector<std::pair<DataID, RegenData>> recover_dump;



				arg.Serialize(value_dump);
				arg.Serialize(recover_dump);

				//Should I algorithm for this?


				int vect_size = value_dump->size();
				int mani_size = ExtraValueInfo::GetManifestCount();

				logger::debug("LOADING: {} vs {}", vect_size, mani_size);

				//Removing a check for manifest size because then it will won't init
				// new actor values when loading an actor in.
				for (uint32_t i = 0; i < vect_size && i < mani_size; i++)
				{
					//If I can only show these once per cycle, that would be great.
					ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByManifest(i);


					if (!info) {
						if (i < mani_size)
							logger::warn("ExtraValueInfo at {} not found. Tossing data.", i);

						continue;
					}

					DataID id = info->GetDataID();

					if (id == ExtraValueInfo::FunctionalID) {
						logger::warn("ExtraValueInfo {}({}) is now functional. Tossing data.", info->GetName(), id);
						continue;
					}

					//If this is an npc storage && info is exclusive, continue
					logger::debug("Deserializing '{}'. Value: [{}/{}/{}/{}], {} -> {}",
						info->GetName(),
						value_dump[i]._base,
						value_dump[i]._evMods[0],
						value_dump[i]._evMods[1],
						value_dump[i]._evMods[2],
						i, id);

					storage._valueData[id] = value_dump[i];

				}


				for (uint32_t i = 0; i < recover_dump->size(); i++)
				{
					std::pair<DataID, RegenData>& rec_data = recover_dump[i];

					//If it's zero, who gives a shit, these don't have a default value of anything.
					if ((isnan(rec_data.second._pool) || rec_data.second._pool <= 0) && rec_data.second._time <= 0) {
						logger::debug("Regen data {} trivial, skipping deserialization.", i);
						continue;

					}

					ExtraValueInfo* info = ExtraValueInfo::GetValueInfoByManifest(rec_data.first);

					if (!info || info->GetRecoverInfo() == nullptr) {
						logger::warn("ExtraValueInfo at {} not found. Tossing regen data.", i);
						continue;
					}
					DataID id = info->GetDataID();

					if (id == ExtraValueInfo::FunctionalID) {
						logger::warn("ExtraValueInfo at {} has become functional. Tossing regen data.", i);
						continue;
					}


					auto predicate = [=](std::pair<DataID, RegenData> pair) {
						return pair.first == id;
						};

					//If this is an npc storage && info is exclusive, continue
					auto result = std::find_if(storage._recoveryData.begin(), storage._recoveryData.end(), predicate);
					;
					if (storage._recoveryData.end() != result) {
						logger::debug("Deserializing '{}' Recovery Data. : [{}/{}], {} -> {}",
							info->GetName(),
							rec_data.second._time,
							rec_data.second._pool,
							rec_data.first, id);

						result->second = rec_data.second;
					}
					else {
						logger::debug("'{}''s Recovery Data failed to Deserialize.", info->GetName());
					}
				}
			}
		};

		using legExtraValueStorage = SerializingWrapper<ExtraValueStorage, HandleEVStorage>;
		using legPlayerStorage = SerializingWrapper<PlayerStorage, HandleEVStorage>;

		struct HandleMap
		{
			void operator()(std::pair<const SerialFormID, legExtraValueStorage>& entry, SerialArgument& serializer, bool& success)
			{
				success = serializer.Serialize(entry.first);//Needs to be a particular type of object, serializable formID

				if (success) {
					RE::Actor* actor = nullptr;
					if (success) {
						actor = RE::TESForm::LookupByID<RE::Actor>(entry.first);

						if (!actor) {
							logger::error("Actor FormID {:08X} invalid, dumping.", static_cast<RE::FormID>(entry.first));
							success = false;
						}
						else {
							logger::info("Actor {}(FormID:{:08X}) successful, creating and deserializing.", actor->GetName(), static_cast<RE::FormID>(entry.first));
							entry.second = ExtraValueStorage(actor, true);
						}
					}
					else
					{
						logger::error("Failure deserializing form ID.");
					}
				}

				//If the pointer is null or the success is false, it will dump the data, and return unsuccessful.
				success = serializer.DumpIfFailure(entry.second, success);

				if (success)
					logger::info("serialized: {:08X}", static_cast<RE::FormID>(entry.first));
				else
					logger::error("failed to de/serialize");
			}

		};

		
		using legEVStorageMap = SerializableMap<SerialFormID, legExtraValueStorage, HandleMap>;
		
		static bool Initialize()
		{
			static bool _initialized = false;

			if (_initialized)
				return true;

			TOME::SerialManager::AddLegacySupport(LoadCallback);

			logger::info("Legacy ID {} initialized.", 'AVG');

			return true;
		}

		void Handle(SKSE::SerializationInterface* io, const TOME::SerialHeader& header)
		{
			SerialArgument arg(io, SerializingState::Deserializing, header.version);

			if (!header) {
				logger::warn("No values stored");
			}


			uint16_t id = 0;

			while (arg.IsFinished() == false)
			{
				if (arg.Serialize(id) == false) {
					logger::error("ID failed to deserialize properly.");
					continue;
				}


				switch ((PrimaryRecordType)id)
				{
				case PrimaryRecordType::ExtraValueInfo: {

					PrimaryHandle handler{ manifestCallback };
					arg.Serialize(handler);
					break;
				}
				case PrimaryRecordType::ExtraValueStorage: {
					PrimaryHandle handler{ storageMap };
					arg.Serialize(handler);
					ExtraValueStorage::_valueTable = reinterpret_cast<std::map<RE::FormID, ExtraValueStorage>&&>(std::move(storageMap.get()));
					break;
				}
				case PrimaryRecordType::PlayerStorage: {
					//I'm doing this like this because the fucking PlayerSingleton data is a fucking mess. Fix it. Please.
					auto player = PlayerStorage::GetSingleton();
					player->ResetStorage(true);
					PrimaryHandle handler{ reinterpret_cast<legPlayerStorage&>(*player) };
					arg.Serialize(handler);
					//*PlayerStorage::GetSingleton() = std::move(player.GetWrapObject());
					break;
				}
													 //buffer.Serialize(serial_entry->second);

				}
			}
		}

		static bool LoadCallback(TOME::SerialHeader header, SKSE::SerializationInterface* io)
		{
			if ("2.0.0.5"_v.pack() < header.version) {
				return false;
			}


			Handler self{};

			self.Handle(io, header);

			return true;
		}


		SerialCallback manifestCallback = SerializeList;
		legEVStorageMap storageMap;
		//legPlayerStorage player = PlayerStorage{true };
	};



}