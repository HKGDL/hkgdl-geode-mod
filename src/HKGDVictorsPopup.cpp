#include "HKGDVictorsPopup.hpp"
#include <Geode/ui/Notification.hpp>
#include <chrono>

// HKGDVictorsPopup implementation

bool HKGDVictorsPopup::init(int levelId, const std::string& levelName, int position, bool isPlatformer) {
    m_levelId = levelId;
    m_levelName = levelName;
    m_position = position;
    m_isPlatformer = isPlatformer;
    
    return geode::Popup::init(400.f, 300.f);
}

HKGDVictorsPopup* HKGDVictorsPopup::create(int levelId, const std::string& levelName, int position, bool isOnHKGDL, bool isPlatformer) {
    auto ret = new HKGDVictorsPopup();
    ret->m_isOnHKGDL = isOnHKGDL;
    ret->m_isPlatformer = isPlatformer;
    if (ret->init(levelId, levelName, position, isPlatformer)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void HKGDVictorsPopup::createUI() {
    // Title
    setTitle(fmt::format("HKGD - {}", m_levelName).c_str());
    
    // Position label
    std::string posText = m_isOnHKGDL ? fmt::format("HKGD Rank: #{}", m_position) : "Not on HKGD List";
    auto posLabel = CCLabelBMFont::create(posText.c_str(), m_isOnHKGDL ? "goldFont.fnt" : "bigFont.fnt");
    posLabel->setPosition({m_size.width / 2, m_size.height - 40.f});
    posLabel->setScale(0.5f);
    m_mainLayer->addChild(posLabel);
    
    // Button menu
    auto buttonMenu = CCMenu::create();
    buttonMenu->setPosition({m_size.width / 2, m_size.height - 65.f});
    m_mainLayer->addChild(buttonMenu);
    
    // Submit Record button (for extreme classic demons)
    auto submitBtnSprite = ButtonSprite::create("Submit Record", "bigFont.fnt", "GJ_button_01.png", 0.8f);
    submitBtnSprite->setScale(0.55f);
    auto submitBtn = CCMenuItemSpriteExtra::create(
        submitBtnSprite,
        this,
        menu_selector(HKGDVictorsPopup::onSubmitRecord)
    );
    submitBtn->setPosition({0.f, 0.f});
    buttonMenu->addChild(submitBtn);
    
    // Loading circle
    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setPosition({m_size.width / 2, m_size.height / 2 - 20.f});
    m_loadingCircle->setVisible(true);
    m_mainLayer->addChild(m_loadingCircle);
    
    // Start loading victors if on HKGDL
    if (m_isOnHKGDL) {
        HKGDManager::get()->fetchVictors(m_levelId, [this](std::vector<HKGDRecord> records) {
            this->onLoadingFinished(records);
        });
    } else {
        onLoadingFinished({});
    }
}

void HKGDVictorsPopup::onSubmitRecord(CCObject* sender) {
    auto popup = HKGDSubmitPopup::create(m_levelId, m_levelName, m_isPlatformer);
    if (popup) {
        popup->show();
    }
}

void HKGDVictorsPopup::onLoadingFinished(std::vector<HKGDRecord> records) {
    m_records = records;
    
    if (m_loadingCircle) {
        m_loadingCircle->setVisible(false);
        m_loadingCircle->removeFromParent();
        m_loadingCircle = nullptr;
    }
    
    if (!m_isOnHKGDL) {
        auto noRecords = CCLabelBMFont::create(
            "This level is not on HKGDL yet.\nYou can submit your record!",
            "bigFont.fnt"
        );
        noRecords->setPosition({m_size.width / 2, m_size.height / 2 - 20.f});
        noRecords->setScale(0.4f);
        noRecords->setAlignment(kCCTextAlignmentCenter);
        m_mainLayer->addChild(noRecords);
        return;
    }
    
    if (m_records.empty()) {
        auto noRecords = CCLabelBMFont::create("No victors found yet.\nBe the first!", "bigFont.fnt");
        noRecords->setPosition({m_size.width / 2, m_size.height / 2 - 20.f});
        noRecords->setScale(0.5f);
        noRecords->setAlignment(kCCTextAlignmentCenter);
        m_mainLayer->addChild(noRecords);
        return;
    }
    
    createVictorsList();
}

void HKGDVictorsPopup::createVictorsList() {
    float listWidth = 360.f;
    float listHeight = 180.f;
    float cellHeight = 25.f;
    
    // Calculate total content height
    float totalHeight = (m_records.size() + 1) * cellHeight; // +1 for header
    
    // Create scroll layer
    auto scrollLayer = ScrollLayer::create({listWidth, listHeight});
    scrollLayer->setPosition({(m_size.width - listWidth) / 2, 15.f});
    m_mainLayer->addChild(scrollLayer);
    m_scrollView = scrollLayer;
    
    // Content layer
    auto contentLayer = scrollLayer->m_contentLayer;
    
    // Header row
    auto header = CCLabelBMFont::create("Rank   Player                                      Date        Att    Vid", "bigFont.fnt");
    header->setAnchorPoint({0, 0.5f});
    header->setPosition({10.f, totalHeight - cellHeight / 2});
    header->setScale(0.27f);
    header->setColor({200, 200, 200});
    contentLayer->addChild(header);
    
    // Add victor cells
    for (size_t i = 0; i < m_records.size(); i++) {
        auto cell = createVictorCell(m_records[i], static_cast<int>(i));
        cell->setPosition({listWidth / 2, totalHeight - (static_cast<float>(i) + 1.5f) * cellHeight});
        contentLayer->addChild(cell);
    }
    
    // Set content layer size
    contentLayer->setContentSize({listWidth, totalHeight});
    scrollLayer->scrollToTop();
}

CCNode* HKGDVictorsPopup::createVictorCell(HKGDRecord const& record, int index) {
    float cellHeight = 25.f;
    
    auto cell = CCNode::create();
    cell->setContentSize({360.f, cellHeight});
    
    // Background
    ccColor4B bgColor = index % 2 == 0 ? ccc4(50, 50, 50, 100) : ccc4(30, 30, 30, 100);
    auto bg = CCLayerColor::create(bgColor, 360.f, cellHeight);
    bg->setPosition({-180.f, -cellHeight / 2});
    cell->addChild(bg);
    
    // Rank (x = -170)
    auto rankLabel = CCLabelBMFont::create(fmt::format("#{}", record.rank).c_str(), "bigFont.fnt");
    rankLabel->setAnchorPoint({0, 0.5f});
    rankLabel->setPosition({-170.f, 0});
    rankLabel->setScale(0.3f);
    
    // Color based on rank
    if (record.rank == 1) {
        rankLabel->setColor({255, 215, 0}); // Gold
    } else if (record.rank == 2) {
        rankLabel->setColor({192, 192, 192}); // Silver
    } else if (record.rank == 3) {
        rankLabel->setColor({205, 127, 50}); // Bronze
    }
    cell->addChild(rankLabel);
    
    // Username (x = -130)
    std::string username = record.username;
    if (username.length() > 12) {
        username = username.substr(0, 9) + "...";
    }
    auto usernameLabel = CCLabelBMFont::create(username.c_str(), "bigFont.fnt");
    usernameLabel->setAnchorPoint({0, 0.5f});
    usernameLabel->setPosition({-130.f, 0});
    usernameLabel->setScale(0.3f);
    cell->addChild(usernameLabel);
    
    // Date (x = 40)
    auto dateLabel = CCLabelBMFont::create(record.date.c_str(), "bigFont.fnt");
    dateLabel->setAnchorPoint({0, 0.5f});
    dateLabel->setPosition({40.f, 0});
    dateLabel->setScale(0.28f);
    cell->addChild(dateLabel);
    
    // Attempts (x = 115) - always show, even if 0
    std::string attemptsText = record.attempts > 0 ? std::to_string(record.attempts) : "-";
    auto attemptsLabel = CCLabelBMFont::create(attemptsText.c_str(), "bigFont.fnt");
    attemptsLabel->setAnchorPoint({0, 0.5f});
    attemptsLabel->setPosition({115.f, 0});
    attemptsLabel->setScale(0.28f);
    attemptsLabel->setColor({150, 150, 150});
    cell->addChild(attemptsLabel);
    
    // Video button (x = 160)
    if (!record.videoUrl.empty()) {
        auto videoBtnSprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
        if (videoBtnSprite) {
            videoBtnSprite->setScale(0.3f);
            auto videoBtn = CCMenuItemSpriteExtra::create(
                videoBtnSprite,
                this,
                menu_selector(HKGDVictorsPopup::onVideoButton)
            );
            videoBtn->setPosition({155.f, 0});
            videoBtn->setUserObject(CCString::create(record.videoUrl));
            
            auto menu = CCMenu::create();
            menu->addChild(videoBtn);
            menu->setPosition({0, 0});
            cell->addChild(menu);
        }
    }
    
    return cell;
}

void HKGDVictorsPopup::onVideoButton(CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto urlStr = static_cast<CCString*>(btn->getUserObject());
    if (urlStr) {
        CCApplication::sharedApplication()->openURL(urlStr->getCString());
    }
}

// HKGDSubmitPopup implementation

bool HKGDSubmitPopup::init(int levelId, const std::string& levelName, bool isPlatformer) {
    m_levelId = levelId;
    m_levelName = levelName;
    m_isPlatformer = isPlatformer;
    
    if (!geode::Popup::init(350.f, 280.f)) {
        return false;
    }
    
    // Update title to indicate platformer if needed
    std::string title = fmt::format("Submit Record - {}", m_levelName);
    if (isPlatformer) {
        title += " (Platformer)";
    }
    setTitle(title.c_str());
    
    float startY = m_size.height - 60.f;
    float spacing = 40.f;
    float inputWidth = 280.f;
    
    // Username input
    auto usernameLabel = CCLabelBMFont::create("Username:", "bigFont.fnt");
    usernameLabel->setPosition({40.f, startY});
    usernameLabel->setScale(0.4f);
    usernameLabel->setAnchorPoint({0, 0.5f});
    m_mainLayer->addChild(usernameLabel);
    
    m_usernameInput = TextInput::create(inputWidth, "Your GD username");
    m_usernameInput->setPosition({m_size.width / 2, startY - 20.f});
    m_usernameInput->setScale(0.8f);
    m_mainLayer->addChild(m_usernameInput);
    
    // Attempts input
    auto attemptsLabel = CCLabelBMFont::create("Attempts:", "bigFont.fnt");
    attemptsLabel->setPosition({40.f, startY - spacing});
    attemptsLabel->setScale(0.4f);
    attemptsLabel->setAnchorPoint({0, 0.5f});
    m_mainLayer->addChild(attemptsLabel);
    
    m_attemptsInput = TextInput::create(inputWidth, "Number of attempts");
    m_attemptsInput->setPosition({m_size.width / 2, startY - spacing - 20.f});
    m_attemptsInput->setScale(0.8f);
    m_attemptsInput->setFilter("0123456789");
    m_mainLayer->addChild(m_attemptsInput);
    
    // Video URL input
    auto videoLabel = CCLabelBMFont::create("Video URL:", "bigFont.fnt");
    videoLabel->setPosition({40.f, startY - spacing * 2});
    videoLabel->setScale(0.4f);
    videoLabel->setAnchorPoint({0, 0.5f});
    m_mainLayer->addChild(videoLabel);
    
    m_videoUrlInput = TextInput::create(inputWidth, "YouTube/video link (required)");
    m_videoUrlInput->setPosition({m_size.width / 2, startY - spacing * 2 - 20.f});
    m_videoUrlInput->setScale(0.8f);
    m_mainLayer->addChild(m_videoUrlInput);
    
    // FPS input
    auto fpsLabel = CCLabelBMFont::create("FPS:", "bigFont.fnt");
    fpsLabel->setPosition({40.f, startY - spacing * 3});
    fpsLabel->setScale(0.4f);
    fpsLabel->setAnchorPoint({0, 0.5f});
    m_mainLayer->addChild(fpsLabel);
    
    m_fpsInput = TextInput::create(inputWidth, "FPS (e.g., 60, 144, 240)");
    m_fpsInput->setPosition({m_size.width / 2, startY - spacing * 3 - 20.f});
    m_fpsInput->setScale(0.8f);
    m_fpsInput->setFilter("0123456789");
    m_mainLayer->addChild(m_fpsInput);
    
    // Submit button
    auto submitBtnSprite = ButtonSprite::create("Submit", "bigFont.fnt", "GJ_button_01.png", 1.0f);
    submitBtnSprite->setScale(0.8f);
    auto submitBtn = CCMenuItemSpriteExtra::create(
        submitBtnSprite,
        this,
        menu_selector(HKGDSubmitPopup::onSubmit)
    );
    
    auto menu = CCMenu::create();
    menu->setPosition({m_size.width / 2, 35.f});
    menu->addChild(submitBtn);
    m_mainLayer->addChild(menu);
    
    return true;
}

HKGDSubmitPopup* HKGDSubmitPopup::create(int levelId, const std::string& levelName, bool isPlatformer) {
    auto ret = new HKGDSubmitPopup();
    if (ret->init(levelId, levelName, isPlatformer)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void HKGDSubmitPopup::onSubmit(CCObject* sender) {
    auto username = m_usernameInput->getString();
    auto attemptsStr = m_attemptsInput->getString();
    auto videoUrl = m_videoUrlInput->getString();
    auto fps = m_fpsInput->getString();
    
    // Validate
    if (username.empty()) {
        Notification::create("Please enter your username", NotificationIcon::Warning, 2.f)->show();
        return;
    }
    
    if (videoUrl.empty()) {
        Notification::create("Video URL is required", NotificationIcon::Warning, 2.f)->show();
        return;
    }
    
    int attempts = 0;
    if (!attemptsStr.empty()) {
        try {
            attempts = std::stoi(attemptsStr);
        } catch (...) {
            Notification::create("Invalid attempts number", NotificationIcon::Warning, 2.f)->show();
            return;
        }
    }
    
    // Get current date
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);
    char dateBuffer[20];
    std::strftime(dateBuffer, sizeof(dateBuffer), "%Y/%m/%d", tm);
    std::string date(dateBuffer);
    
    // Show loading
    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setPosition({m_size.width / 2, m_size.height / 2});
    m_loadingCircle->setVisible(true);
    m_mainLayer->addChild(m_loadingCircle);
    
    // Submit to API - use platformer method if it's a platformer level
    if (m_isPlatformer) {
        HKGDManager::get()->submitPlatformerRecord(m_levelId, m_levelName, username, attempts, videoUrl, fps, date, 
            [this](HKGDSubmissionResult result) {
                this->handleSubmitResult(result);
            }
        );
    } else {
        // Classic demon submission
        HKGDManager::get()->submitRecord(m_levelId, m_levelName, username, attempts, videoUrl, fps, date, 
            [this](HKGDSubmissionResult result) {
                this->handleSubmitResult(result);
            }
        );
    }
}

void HKGDSubmitPopup::handleSubmitResult(HKGDSubmissionResult result) {
    // Hide loading
    if (m_loadingCircle) {
        m_loadingCircle->setVisible(false);
        m_loadingCircle->removeFromParent();
        m_loadingCircle = nullptr;
    }
    
    if (result.success) {
        Notification::create(result.message, NotificationIcon::Success, 3.f)->show();
        // Close popup
        this->onClose(nullptr);
    } else {
        Notification::create(result.message, NotificationIcon::Error, 3.f)->show();
    }
}

void HKGDSubmitPopup::onSubmitComplete(HKGDSubmissionResult result) {
    if (m_loadingCircle) {
        m_loadingCircle->setVisible(false);
        m_loadingCircle->removeFromParent();
        m_loadingCircle = nullptr;
    }
    
    if (result.success) {
        Notification::create(result.message, NotificationIcon::Success, 3.f)->show();
        this->onClose(nullptr);
    } else {
        Notification::create(result.message, NotificationIcon::Error, 3.f)->show();
    }
}

// HKGDListPopup implementation

bool HKGDListPopup::init() {
    if (!geode::Popup::init(420.f, 320.f)) {
        return false;
    }
    
    setTitle("HKGD Demon List");
    
    // Loading circle
    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setPosition({m_size.width / 2, m_size.height / 2});
    m_loadingCircle->setVisible(true);
    m_mainLayer->addChild(m_loadingCircle);
    
    return true;
}

HKGDListPopup* HKGDListPopup::create() {
    auto ret = new HKGDListPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void HKGDListPopup::createUI() {
    HKGDManager::get()->fetchAllLevels([this](std::vector<HKGDLevelInfo> levels) {
        this->onLoadingFinished(levels);
    });
}

void HKGDListPopup::onLoadingFinished(std::vector<HKGDLevelInfo> levels) {
    m_levels = levels;
    
    if (m_loadingCircle) {
        m_loadingCircle->setVisible(false);
        m_loadingCircle->removeFromParent();
        m_loadingCircle = nullptr;
    }
    
    if (m_levels.empty()) {
        auto noLevels = CCLabelBMFont::create("Failed to load levels.\nCheck your internet connection.", "bigFont.fnt");
        noLevels->setPosition({m_size.width / 2, m_size.height / 2});
        noLevels->setScale(0.5f);
        noLevels->setAlignment(kCCTextAlignmentCenter);
        m_mainLayer->addChild(noLevels);
        return;
    }
    
    createLevelsList();
}

void HKGDListPopup::createLevelsList() {
    float listWidth = 380.f;
    float listHeight = 250.f;
    float cellHeight = 30.f;
    
    // Create scroll layer
    auto scrollLayer = ScrollLayer::create({listWidth, listHeight});
    scrollLayer->setPosition({(m_size.width - listWidth) / 2, 20.f});
    m_mainLayer->addChild(scrollLayer);
    m_scrollView = scrollLayer;
    
    // Content layer
    auto contentLayer = scrollLayer->m_contentLayer;
    
    // Add level cells
    for (size_t i = 0; i < m_levels.size(); i++) {
        auto cell = createLevelCell(m_levels[i], static_cast<int>(i));
        cell->setPosition({listWidth / 2, static_cast<float>(m_levels.size() - i - 0.5f) * cellHeight});
        contentLayer->addChild(cell);
    }
    
    // Set content layer size
    contentLayer->setContentSize({listWidth, m_levels.size() * cellHeight});
    scrollLayer->scrollToTop();
}

CCNode* HKGDListPopup::createLevelCell(HKGDLevelInfo const& level, int index) {
    float cellHeight = 30.f;
    
    auto cell = CCNode::create();
    cell->setContentSize({380.f, cellHeight});
    
    // Background
    ccColor4B bgColor = index % 2 == 0 ? ccc4(60, 40, 40, 100) : ccc4(40, 30, 30, 100);
    auto bg = CCLayerColor::create(bgColor, 380.f, cellHeight);
    bg->setPosition({-190.f, -cellHeight / 2});
    cell->addChild(bg);
    
    // Rank
    auto rankLabel = CCLabelBMFont::create(fmt::format("#{}", level.position).c_str(), "goldFont.fnt");
    rankLabel->setAnchorPoint({0, 0.5f});
    rankLabel->setPosition({-180.f, 0});
    rankLabel->setScale(0.35f);
    cell->addChild(rankLabel);
    
    // Level name (truncate if too long)
    std::string name = level.name;
    if (name.length() > 25) {
        name = name.substr(0, 22) + "...";
    }
    auto nameLabel = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
    nameLabel->setAnchorPoint({0, 0.5f});
    nameLabel->setPosition({-120.f, 0});
    nameLabel->setScale(0.35f);
    cell->addChild(nameLabel);
    
    // Level ID
    auto idLabel = CCLabelBMFont::create(fmt::format("ID: {}", level.levelId).c_str(), "bigFont.fnt");
    idLabel->setAnchorPoint({1, 0.5f});
    idLabel->setPosition({180.f, 0});
    idLabel->setScale(0.3f);
    idLabel->setColor({150, 150, 150});
    cell->addChild(idLabel);
    
    // Make clickable
    auto touchSpr = CCSprite::create();
    touchSpr->setTextureRect({0, 0, 380, cellHeight});
    touchSpr->setPosition({0, 0});
    touchSpr->setOpacity(0);
    
    auto btn = CCMenuItemSpriteExtra::create(
        touchSpr,
        this,
        menu_selector(HKGDListPopup::onLevelClicked)
    );
    btn->setTag(level.levelId);
    
    auto menu = CCMenu::create();
    menu->addChild(btn);
    menu->setPosition({0, 0});
    cell->addChild(menu);
    
    return cell;
}

void HKGDListPopup::onLevelClicked(CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    int levelId = btn->getTag();
    
    // Open the HKGDL website
    auto url = HKGDManager::get()->getWebsiteUrl();
    CCApplication::sharedApplication()->openURL(url.c_str());
}

// HKGDPlayerStatsPopup implementation

bool HKGDPlayerStatsPopup::init(const std::string& username, int accountId) {
    m_username = username;
    m_accountId = accountId;
    
    if (!geode::Popup::init(420.f, 320.f)) {
        return false;
    }
    
    setTitle(fmt::format("HKGD Stats - {}", m_username).c_str());
    
    // Loading circle
    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setPosition({m_size.width / 2, m_size.height / 2});
    m_loadingCircle->setVisible(true);
    m_mainLayer->addChild(m_loadingCircle);
    
    return true;
}

HKGDPlayerStatsPopup* HKGDPlayerStatsPopup::create(const std::string& username, int accountId) {
    auto ret = new HKGDPlayerStatsPopup();
    if (ret->init(username, accountId)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void HKGDPlayerStatsPopup::createUI() {
    HKGDManager::get()->fetchPlayerRecordsWithMapping(m_username, [this](std::vector<HKGDRecord> records) {
        this->onLoadingFinished(records);
    });
}

void HKGDPlayerStatsPopup::onLoadingFinished(std::vector<HKGDRecord> records) {
    m_records = records;
    
    if (m_loadingCircle) {
        m_loadingCircle->setVisible(false);
        m_loadingCircle->removeFromParent();
        m_loadingCircle = nullptr;
    }
    
    if (m_records.empty()) {
        auto noRecords = CCLabelBMFont::create(
            "No extreme demons beaten yet,\nor player not found on HKGDL.",
            "bigFont.fnt"
        );
        noRecords->setPosition({m_size.width / 2, m_size.height / 2});
        noRecords->setScale(0.45f);
        noRecords->setAlignment(kCCTextAlignmentCenter);
        m_mainLayer->addChild(noRecords);
        
        // Add info about name mapping
        auto mappingInfo = CCLabelBMFont::create(
            "Note: If this isn't your account,\nuse the admin panel to map names.",
            "chatFont.fnt"
        );
        mappingInfo->setPosition({m_size.width / 2, m_size.height / 2 - 40.f});
        mappingInfo->setScale(0.4f);
        mappingInfo->setAlignment(kCCTextAlignmentCenter);
        mappingInfo->setColor({150, 150, 150});
        m_mainLayer->addChild(mappingInfo);
        return;
    }
    
    createStatsList();
}

void HKGDPlayerStatsPopup::createStatsList() {
    float listWidth = 380.f;
    float listHeight = 250.f;
    float cellHeight = 30.f;
    
    // Create scroll layer
    auto scrollLayer = ScrollLayer::create({listWidth, listHeight});
    scrollLayer->setPosition({(m_size.width - listWidth) / 2, 20.f});
    m_mainLayer->addChild(scrollLayer);
    m_scrollView = scrollLayer;
    
    // Content layer
    auto contentLayer = scrollLayer->m_contentLayer;
    
    // Header row
    auto header = CCLabelBMFont::create("Rank   Level Name                            Date", "bigFont.fnt");
    header->setAnchorPoint({0, 0.5f});
    header->setPosition({10.f, m_records.size() * cellHeight - cellHeight / 2});
    header->setScale(0.28f);
    header->setColor({200, 200, 200});
    contentLayer->addChild(header);
    
    // Add record cells
    for (size_t i = 0; i < m_records.size(); i++) {
        auto cell = createRecordCell(m_records[i], static_cast<int>(i));
        cell->setPosition({listWidth / 2, static_cast<float>(m_records.size() - i - 1.5f) * cellHeight});
        contentLayer->addChild(cell);
    }
    
    // Set content layer size
    contentLayer->setContentSize({listWidth, m_records.size() * cellHeight});
    scrollLayer->scrollToTop();
}

CCNode* HKGDPlayerStatsPopup::createRecordCell(HKGDRecord const& record, int index) {
    float cellHeight = 30.f;
    
    auto cell = CCNode::create();
    cell->setContentSize({380.f, cellHeight});
    
    // Background
    ccColor4B bgColor = index % 2 == 0 ? ccc4(50, 50, 60, 100) : ccc4(30, 30, 40, 100);
    auto bg = CCLayerColor::create(bgColor, 380.f, cellHeight);
    bg->setPosition({-190.f, -cellHeight / 2});
    cell->addChild(bg);
    
    // HKGD Rank
    auto rankLabel = CCLabelBMFont::create(fmt::format("#{}", record.rank).c_str(), "goldFont.fnt");
    rankLabel->setAnchorPoint({0, 0.5f});
    rankLabel->setPosition({-180.f, 0});
    rankLabel->setScale(0.35f);
    cell->addChild(rankLabel);
    
    // Level name
    std::string levelName = record.levelName;
    if (levelName.empty()) {
        levelName = "Unknown";
    }
    if (levelName.length() > 20) {
        levelName = levelName.substr(0, 17) + "...";
    }
    auto nameLabel = CCLabelBMFont::create(levelName.c_str(), "bigFont.fnt");
    nameLabel->setAnchorPoint({0, 0.5f});
    nameLabel->setPosition({-140.f, 0});
    nameLabel->setScale(0.3f);
    cell->addChild(nameLabel);
    
    // Date
    auto dateLabel = CCLabelBMFont::create(record.date.c_str(), "bigFont.fnt");
    dateLabel->setAnchorPoint({1, 0.5f});
    dateLabel->setPosition({130.f, 0});
    dateLabel->setScale(0.28f);
    dateLabel->setColor({150, 150, 150});
    cell->addChild(dateLabel);

    // Video button (if video URL exists)
    if (!record.videoUrl.empty()) {
        auto videoBtnSprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
        if (videoBtnSprite) {
            videoBtnSprite->setScale(0.3f);
            auto videoBtn = CCMenuItemSpriteExtra::create(
                videoBtnSprite,
                this,
                menu_selector(HKGDPlayerStatsPopup::onVideoButton)
            );
            videoBtn->setPosition({170.f, 0});
            videoBtn->setUserObject(CCString::create(record.videoUrl));

            auto menu = CCMenu::create();
            menu->addChild(videoBtn);
            menu->setPosition({0, 0});
            cell->addChild(menu);
        }
    }
    
    // Attempts (if available)
    if (record.attempts > 0) {
        auto attLabel = CCLabelBMFont::create(fmt::format("{} att", record.attempts).c_str(), "bigFont.fnt");
        attLabel->setAnchorPoint({0, 0.5f});
        attLabel->setPosition({80.f, 0});
        attLabel->setScale(0.25f);
        attLabel->setColor({120, 120, 120});
        cell->addChild(attLabel);
    }
    
    return cell;
}

void HKGDPlayerStatsPopup::onVideoButton(CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto urlObj = static_cast<CCString*>(btn->getUserObject());
    if (urlObj) {
        std::string url = urlObj->getCString();
        if (!url.empty()) {
            // Ensure URL has protocol
            if (url.find("http://") != 0 && url.find("https://") != 0) {
                url = "https://" + url;
            }
            CCApplication::sharedApplication()->openURL(url.c_str());
        }
    }
}
