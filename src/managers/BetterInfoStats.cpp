#include "BetterInfoStats.h"
#include "../utils.hpp"

#include <fmt/format.h>

bool BetterInfoStats::init(){
    bool init = CCNode::init();
    if(!init) return false;

    this->m_fileName = fmt::format("geode/mods/{}/CCBetterInfoStats.dat", Mod::get()->getID());
    #ifdef GEODE_IS_ANDROID
        this->m_fileName = "CCBetterInfoStats.dat";
    #else
        this->migrateSaveData();
    #endif

    auto FU = CCFileUtils::sharedFileUtils();

    m_normalDict = CCDictionary::create();
    m_normalDict->retain();

    m_practiceDict = CCDictionary::create();
    m_practiceDict->retain();

    m_firstPlayedDict = CCDictionary::create();
    m_firstPlayedDict->retain();

    m_lastPlayedDict = CCDictionary::create();
    m_lastPlayedDict->retain();

    m_normalAttemptDict = CCDictionary::create();
    m_normalAttemptDict->retain();

    m_practiceAttemptDict = CCDictionary::create();
    m_practiceAttemptDict->retain();

    this->setup();

    return true;
}

void BetterInfoStats::migrateSaveData() {
    auto oldPath = dirs::getSaveDir() / "CCBetterInfoStats.dat";
    auto newPath = dirs::getSaveDir() / std::string(this->m_fileName); //it would be better to utilize Mod::get()->getSaveDir, however that can result in future issues

    if(ghc::filesystem::exists(oldPath) && !ghc::filesystem::exists(newPath)) {
        log::info("CCBetterInfoStats exists in main GD folder but not in mod folder, migrating");
        ghc::filesystem::rename(oldPath, newPath);
        
        this->getScheduler()->scheduleSelector(schedule_selector(BetterInfoStats::migrationPopup), this, 1, 0, 3, false);
    }
}

void BetterInfoStats::migrationPopup(float dt){
    FLAlertLayer::create("BetterInfo", "<cg>Succesfully</c> loaded stats from a previous installation of <c_>Better</c><cj>Info</c>.\n\nPlease note that if you choose to remove your data while uninstalling this version of BI, you will lose these stats permanently.", "OK")->show();
}

BetterInfoStats::BetterInfoStats(){}

void BetterInfoStats::encodeDataTo(DS_Dictionary* data) {
    data->setDictForKey("normalCompletions", m_normalDict);
    data->setDictForKey("practiceCompletions", m_practiceDict);
    data->setDictForKey("firstPlayed", m_firstPlayedDict);
    data->setDictForKey("lastPlayed", m_lastPlayedDict);
    data->setDictForKey("normalAttempt", m_normalAttemptDict);
    data->setDictForKey("practiceAttempt", m_practiceAttemptDict);
}

void BetterInfoStats::dataLoaded(DS_Dictionary* data) {
    auto normalDict = static_cast<CCDictionary*>(data->getObjectForKey("normalCompletions"));
    if(normalDict) {
        m_normalDict->release();
        m_normalDict = normalDict;
        m_normalDict->retain();
    }

    auto practiceDict = static_cast<CCDictionary*>(data->getObjectForKey("practiceCompletions"));
    if(practiceDict) {
        m_practiceDict->release();
        m_practiceDict = practiceDict;
        m_practiceDict->retain();
    }

    auto firstPlayedDict = static_cast<CCDictionary*>(data->getObjectForKey("firstPlayed"));
    if(firstPlayedDict) {
        m_firstPlayedDict->release();
        m_firstPlayedDict = firstPlayedDict;
        m_firstPlayedDict->retain();
    }

    auto lastPlayedDict = static_cast<CCDictionary*>(data->getObjectForKey("lastPlayed"));
    if(lastPlayedDict) {
        m_lastPlayedDict->release();
        m_lastPlayedDict = lastPlayedDict;
        m_lastPlayedDict->retain();
    }

    auto normalAttemptDict = static_cast<CCDictionary*>(data->getObjectForKey("normalAttempt"));
    if(normalAttemptDict) {
        m_normalAttemptDict->release();
        m_normalAttemptDict = normalAttemptDict;
        m_normalAttemptDict->retain();
    }

    auto practiceAttemptDict = static_cast<CCDictionary*>(data->getObjectForKey("practiceAttempt"));
    if(practiceAttemptDict) {
        m_practiceAttemptDict->release();
        m_practiceAttemptDict = practiceAttemptDict;
        m_practiceAttemptDict->retain();
    }

    this->save();
}

void BetterInfoStats::firstLoad() {
    this->save();
}

void BetterInfoStats::logCompletion(GJGameLevel* level, bool practice) {
    if(getCompletion(level, practice) != 0) return;

    logCompletion(level, practice, std::time(nullptr));
}

void BetterInfoStats::logCompletion(GJGameLevel* level, bool practice, time_t timestamp) {
    if(level->m_levelType == GJLevelType::Editor) return;

    auto dict = practice ? m_practiceDict : m_normalDict;
    dict->setObject(CCString::create(std::to_string(timestamp).c_str()), keyForLevel(level));
    
    this->save();
}

time_t BetterInfoStats::getCompletion(GJGameLevel* level, bool practice) {
    auto dict = practice ? m_practiceDict : m_normalDict;

    auto ret = dict->valueForKey(keyForLevel(level));
    if(std::string_view(ret->getCString()).empty()) return 0;

    return BetterInfo::strtol(ret->getCString());
}

void BetterInfoStats::logPlay(GJGameLevel* level) {
    if(level->m_levelType == GJLevelType::Editor) return;
    
    auto idString = keyForLevel(level);
    auto timeString = CCString::create(std::to_string(std::time(nullptr)).c_str());
    m_lastPlayedDict->setObject(timeString, idString);
    if(getPlay(level, false) == 0 && level->m_normalPercent <= 0) m_firstPlayedDict->setObject(timeString, idString);
    if(level->m_normalPercent == 100 && getCompletion(level, false) == 0) logCompletion(level, false, -1);
    if(level->m_practicePercent == 100 && getCompletion(level, true) == 0) logCompletion(level, true, -1);
    
    this->save();
}

time_t BetterInfoStats::getPlay(GJGameLevel* level, bool last) {
    auto dict = last ? m_lastPlayedDict : m_firstPlayedDict;

    auto ret = dict->valueForKey(keyForLevel(level));
    if(std::string_view(ret->getCString()).empty()) return 0;

    return BetterInfo::strtol(ret->getCString());
}

std::string BetterInfoStats::keyForLevel(GJGameLevel* level) {
    //return "";
    return fmt::format("{}_{}_{}_{}", level->m_levelID.value(), static_cast<int>(level->m_levelType), level->m_dailyID.value(), level->m_gauntletLevel);
}

void BetterInfoStats::logAttempt(GJGameLevel* level, bool practice) {
    if(level->m_levelType == GJLevelType::Editor) return;

    auto dict = practice ? m_practiceAttemptDict : m_normalAttemptDict;
    dict->setObject(CCString::create(std::to_string(getAttempts(level, practice) + 1).c_str()), keyForLevel(level));
}

int BetterInfoStats::getAttempts(GJGameLevel* level, bool practice) {
    auto dict = practice ? m_practiceAttemptDict : m_normalAttemptDict;

    auto ret = dict->valueForKey(keyForLevel(level));
    if(std::string_view(ret->getCString()).empty()) return 0;

    return BetterInfo::stoi(ret->getCString());
}