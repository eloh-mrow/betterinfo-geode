#pragma once
#include <deque>
#include "../../delegates/BISearchObjectDelegate.h"
#include "../../objects/BISearchObject.h"
#include "../_bases/BIViewLayer.h"
#include <Geode/Bindings.hpp>

using namespace geode::prelude;

class LevelSearchViewLayer : public BIViewLayer, public LevelManagerDelegate, public BISearchObjectDelegate {
    BISearchObject m_searchObj;
    GJSearchObject* m_gjSearchObj = nullptr;
    GJSearchObject* m_gjSearchObjOptimized = nullptr;
    GJSearchObject* m_gjSearchObjLoaded = nullptr;
    cocos2d::CCLabelBMFont* m_statusText = nullptr;
    std::deque<GJGameLevel*> m_unloadedLevels;
    std::deque<GJGameLevel*> m_allLevels;
    size_t m_shownLevels = 0;
    size_t m_firstIndex = 0;
    size_t m_lastIndex = 0;
    size_t m_totalAmount = 0;
protected:
    virtual bool init(std::deque<GJGameLevel*> allLevels, BISearchObject searchObj = BISearchObject());
    virtual bool init(GJSearchObject* gjSearchObj, BISearchObject searchObj = BISearchObject());
    virtual bool init(BISearchObject searchObj = BISearchObject());
    virtual void keyBackClicked();
    virtual void updateCounter();
    void onBack(cocos2d::CCObject*);
    void unload();
    void reload();
    void startLoading();
    void setTextStatus(bool finished);
    int resultsPerPage() const;
public:
    void loadPage(bool reload);
    void loadPage();
    static LevelSearchViewLayer* create(std::deque<GJGameLevel*> allLevels, BISearchObject searchObj = BISearchObject());
    static LevelSearchViewLayer* create(GJSearchObject* gjSearchObj, BISearchObject searchObj = BISearchObject());
    static cocos2d::CCScene* scene(std::deque<GJGameLevel*> allLevels, BISearchObject searchObj = BISearchObject());
    static cocos2d::CCScene* scene(GJSearchObject* gjSearchObj, BISearchObject searchObj = BISearchObject());
    
    void onFilters(cocos2d::CCObject*);
    void onInfo(cocos2d::CCObject*);

    void loadLevelsFinished(cocos2d::CCArray*, const char*);
    void loadLevelsFailed(const char*);
    void setupPageInfo(gd::string, const char*);

    void onSearchObjectFinished(const BISearchObject& searchObj);
    void showInfoDialog();

    void optimizeSearchObject();
    void resetUnloadedLevels();

    void queueLoad(float dt);

    void onEnter();
};