#include "ExtendedLevelInfo.h"
#include "UnregisteredProfileLayer.h"
#include "PaginatedFLAlert.h"
#include "../utils.hpp"
#include "../managers/BetterInfoStats.h"
//#include "../managers/BetterInfoStatsV2.h"
#include "../managers/BetterInfoCache.h"

#include <deque>
#include <algorithm>
#include <thread>
#include <string>
#include <iterator>

ExtendedLevelInfo* ExtendedLevelInfo::create(GJGameLevel* level){
    auto ret = new ExtendedLevelInfo();
    if (ret && ret->init(level)) {
        //robert 1 :D
        ret->autorelease();
    } else {
        //robert -1
        delete ret;
        ret = nullptr;
    }
    return ret;
}

void ExtendedLevelInfo::onClose(cocos2d::CCObject* sender)
{
    BetterInfoCache::sharedState()->m_uploadDateDelegate = nullptr;
    m_level->release();
    
    CvoltonAlertLayerStub::onClose(sender);
}

void ExtendedLevelInfo::onCopyName(cocos2d::CCObject* sender)
{
    BetterInfo::copyToClipboard(m_level->m_levelName.c_str(), this);
}

void ExtendedLevelInfo::onCopyAuthor(cocos2d::CCObject* sender)
{
    BetterInfo::copyToClipboard(m_level->m_creatorName.c_str(), this);
}

void ExtendedLevelInfo::onCopyDesc(cocos2d::CCObject* sender)
{
    BetterInfo::copyToClipboard(m_level->getUnpackedLevelDescription().c_str(), this);
}

void ExtendedLevelInfo::onNext(cocos2d::CCObject* sender)
{
    loadPage(m_page+1);
}

void ExtendedLevelInfo::onPrev(cocos2d::CCObject* sender)
{
    loadPage(m_page-1);
}

void ExtendedLevelInfo::onUploadDateLoaded(int levelID, const std::string& date) {
    m_uploadDateEstimated = date;
    refreshInfoTexts();
    loadPage(m_page);
}

void ExtendedLevelInfo::loadPage(int page) {
    this->m_page = page;
    if(page % 2 == 0) { 
        m_info->setString(m_primary);
        m_nextBtn->setVisible(true);
        m_prevBtn->setVisible(false);
    } else {
        m_info->setString(m_secondary);
        m_nextBtn->setVisible(false);
        m_prevBtn->setVisible(true);
    } 
}

void ExtendedLevelInfo::refreshInfoTexts() {
    auto cache = BetterInfoCache::sharedState();

    auto uploadDateStd = std::string(m_level->m_uploadDate);
    auto updateDateStd = std::string(m_level->m_updateDate);
    int levelPassword = m_level->m_password;
    std::ostringstream infoText;
    infoText << "\n<cj>Uploaded</c>: " << LevelMetadata::stringDate(!uploadDateStd.empty() ? uploadDateStd : LevelMetadata::addPlus(cache->getLevelInfo(m_level->m_levelID, "upload-date")))
        << "\n<cj>Updated</c>: " << LevelMetadata::stringDate(!updateDateStd.empty() ? updateDateStd :  LevelMetadata::addPlus(cache->getLevelInfo(m_level->m_levelID, "update-date")))
        //<< "\n<cy>Stars Requested</c>: " << m_level->m_starsRequested
        << "\n<cg>Original</c>: " << LevelMetadata::zeroIfNA(m_level->m_originalLevel)
        //<< "\n<cg>Feature score</c>: " << LevelMetadata::zeroIfNA(m_level->m_featured)
        << "\n<cy>Game Version</c>: " << LevelMetadata::getGameVersionName(m_level->m_gameVersion)
        //<< "\nFeature Score</c>: " << m_level->m_featured
        << "\n<co>Password</c>: " << LevelMetadata::passwordString(levelPassword)
        << "\n<cr>In Editor</c>: " << TimeUtils::workingTime(m_level->m_workingTime)
        << "\n<cr>Editor (C)</c>: " << TimeUtils::workingTime(m_level->m_workingTime2);

    m_primary = infoText.str();
    infoText.str("");
    infoText << "\n<cj>Uploaded</c>: " << TimeUtils::isoTimeToString(m_uploadDateEstimated)
        << "\n<cg>Objects</c>: " << LevelMetadata::zeroIfNA(m_level->m_objectCount)
        << "\n<cg>Objects (est.)</c>: " << LevelMetadata::zeroIfNA(m_objectsEstimated) //i have no idea what the 0 and 11 mean, i just copied them from PlayLayer::init
        << "\n<cy>Feature Score</c>: " << LevelMetadata::zeroIfNA(m_level->m_featured)
        << "\n<co>Two-player</c>: " << LevelMetadata::boolString(m_level->m_twoPlayerMode)
        << "\n<cr>Size</c>: " << m_fileSizeCompressed << " / " << m_fileSizeUncompressed;
    ;

    m_secondary = infoText.str();

    /*infoText << "<cg>Total Attempts</c>: " << m_level->attempts
        << "\n<cl>Total Jumps</c>: " << m_level->jumps
        << "\n<co>Clicks (best att.)</c>: " << m_level->clicks // the contents of this variable make no sense to me
        << "\n<co>Time (best att.)</c>: " << TimeUtils::workingTime(m_level->attemptTime) // the contents of this variable make no sense to me
        //<< "\n<co>Is legit</c>: " << m_level->isCompletionLegitimate // the contents of this variable make no sense to me
        << "\n<cp>Normal</c>: " << m_level->normalPercent
        << "%\n<co>Practice</c>: " << m_level->practicePercent << "%";

    if(m_level->orbCompletion != m_level->newNormalPercent2) infoText << "\n<cj>2.1 Normal</c>: " << m_level->orbCompletion << "%";
    if(m_level->newNormalPercent2 != m_level->normalPercent) infoText << "\n<cr>2.11 Normal</c>: " << m_level->newNormalPercent2 << "%";*/
}

void ExtendedLevelInfo::setupAdditionalInfo() {
    m_uploadDateEstimated = BetterInfoCache::sharedState()->getUploadDate(m_level->m_levelID, this);
    retain();

    std::thread([this]() {
        std::string levelString(BetterInfo::decodeBase64Gzip(m_level->m_levelString));
        m_objectsEstimated = std::count(levelString.begin(), levelString.end(), ';');
        m_fileSizeCompressed = BetterInfo::fileSize(m_level->m_levelString.size());
        m_fileSizeUncompressed = BetterInfo::fileSize(levelString.size());
        refreshInfoTexts();
        Loader::get()->queueInMainThread([this]() {
            this->loadPage(this->m_page);
            release();
        });
    }).detach();
}

bool ExtendedLevelInfo::init(GJGameLevel* level){
    if(!CvoltonAlertLayerStub::init({440.0f, 290.0f})) return false;

    level->retain();
    this->m_level = level;

    auto levelName = CCLabelBMFont::create(m_level->m_levelName.c_str(), "bigFont.fnt");
    auto levelNameBtn = CCMenuItemSpriteExtra::create(
        levelName,
        this,
        menu_selector(ExtendedLevelInfo::onCopyName)
    );
    m_buttonMenu->addChild(levelNameBtn);
    levelNameBtn->setPosition({0,125});
    levelNameBtn->setID("level-name-button"_spr);

    std::ostringstream userNameText;
    userNameText << "By " << std::string(m_level->m_creatorName);
    auto userName = CCLabelBMFont::create(userNameText.str().c_str(), "goldFont.fnt");
    userName->setScale(0.8f);
    auto userNameBtn = CCMenuItemSpriteExtra::create(
        userName,
        this,
        menu_selector(ExtendedLevelInfo::onCopyAuthor)
    );
    userNameBtn->setPosition({0,99});
    userNameBtn->setID("username-button"_spr);
    m_buttonMenu->addChild(userNameBtn);

    cocos2d::extension::CCScale9Sprite* descBg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    descBg->setContentSize({340,55});
    descBg->setColor({130,64,33});
    m_buttonMenu->addChild(descBg, -1);
    descBg->setPosition({0,52});
    descBg->setID("description-background"_spr);

    auto descText = BetterInfo::fixNullByteCrash(BetterInfo::fixColorCrashes(m_level->getUnpackedLevelDescription()));
    size_t descLength = descText.length();
    float descDelimiter = 1;
    if(descLength > 140) descLength = 140;
    if(descLength > 70) descDelimiter = ((((140 - descLength) / 140) * 0.3f) + 0.7f);
    auto description = TextArea::create(descText, "chatFont.fnt", 1, 295 / descDelimiter, {0.5f,0.5f}, 20, false);
    description->setScale(descDelimiter);
    description->setAnchorPoint({1,1});
    description->setPosition( ( (description->getContentSize() / 2 ) * descDelimiter ) + (CCPoint(340,55) / 2) );
    auto descSprite = CCSprite::create();
    descSprite->addChild(description);
    descSprite->setContentSize({340,55});
    auto descBtn = CCMenuItemSpriteExtra::create(
        descSprite,
        this,
        menu_selector(ExtendedLevelInfo::onCopyDesc)
    );
    descBtn->setPosition({0,52});
    descBtn->setID("description-button"_spr);
    m_buttonMenu->addChild(descBtn);

    cocos2d::extension::CCScale9Sprite* infoBg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    infoBg->setContentSize({340,148});
    //infoBg->setColor({130,64,33});
    //infoBg->setColor({191,114,62});
    infoBg->setColor({123,60,31});
    m_buttonMenu->addChild(infoBg, -1);
    infoBg->setPosition({0,-57});
    infoBg->setID("info-background"_spr);

    refreshInfoTexts();
    setupAdditionalInfo();

    m_info = TextArea::create(m_primary, "chatFont.fnt", 1, 170, {0,1}, 20, false);
    m_info->setPosition({-160.5,26});
    //m_info->setPosition({-160.5,10});
    m_info->setAnchorPoint({0,1});
    m_info->setScale(0.925f);
    m_info->setID("info-text"_spr);
    m_buttonMenu->addChild(m_info);

    /*std::ostringstream progressText;
    if(m_level->personalBests != "") progressText << "\n\n<cy>Progresses</c>:\n" << printableProgress(m_level->personalBests, m_level->newNormalPercent2);
    auto progress = TextArea::create("chatFont.fnt", false, progressText.str(), 0.8f, 130, 20, {0,1});
    progress->setPosition({12,50});
    progress->setAnchorPoint({0,1});
    m_buttonMenu->addChild(progress);*/

    /*std::ostringstream uploadedText;
    uploadedText << "Uploaded: " << m_level->uploadDate << " ago";
    createTextLabel(uploadedText.str(), {0,0}, 0.5f, m_buttonMenu);*/

    auto requestedRate = createTextLabel("Requested Rate:", {88,-1}, 0.5f, m_buttonMenu);
    requestedRate->setID("requested-rate-label"_spr);

    auto diffSprite = CCSprite::createWithSpriteFrameName(LevelMetadata::getDifficultyIcon(m_level->m_starsRequested));
    diffSprite->setPosition({88,-57});
    diffSprite->setID("difficulty-sprite"_spr);
    m_buttonMenu->addChild(diffSprite, 1);

    if(m_level->m_starsRequested > 0){
        auto featureSprite = CCSprite::createWithSpriteFrameName("GJ_featuredCoin_001.png");
        featureSprite->setPosition({88,-57});
        featureSprite->setID("featured-sprite"_spr);
        m_buttonMenu->addChild(featureSprite);
        //infoSprite->setScale(0.7f);

        auto starsLabel = createTextLabel(std::to_string(m_level->m_starsRequested), {88, -87}, 0.4f, m_buttonMenu);
        starsLabel->setAnchorPoint({1,0.5});
        starsLabel->setID("stars-label"_spr);

        auto diffSprite = CCSprite::createWithSpriteFrameName(m_level->isPlatformer() ? "moon_small01_001.png" : "star_small01_001.png");
        diffSprite->setPosition({95,-87});
        diffSprite->setID("stars-sprite"_spr);
        m_buttonMenu->addChild(diffSprite);
    }

    /*
        thanks to Alphalaneous for quick UI improvement concept lol
    */

    auto separator = CCSprite::createWithSpriteFrameName("floorLine_001.png");
    separator->setPosition({6,-57});
    separator->setScaleX(0.3f);
    separator->setScaleY(1);
    separator->setOpacity(100);
    separator->setRotation(90);
    separator->setID("separator-sprite"_spr);
    m_buttonMenu->addChild(separator);

    /*
        next/prev page btn
    */
    auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    prevSprite->setScale(.8f);
    m_prevBtn = CCMenuItemSpriteExtra::create(
        prevSprite,
        this,
        menu_selector(ExtendedLevelInfo::onPrev)
    );
    m_prevBtn->setPosition({-195,-53});
    m_prevBtn->setID("prev-button"_spr);
    m_buttonMenu->addChild(m_prevBtn);

    auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextSprite->setFlipX(true);
    nextSprite->setScale(.8f);
    m_nextBtn = CCMenuItemSpriteExtra::create(
        nextSprite,
        this,
        menu_selector(ExtendedLevelInfo::onNext)
    );
    m_nextBtn->setPosition({195,-53});
    m_nextBtn->setID("next-button"_spr);
    m_buttonMenu->addChild(m_nextBtn);

    loadPage(0);

    return true;
}

CCLabelBMFont* ExtendedLevelInfo::createTextLabel(const std::string text, const CCPoint& position, const float scale, CCNode* menu, const char* font){
    CCLabelBMFont* bmFont = CCLabelBMFont::create(text.c_str(), font);
    bmFont->setPosition(position);
    bmFont->setScale(scale);
    menu->addChild(bmFont);
    return bmFont;
}