#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "Creature.h"
#include "GossipDef.h"
#include "ScriptedGossip.h"
#include "TemporarySummon.h"
#include "Pet.h"
#include "ObjectMgr.h"

class Custom_Beastmaster : public CreatureScript
{
public:
	Custom_Beastmaster() : CreatureScript("Custom_Beastmaster") { }

	void TamePet(Player* player, Creature* creature, uint32 entry)
	{
		Creature* newPet = creature->SummonCreature(entry, player->GetPositionX(), player->GetPositionY() + 2, player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 500);
		if (!newPet)
			return;

		Pet* pet = player->CreateTamedPetFrom(newPet, 0);
		if (!pet)
			return;

		newPet->setDeathState(JUST_DIED);
		newPet->RemoveCorpse();
		newPet->SetHealth(0);

		pet->SetPower(POWER_HAPPINESS, 1048000);
		pet->SetUInt64Value(UNIT_FIELD_CREATEDBY, player->GetGUID());
		pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, player->getFaction());

		pet->SetUInt32Value(UNIT_FIELD_LEVEL, player->getLevel());
		pet->GetMap()->AddToMap(pet->ToCreature());

		pet->GetCharmInfo()->SetPetNumber(sObjectMgr->GeneratePetNumber(), true);
		if (!pet->InitStatsForLevel(player->getLevel()))
			pet->UpdateAllStats();

		player->SetMinion(pet, true);
		pet->SavePetToDB(PET_SAVE_AS_CURRENT);
		pet->InitTalentForLevel();
		player->PetSpellInitialize();

		player->PlayerTalkClass->SendCloseGossip();
	}

	bool OnGossipHello(Player* player, Creature* creature)
	{
		if (player->getClass() != CLASS_HUNTER)
		{
			player->GetSession()->SendNotification("You are not a Hunter!");
			player->PlayerTalkClass->SendCloseGossip();
			return true;
		}

		if (player->GetPet())
			player->ADD_GOSSIP_ITEM(0, "You must disband your current pet to tame a new one!", GOSSIP_SENDER_MAIN, 100);
		else
		player->ADD_GOSSIP_ITEM_EXTENDED(0, "Tame a Custom Pet", GOSSIP_SENDER_MAIN, 1, "", 0, true);

		player->SEND_GOSSIP_MENU(1, creature->GetGUID());
		return true;
	}

	bool OnGossipSelectCode(Player* player, Creature* creature, uint32 /*sender*/, uint32 action, const char* code)
	{
		player->PlayerTalkClass->ClearMenus();
		uint32 creatureID = atoi((char*)code);

		if (!creatureID)
		{
			player->CLOSE_GOSSIP_MENU();
			player->GetSession()->SendNotification("You must enter in a proper ID");
			return false;
		}

		const CreatureTemplate* beast = sObjectMgr->GetCreatureTemplate(creatureID);
		if (action == 1)
		{
			if (beast)
			{
				uint32 type = beast->type;
				uint32 type_flags = beast->type_flags;

				if (type == CREATURE_TYPE_BEAST /*&& type_flags == CREATURE_TYPEFLAGS_TAMEABLE*/) //creature is beast AND tameable
				{
					TamePet(player, creature, creatureID);
				}
				else
				{
					player->GetSession()->SendNotification("The creature you wish to tame is not a beast, or capable of being tamed!");
					player->PlayerTalkClass->SendCloseGossip();
				}
			}
			else
			{
				player->GetSession()->SendNotification("This creature entry does not exist!");
				player->PlayerTalkClass->SendCloseGossip();
			}
		}
		return true;
	}
};

void AddSC_Custom_Beastmaster()
{
	new Custom_Beastmaster();
}