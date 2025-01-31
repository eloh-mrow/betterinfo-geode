#include <Geode/Bindings.hpp>
#include <Geode/modify/CommentCell.hpp>

#include "../layers/UnregisteredProfileLayer.h"
#include "../objects/FLAlertResultHandler.h"

using namespace geode::prelude;

class $modify(BICommentCell, CommentCell) {
    bool retained = false;

    static void onModify(auto& self) {
        auto res = self.setHookPriority("CommentCell::onLike", 99999);
    }

    /*
     * Callbacks
     */
    void onProfilePage(CCObject* sender){
        this->m_comment->m_userScore->m_userID = this->m_comment->m_userID;
        UnregisteredProfileLayer::create(this->m_comment->m_userScore, this->getParent()->getParent()->getParent()->getParent()->getParent()->getParent())->show();
    }

    void onLevelInfoNoLoad(CCObject* sender){
        auto level = GJGameLevel::create();
        level->m_levelID = this->m_comment->m_levelID;

        InfoLayer* infoLayer = InfoLayer::create(level, nullptr, nullptr);
        infoLayer->m_scene = this->getParent()->getParent()->getParent()->getParent()->getParent()->getParent();
        infoLayer->show();
    }

    /*
     * Hooks
     */

    void loadFromComment(GJComment* b) {
        CommentCell::loadFromComment(b);

        if(!Mod::get()->setSavedValue("comment-id-alert-shown", true)) FLAlertLayer::create(FLAlertResultHandler::create("show-comment-ids", true), Mod::get()->getName().c_str(), "Would you like to show comment IDs next to comments?", "No", "Yes")->show();

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        bool smallCommentsMode = this->m_height == 36; //this is how robtop does the check

        /**
         * Add comment ID
         */
        if(Mod::get()->getSettingValue<bool>("show-comment-ids")) {
            auto idText = CCLabelBMFont::create(fmt::format("#{}", b->m_commentID).c_str(), "chatFont.fnt");
            idText->setPosition(smallCommentsMode ? CCPoint(332, 12.5) : CCPoint(329, 21.5));
            idText->setAnchorPoint({1, .5f});
            idText->setScale(smallCommentsMode ? .325f : .425f);
            idText->setOpacity(110);
            idText->setColor({0,0,0});
            idText->setID("comment-id"_spr);
            m_mainLayer->addChild(idText);
        }

        if(auto menu = static_cast<CCMenu*>(m_mainLayer->getChildByID("main-menu"))) {
            /**
             * Add comments InfoLayer button
             */
            if(b->m_hasLevelID) {
                auto commentsSprite = CCSprite::createWithSpriteFrameName("GJ_chatBtn_001.png");
                commentsSprite->setScale(smallCommentsMode ? 0.35f : 0.5f);
                auto commentsButton = CCMenuItemSpriteExtra::create(
                    commentsSprite,
                    this,
                    menu_selector(BICommentCell::onLevelInfoNoLoad)
                );

                CCPoint smallPosition(206 - (winSize.width / 2), 24.5f - (winSize.height / 2));
                CCPoint largePosition(154 - (winSize.width / 2), 60 - (winSize.height / 2));
                commentsButton->setPosition(smallCommentsMode ? smallPosition : largePosition);
                commentsButton->setSizeMult(1.2f);
                commentsButton->setID("comments-button"_spr);

                menu->addChild(commentsButton);
            }

            /**
             * Add username button
            */
            if(b->m_accountID == 0) if(auto usernameLabel = static_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("username-label"))) {
                auto content = std::string_view(usernameLabel->getString());
                if(content == "" || content == "-") {
                    usernameLabel->setString(fmt::format("- (ID: {})", b->m_userID).c_str());
                }

                CCPoint position = menu->convertToNodeSpace(m_mainLayer->convertToWorldSpace(usernameLabel->getPosition()));
                usernameLabel->removeFromParent();


                auto buttonButton = CCMenuItemSpriteExtra::create(
                    usernameLabel,
                    this,
                    menu_selector(BICommentCell::onProfilePage)
                );
                buttonButton->setSizeMult(1.2f);
                buttonButton->setPosition(position + CCPoint(usernameLabel->getScaledContentSize().width / 2, 0));
                buttonButton->setEnabled(true);
                buttonButton->setID("username-button"); //using vanilla style node ID because this is a replacement for a vanilla feature

                menu->addChild(buttonButton);
            }

            
        }

    }

    //The following CommentCell functions are required as fixes for crashes caused by refreshing the layer while liking/deleting an item
    void onConfirmDelete(CCObject* sender) {
        this->retain();
        m_fields->retained = true;
        CommentCell::onConfirmDelete(sender);
    }

    void FLAlert_Clicked(FLAlertLayer* layer, bool btn) {
        CommentCell::FLAlert_Clicked(layer, btn);

        if(m_fields->retained) this->release();
        m_fields->retained = false;
    }

    //This function is simple enough that it's easier to rewrite it than to figure out how midhooks work
    void onLike(CCObject* sender) {
        if(!this->m_comment) return;

        LikeItemType type = LikeItemType::Comment;
        int special = this->m_comment->m_levelID;

        if (this->m_accountComment) {
            special = this->m_comment->m_accountID;
            type = LikeItemType::AccountComment;
        }

        bool liked = GameLevelManager::sharedState()->hasLikedItemFullCheck(type, this->m_comment->m_commentID, special);
        if(liked) return;

        auto GM = GameManager::sharedState();
        if (this->m_comment->m_userID == GM->m_playerUserID) return;
        
        auto AM = GJAccountManager::sharedState();
        if (this->m_comment->m_accountID && this->m_comment->m_accountID == AM->m_accountID) return;

        auto layer = LikeItemLayer::create(type, this->m_comment->m_commentID, special);
        layer->m_likeDelegate = this;
        layer->show();
        this->retain();
        m_fields->retained = true;
    }

    void likedItem(LikeItemType type, int id, int special) {
        CommentCell::likedItem(type, id, special);

        if(m_fields->retained) this->release();
        m_fields->retained = false;
    }
};