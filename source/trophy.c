/*
*
* Trophy set editing (lock/unlock) based on PS3TrophyIsGood by darkautism
*  - https://github.com/darkautism/PS3TrophyIsGood/
*
*/

#include <stdlib.h>
#include <sys/systime.h>

#include "util.h"
#include "trophy.h"

#ifdef APOLLO_ENABLE_LOGGING
void log_tropRecord(const tropRecord_t* t)
{
    LOG("tropRecord\n Id=%d\n Offset=0x%X\n Size=%d\n Used=%d\n", t->id, t->offset, t->size, t->usedTimes);
}

void log_trnsTrophyInfo(const trnsTrophyInfo_t* t)
{
    LOG("trnsTrophyInfo\n Id=%d\n Type=%d\n Sync=%d\n Exist=%d\n Seq=%d", t->trophyID, t->trophyType, t->syncState, t->isExist, t->sequenceNumber);
}

void log_tropListInfo(const tropListInfo_t* t)
{
    LOG("tropListInfo\n Trop#=%d\n", t->getTrophyNumber);
}

void log_trnsInitTime(const trnsInitTime_t* t)
{
    LOG("trnsInitTime\n %X / %X\n", t->initTime.dt1, t->initTime.dt2);
}

void log_tropType(const tropType_t* t)
{
    LOG("tropType\n Type=%d\n Seq#=%d\n", t->type, t->sequenceNumber);
}

void log_tropTimeInfo(const tropTimeInfo_t* t)
{
    // syncState =  means 0x011000 synchronized. 
    // 0x001000  =  unsync
    LOG("tropTimeInfo\n Unlocked=%d\n Sync=%d\n Seq#=%d\n", t->unlocked, t->syncState, t->sequenceNumber);
}

void log_tropUnkType7(const tropUnkType7_t* t)
{
    LOG("tropUnkType7\n Count=%d\n Synced=%d\n", t->getTrophyCount, t->syncTrophyCount);
}
#endif

// Trophies store a date-time as: ticks/10
// 621355968000000000 = epoch ticks
// https://tickstodatetime.azurewebsites.net/
uint64_t getCurrentDateTime()
{
	uint64_t sec;

	sysGetCurrentTime(&sec, NULL);
	sec = (sec * 1000000) + 62135596800000000;

    return (sec);
}

int trns_parseTrophy(const uint8_t* data, trnsTrophy_t* trns_trophy)
{
    tropHeader_t* trnsHeader;
    tropRecord_t* typeRecordTable;

    memset(trns_trophy, 0, sizeof(trnsTrophy_t));
    trnsHeader = (tropHeader_t*) data;

    if (trnsHeader->magic != 0x818F54AD00010000)
    {
        LOG("Not a valid TROPTRNS.DAT.");
        return 0;
    }

    typeRecordTable = (tropRecord_t*)(data + sizeof(tropHeader_t));

    for (int i = 0; i < trnsHeader->recordCount; i++)
    {
        switch (typeRecordTable[i].id)
        {
        case 2:
            // Type 2
            trns_trophy->account_id = (char*) data + typeRecordTable[i].offset + sizeof(tropBlockHeader_t) + 16;
            break;

        case 3:
            // Type 3
            trns_trophy->npcomm_id = (char*) data + typeRecordTable[i].offset + sizeof(tropBlockHeader_t);

            trns_trophy->allGetTrophysCount = (uint32_t*)(data + typeRecordTable[i].offset + sizeof(tropBlockHeader_t) + 20);
            trns_trophy->allSyncPSNTrophyCount = (uint32_t*)(data + typeRecordTable[i].offset + sizeof(tropBlockHeader_t) + 24);

            int u1 = *(uint32_t*)(data + typeRecordTable[i].offset + sizeof(tropBlockHeader_t) + 16); // always 00000090
            break;

        case 4:
            // Type 4
            trns_trophy->trophyInitTime = (trnsInitTime_t*)(data + typeRecordTable[i].offset);
            trns_trophy->trophyInfoTable = (trnsTrophyInfo_t*)(data + typeRecordTable[i].offset + sizeof(trnsInitTime_t));
            break;

        default:
            break;
        }
//        log_tropRecord(&typeRecordTable[i]);
    }

    LOG("Total Unlocked: %d (Synced: %d)", *trns_trophy->allGetTrophysCount, *trns_trophy->allSyncPSNTrophyCount);

/*
    tropRecord_t* TrophyInfoRecord = &typeRecordTable[3];
    trnsTrophyInfo_t* trophyInfoTable = (trnsTrophyInfo_t*) (data + TrophyInfoRecord->offset + sizeof(trnsInitTime_t));
    for (int i = 0; i < (*(trns_trophy->allGetTrophysCount) - 1); i++)
    {
        log_trnsTrophyInfo(&trophyInfoTable[i]);
    }
*/

    return (trns_trophy->trophyInitTime && trns_trophy->trophyInfoTable);
}

int usr_parseTrophy(const uint8_t* data, usrTrophy_t* usr_trophy)
{
    tropHeader_t* trnsHeader;
    tropRecord_t* typeRecordTable;
//    uint32_t all_trophy_number;
//    uint32_t AchievementRate[4];

    memset(usr_trophy, 0, sizeof(usrTrophy_t));
    trnsHeader = (tropHeader_t*) data;

    if (trnsHeader->magic != 0x818F54AD00010000)
    {
        LOG("Not a valid TROPUSR.DAT.");
        return 0;
    }

    typeRecordTable = (tropRecord_t*)(data + sizeof(tropHeader_t));

    for (int i = 0; i < trnsHeader->recordCount; i++)
    {
        switch (typeRecordTable[i].id)
        {
        case 1:
            // unknown
            break;
        case 2:
            usr_trophy->account_id = (char*) data + typeRecordTable[i].offset + sizeof(tropBlockHeader_t) + 16;
            break;
        case 3:
            usr_trophy->npcomm_id = (char*) data + typeRecordTable[i].offset + sizeof(tropBlockHeader_t);
/*
            blockdata = data + typeRecordTable[i].offset + sizeof(tropBlockHeader_t);
            memcpy(trophy_id, blockdata, 16);
            trophy_id[16]=0;
            short u1 = BitConverter.ToInt16(blockdata, 16);
            short u2 = BitConverter.ToInt16(blockdata, 18);
            short u3 = BitConverter.ToInt16(blockdata, 20);
            short u4 = BitConverter.ToInt16(blockdata, 22);
            int u5 = BitConverter.ToInt32(blockdata, 28).ChangeEndian();
            all_trophy_number = *(uint32_t*)(blockdata + 24);
            AchievementRate[0] = *(uint32_t*)(blockdata + 64);
            AchievementRate[1] = *(uint32_t*)(blockdata + 68);
            AchievementRate[2] = *(uint32_t*)(blockdata + 72);
            AchievementRate[3] = *(uint32_t*)(blockdata + 76);
*/
            break;
        case 4:
            usr_trophy->trophyTypeTable = (tropType_t*)(data + typeRecordTable[i].offset);
            break;
        case 5:
            usr_trophy->trophyListInfo = (tropListInfo_t*)(data + typeRecordTable[i].offset);
            break;
        case 6:
            usr_trophy->trophyTimeInfoTable = (tropTimeInfo_t*)(data + typeRecordTable[i].offset);
            break;
        case 7:
            // unknown
            usr_trophy->unknownType7 = (tropUnkType7_t*)(data + typeRecordTable[i].offset);
            break;
        case 8:
            // hash
            usr_trophy->unknownHash = (char*) data + typeRecordTable[i].offset + sizeof(tropBlockHeader_t);
            break;
        case 9:
            // Usually written with some numbers of platinum trophies, unknown
            // LOG("Unsupported block type. (Type{%d})", typeRecordTable[i].id);
            break;
        case 10:
            // i think it just a padding
            break;

        default:
            break;
        }
    }

    return (usr_trophy->trophyListInfo && usr_trophy->trophyTimeInfoTable && usr_trophy->unknownType7);
}

tropTimeInfo_t* getTrophyTimeInfo(const uint8_t* data)
{
    usrTrophy_t tropusr;

    usr_parseTrophy(data, &tropusr);

    return(tropusr.trophyTimeInfoTable);
}

void trns_UpdateAccount(uint8_t* trns_data, const char* account)
{
    trnsTrophy_t troptrns;

    if (!trns_parseTrophy(trns_data, &troptrns))
        return;

    memcpy(troptrns.account_id, account, TROP_ACCOUNT_ID_SIZE);
}

void trns_UnlockTrophy(uint8_t* trns_data, int id, int type)
{
    trnsTrophy_t troptrns;
    uint64_t dt = getCurrentDateTime();
    uint32_t uCount;

    if (!trns_parseTrophy(trns_data, &troptrns))
        return;

    uCount = *troptrns.allGetTrophysCount;

    for (int i = 0; i < uCount; i++)
    {
        if(troptrns.trophyInfoTable[i].trophyID == id)
        {
            LOG("Error: Can't unlock a trophy twice");
            return;
        }

        if (troptrns.trophyInfoTable[i].getTime.dt1 > dt || troptrns.trophyInfoTable[i].getTime.dt2 > dt)
        {
            LOG("Error: synchronization has already been done at this time, please try setting a later time.");
            LOG("Table(1)=%llX Table(2)=%llX dt=%llX", troptrns.trophyInfoTable[i].getTime.dt1, troptrns.trophyInfoTable[i].getTime.dt2, dt);
            return;
        }
    }

    trnsTrophyInfo_t* ti = &troptrns.trophyInfoTable[uCount];

    ti->trophyID = id;
    ti->trophyType = type;
    ti->syncState = 0;
    ti->_unknowInt1 = 0;
    ti->_unknowInt2 = 0x00001000;
    ti->_unknowInt3 = 0;
    ti->getTime.dt1 = dt;
    ti->getTime.dt2 = dt;
    ti->isExist = 2;

    uCount++;
    *troptrns.allGetTrophysCount = uCount;

    LOG("Unlocked Total: %d", *troptrns.allGetTrophysCount);
}

void trns_LockTrophy(uint8_t* trns_data, int id, int type)
{
    trnsTrophy_t troptrns;
    uint32_t uCount;

    if (!trns_parseTrophy(trns_data, &troptrns))
        return;

    uCount = *troptrns.allGetTrophysCount;

    for (int i = 0; i < uCount; i++)
    {
        if(troptrns.trophyInfoTable[i].trophyID == id)
        {
            if (troptrns.trophyInfoTable[i].syncState)
            {
                LOG("Can't lock: synchronization has already been done. (State = %08X)", troptrns.trophyInfoTable[i].syncState);
                return;
            }

            troptrns.trophyInfoTable[i].trophyID = 0;
            troptrns.trophyInfoTable[i].trophyType = 0;
            troptrns.trophyInfoTable[i]._unknowInt1 = 0;
            troptrns.trophyInfoTable[i]._unknowInt2 = 0;
            troptrns.trophyInfoTable[i]._unknowInt3 = 0;
            troptrns.trophyInfoTable[i].getTime.dt1 = 0;
            troptrns.trophyInfoTable[i].getTime.dt2 = 0;
            troptrns.trophyInfoTable[i].isExist = 0;

            uCount--;
            *troptrns.allGetTrophysCount = uCount;

        }
    }

    LOG("Unlocked Total: %d", *troptrns.allGetTrophysCount);
}

void usr_UpdateAccount(uint8_t* usr_data, const char* account)
{
    usrTrophy_t tropusr;

    if (!usr_parseTrophy(usr_data, &tropusr))
        return;

    memcpy(tropusr.account_id, account, TROP_ACCOUNT_ID_SIZE);
}

int usr_UnlockTrophy(uint8_t* usr_data, int id)
{
    usrTrophy_t tropusr;
    uint64_t dt = getCurrentDateTime();

    if (!usr_parseTrophy(usr_data, &tropusr))
        return 0;

    tropTimeInfo_t* tti = &tropusr.trophyTimeInfoTable[id];

    tti->unlockTime.dt1 = dt;
    tti->unlockTime.dt2 = dt;
    if (!tti->unlocked)
    {
        tropusr.trophyListInfo->getTrophyNumber++;
        tropusr.unknownType7->getTrophyCount++;
    }
    tropusr.trophyListInfo->AchievementRate[id / 32] |= (uint32_t)(1 << id);

    tti->unlocked = 1;
    tti->syncState = TROP_STATE_UNSYNC; // unsync'ed

    if (tropusr.trophyListInfo->listLastGetTrophyTime.dt1 < dt)
    {
        tropusr.trophyListInfo->listLastGetTrophyTime.dt1 = dt;
        tropusr.trophyListInfo->listLastGetTrophyTime.dt2 = dt;
    }

    tropusr.trophyListInfo->listLastUpdateTime.dt1 = dt;
    tropusr.trophyListInfo->listLastUpdateTime.dt2 = dt;

    return(tropusr.trophyTypeTable[id].type);
}

int usr_LockTrophy(uint8_t* usr_data, int id)
{
    usrTrophy_t tropusr;

    if (!usr_parseTrophy(usr_data, &tropusr))
        return 0;

    tropTimeInfo_t* tti = &tropusr.trophyTimeInfoTable[id];

    if (tti->syncState == TROP_STATE_SYNCED)
    {
        LOG("Can't lock a synced trophy!");
        return 0;
    }

    tti->unlockTime.dt1 = 0;
    tti->unlockTime.dt2 = 0;
    if (tti->unlocked)
    {
        tropusr.trophyListInfo->getTrophyNumber--;
        tropusr.unknownType7->getTrophyCount--;
    }
    tropusr.trophyListInfo->AchievementRate[id / 32] &= 0xFFFFFFFF ^ (uint32_t)(1 << id);

    tti->unlocked = 0;
    tti->syncState = 0;

    return(tropusr.trophyTypeTable[id].type);
}

int patch_trophy_account(const char* trp_path, const char* account_id)
{
    char filepath[256];
    uint8_t* data;
    size_t len;

    snprintf(filepath, sizeof(filepath), "%s" "TROPUSR.DAT", trp_path);
    if (read_buffer(filepath, &data, &len) != 0)
    {
        LOG("Failed to open %s", filepath);
        return 0;
    }

    // update Account_ID on TROPUSR.DAT
    usr_UpdateAccount(data, account_id);

    if (write_buffer(filepath, data, len) != 0)
    {
        LOG("Failed to save %s", filepath);
        return 0;
    }
    free(data);

    snprintf(filepath, sizeof(filepath), "%s" "TROPTRNS.DAT", trp_path);
    if (read_buffer(filepath, &data, &len) != 0)
    {
        LOG("Failed to open %s", filepath);
        return 0;
    }

    // update Account_ID on TROPTRNS.DAT
    trns_UpdateAccount(data, account_id);

    if (write_buffer(filepath, data, len) != 0)
    {
        LOG("Failed to save %s", filepath);
        return 0;
    }

    free(data);
    return 1;
}

int apply_trophy_patch(const char* trp_path, uint32_t trophy_id, int unlock)
{
    char filepath[256];
    int trop_type;
    uint8_t* data;
    size_t len;

    snprintf(filepath, sizeof(filepath), "%s" "TROPUSR.DAT", trp_path);

    if (read_buffer(filepath, &data, &len) != 0)
    {
        LOG("Failed to open %s", filepath);
        return 0;
    }

    if (unlock)
        trop_type = usr_UnlockTrophy(data, trophy_id);
    else
        trop_type = usr_LockTrophy(data, trophy_id);

    if (write_buffer(filepath, data, len) != 0)
    {
        LOG("Failed to save %s", filepath);
        return 0;
    }
    free(data);

    snprintf(filepath, sizeof(filepath), "%s" "TROPTRNS.DAT", trp_path);

    if (read_buffer(filepath, &data, &len) != 0)
    {
        LOG("Failed to open %s", filepath);
        return 0;
    }

    if (unlock)
        trns_UnlockTrophy(data, trophy_id, trop_type);
    else
        trns_LockTrophy(data, trophy_id, trop_type);

    if (write_buffer(filepath, data, len) != 0)
    {
        LOG("Failed to save %s", filepath);
        return 0;
    }

    free(data);
    return 1;
}
