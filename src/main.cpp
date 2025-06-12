namespace Hooks
{
	class hkGetFormTypeString
	{
	public:
		static void FString(UE::FString* a_this, const UE::FString& a_that)
		{
			using func_t = decltype(&FString);
			static REL::Relocation<func_t> func{ REL::Offset(0x0D820A0) };
			return func(a_this, a_that);
		}

		static void GetFormTypeString(UE::UTESForm* a_this, UE::FString* a_string)
		{
			if (!a_this)
			{
				static UE::FString FormTypeNull{ "NULL" };
				return FString(a_string, FormTypeNull);
			}

			return _GetFormTypeString(a_this, a_string);
		}

		inline static REL::Hook _GetFormTypeString{ REL::Offset(0x4955C10), 0xC6, GetFormTypeString };
	};

	class hkPickRef
	{
	public:
		static UE::UAltarCheatManager* GetCheatManager()
		{
			if (auto Engine = UE::UEngine::GetSingleton())
			{
				if (auto World = Engine->GetCurrentPlayWorld(nullptr))
				{
					if (auto PlayerController = UE::UGameplayStatics::GetPlayerController(World, 0))
					{
						return static_cast<UE::UAltarCheatManager*>(PlayerController->cheatManager.objectPtr.handle);
					}
				}
			}

			return nullptr;
		}

		static bool PickRef(
			const RE::SCRIPT_PARAMETER* a_parameters,
			const char* a_compiledParams,
			RE::TESObjectREFR* a_refObject,
			RE::TESObjectREFR* a_container,
			RE::Script* a_script,
			RE::ScriptLocals* a_scriptLocals,
			double&,
			std::uint32_t& a_offset)
		{
			RE::TESObjectREFR* pick{ nullptr };
			auto result = RE::Script::ParseParameters(
				a_parameters,
				a_compiledParams,
				a_offset,
				a_refObject,
				a_container,
				a_script,
				a_scriptLocals,
				&pick);

			if (result && pick && pick->pairingEntry && pick->pairingEntry->hostItem)
			{
				switch (pick->GetFormType())
				{
				case RE::FormType::Reference:
				case RE::FormType::ActorCharacter:
				case RE::FormType::ActorCreature:
					UE::AsyncTask(
						UE::ENamedThreads::GameThread,
						[&]()
						{
							if (auto CheatManager = GetCheatManager())
							{
								if (auto Actor = static_cast<UE::AActor*>(pick->pairingEntry->hostItem))
								{
									CheatManager->SetSelectedActor(Actor);
								}
							}
						});
					break;

				default:
					break;
				}
			}

			return result;
		}

		static void Install()
		{
			if (auto cmd = RE::SCRIPT_FUNCTION::LocateConsoleCommand("PickRefByID"sv))
			{
				cmd->executeFunction = PickRef;
			}
		}
	};

	static void Install()
	{
		hkPickRef::Install();
	}
}

namespace
{
	void MessageHandler(OBSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type)
		{
		case OBSE::MessagingInterface::kPostLoad:
			Hooks::Install();
			break;
		default:
			break;
		}
	}
}

OBSE_PLUGIN_LOAD(const OBSE::LoadInterface* a_obse)
{
	OBSE::Init(a_obse, { .trampoline = true });
	OBSE::GetMessagingInterface()->RegisterListener(MessageHandler);
	return true;
}
