#include "ArenaScore.h"
#include "Battleground.h"
#include "Chat.h"
#include "Common.h"
#include "Config.h"
#include "DBCEnums.h"
#include "GameObject.h"
#include "Player.h"
#include "ReputationMgr.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "spirit_of_competition_loader.h"
#include "TaskScheduler.h"

enum Spells
{
    PARTICIPANT        = 48163,
    PARTICIPANT_EFFECT = 48056,
    WINNER             = 48164,
    WINNER_EFFECT      = 48057,
    COMMONER_ALL       = 48305,
    COMMONER           = 65527,
    SPIRIT_DESPAWN     = 48709
};

enum Quests
{
    FLAG_PARTICIPANT = 12187,
    FLAG_WINNER      = 12186
};

class SCreward : public BGScript {

public:
    SCreward() : BGScript("SCreward") { }

    typedef std::map<ObjectGuid, Player*> BattlegroundPlayerMap;
    BattlegroundPlayerMap bgPlayerMap; //TODO: Clear out player map.

    void OnBattlegroundAddPlayer(Battleground* /*bg*/, Player* player) override
    {
        if (player) // In case null.
        {
            player->removeSpell(PARTICIPANT, SPEC_MASK_ALL, false); // Clear out any vestiges.
            player->removeSpell(WINNER, SPEC_MASK_ALL, false);
        }
    }

    void OnBattlegroundStart(Battleground* bg) override // When gates open?
    {
        bgPlayerMap = bg->GetPlayers();

        for (auto itr : bgPlayerMap) // For every player in BG
        {
            if (itr.second->GetQuestStatus(FLAG_PARTICIPANT) != QUEST_STATUS_COMPLETE) // If player hasn't already fully participated in a BG (Gotten tabard)
            {
                itr.second->CastSpell(itr.second, PARTICIPANT, true); // Give them participant spell, will be checked later.
            }
        }
    }

    void OnBattlegroundEnd(Battleground* bg, TeamId teamWin) override
    {
        bgPlayerMap = bg->GetPlayers();

        for (auto itr : bgPlayerMap) // For every player in BG
        {            
            if (itr.second->GetTeamId() == teamWin && itr.second->GetQuestStatus(FLAG_WINNER) != QUEST_STATUS_COMPLETE) // If won and hasn't already gotten pet.
            {
                itr.second->CastSpell(itr.second, WINNER, true); // Winner flag spell, will be checked later.
            }

            HandleParticipantReward(itr.second);
            HandleWinnerRewardPlayer(itr.second); //TODO: Lock behind config
        }
        HandleWinnerRewardTeam(bgPlayerMap); //TODO:      Lock behind config
    }

private:
    void HandleParticipantReward(Player* player)
    {
        if (player->HasActiveSpell(PARTICIPANT)) // If player has been in BG since start and hasn't completed participant quest.
        { //                                        Avoids multiple mail sending for tabard reward.
            player->SetQuestStatus(FLAG_PARTICIPANT, QUEST_STATUS_COMPLETE, true); // Set flag quest.
            player->removeSpell(PARTICIPANT, SPEC_MASK_ALL, false); //                Remove flag spell.

            //TODO: send mail + tabard
        }
    }

    void HandleWinnerRewardTeam(BattlegroundPlayerMap bgPlayerMap) // Method 1: limited pets for entire winning team
    {
        float petPct = 10.0f;

        //TODO: Genuinely no idea how I'll do this one yet lol.
    }

    void HandleWinnerRewardPlayer(Player* player) // Method 2: rolls for each individual winning player
    {
        float chance = 1.0f;

        uint8 randomN = urand(1, 100);

        if (randomN <= chance)
        {
            //TODO: send mail + pet
            player->SetQuestStatus(FLAG_WINNER, QUEST_STATUS_COMPLETE, true);
        }

        player->removeSpell(WINNER, SPEC_MASK_ALL, false);
    }
};

void Add_spirit_of_competition()
{
    new SCreward();
}
