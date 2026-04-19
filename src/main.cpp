#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Notification.hpp>
#include "HKGDManager.hpp"
#include "HKGDVictorsPopup.hpp"

using namespace geode::prelude;

// LevelInfoLayer modification - Show button for EXTREME CLASSIC demons
class $modify(HKGDLevelInfoLayer, LevelInfoLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* hkgdButton = nullptr;
        int hkgdPosition = -1;
        bool hasCheckedHKGDL = false;
        bool isOnHKGDL = false;
        bool isExtremeClassic = false;
        bool isPlatformerExtreme = false;
    };
    
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) {
            return false;
        }
        
        // Check if this is an extreme classic demon
        m_fields->isExtremeClassic = HKGDManager::get()->isExtremeClassicDemon(level);
        
        // Check if this is ANY platformer level (not just extreme)
        m_fields->isPlatformerExtreme = HKGDManager::get()->isPlatformerLevel(level);
        
        if (m_fields->isExtremeClassic || m_fields->isPlatformerExtreme) {
            // Create the HKGD button for extreme classic demons OR any platformer levels
            createHKGDButton();
            
            // Also check if it's on HKGDL to show position
            auto levelId = level->m_levelID.value();
            bool isPlatformer = m_fields->isPlatformerExtreme;
            HKGDManager::get()->checkLevelOnHKGDL(levelId, isPlatformer, [this](bool isOnHKGDL, int position) {
                m_fields->hasCheckedHKGDL = true;
                m_fields->isOnHKGDL = isOnHKGDL;
                m_fields->hkgdPosition = position;
                this->updateHKGDLabel();
            });
        }
        
        return true;
    }
    
    void createHKGDButton() {
        if (m_fields->hkgdButton) return;
        
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        // Create button sprite using ButtonSprite
        auto buttonSprite = ButtonSprite::create("HKGD", "bigFont.fnt", "GJ_button_04.png", 0.8f);
        buttonSprite->setScale(0.6f);
        
        // Create button
        m_fields->hkgdButton = CCMenuItemSpriteExtra::create(
            buttonSprite,
            this,
            menu_selector(HKGDLevelInfoLayer::onHKGDButton)
        );
        
        // Position button under the difficulty sprite
        float buttonX, buttonY;
        
        if (m_difficultySprite) {
            auto diffPos = m_difficultySprite->getPosition();
            buttonX = diffPos.x;
            buttonY = diffPos.y - m_difficultySprite->getContentSize().height / 2 - 30.f;
        } else if (m_starsIcon) {
            auto starsPos = m_starsIcon->getPosition();
            buttonX = starsPos.x;
            buttonY = starsPos.y - 30.f;
        } else {
            buttonX = winSize.width / 2 - 100.f;
            buttonY = winSize.height / 2 - 60.f;
        }
        
        m_fields->hkgdButton->setPosition({buttonX, buttonY});
        
        // Add to the layer
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        menu->setTag(9999); // Tag for easy identification
        menu->addChild(m_fields->hkgdButton);
        this->addChild(menu);
        
        // Add position indicator label (will be updated later)
        auto posLabel = CCLabelBMFont::create("Loading...", "goldFont.fnt");
        posLabel->setScale(0.35f);
        posLabel->setPosition({buttonX, buttonY - 20.f});
        posLabel->setZOrder(10);
        posLabel->setTag(10000); // Tag for updating later
        this->addChild(posLabel);
    }
    
    void updateHKGDLabel() {
        auto posLabel = static_cast<CCLabelBMFont*>(this->getChildByTag(10000));
        if (!posLabel) return;
        
        if (m_fields->isOnHKGDL) {
            posLabel->setString(fmt::format("HKGD #{}", m_fields->hkgdPosition).c_str());
            posLabel->setColor({255, 215, 0}); // Gold color
        } else {
            posLabel->setString("Not on HKGDL");
            posLabel->setColor({150, 150, 150}); // Gray color
        }
    }
    
    void onHKGDButton(CCObject* sender) {
        auto level = m_level;
        if (!level) return;
        
        auto popup = HKGDVictorsPopup::create(
            level->m_levelID.value(),
            level->m_levelName,
            m_fields->hkgdPosition,
            m_fields->isOnHKGDL,
            m_fields->isPlatformerExtreme
        );
        
        if (popup) {
            popup->createUI();
            popup->show();
        }
    }
};

// ProfilePage modification - Show HKGD stats on player profiles
class $modify(HKGDProfilePage, ProfilePage) {
    struct Fields {
        CCLabelBMFont* hkgdStatsLabel = nullptr;
        std::string hkgdStatsText;
        int hkgdAccountId = 0;
        bool m_hkgdButtonAdded = false;
    };

    void show() {
        ProfilePage::show();

        geode::log::info("HKGD: show() called");
    }

    void getUserInfoFinished(GJUserScore* score) {
        ProfilePage::getUserInfoFinished(score);

        geode::log::info("HKGD: getUserInfoFinished called, score={}, buttonAdded={}", (void*)score, m_fields->m_hkgdButtonAdded);

        if (!score || m_fields->m_hkgdButtonAdded) return;

        auto accountName = std::string(score->m_userName);
        auto accountId = score->m_accountID;

        geode::log::info("HKGD: accountName={}, mainLayer={}", accountName, (void*)m_mainLayer);

        if (!m_mainLayer) return;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Create HKGD button
        auto hkgdBtnSprite = ButtonSprite::create("HKGD", "bigFont.fnt", "GJ_button_04.png", 0.8f);

        geode::log::info("HKGD: btnSprite={}", (void*)hkgdBtnSprite);

        if (!hkgdBtnSprite) return;

        hkgdBtnSprite->setScale(0.5f);

        auto hkgdBtn = CCMenuItemSpriteExtra::create(
            hkgdBtnSprite,
            this,
            menu_selector(HKGDProfilePage::onHKGDStats)
        );

        geode::log::info("HKGD: btn={}", (void*)hkgdBtn);

        if (!hkgdBtn) return;

        // Position at top center (was visible here)
        hkgdBtn->setPosition({90.f, winSize.height / 2 - 50.f});

        // Create menu and add button
        auto menu = CCMenu::create();
        menu->addChild(hkgdBtn);
        menu->setPosition({0, 0});
        menu->setTag(9999);

        // Add to main layer with high z-order
        m_mainLayer->addChild(menu, 1000);

        geode::log::info("HKGD: Button added successfully at position y={}", winSize.height / 2 - 60.f);

        m_fields->m_hkgdButtonAdded = true;

        // Store the username and account ID for later use
        m_fields->hkgdStatsText = accountName;
        m_fields->hkgdAccountId = accountId;
    }
    
    void onHKGDStats(CCObject* sender) {
        // Show player stats popup with name mapping
        auto popup = HKGDPlayerStatsPopup::create(m_fields->hkgdStatsText, m_fields->hkgdAccountId);
        if (popup) {
            popup->createUI();
            popup->show();
        }
    }
};
