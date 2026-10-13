#include "GameStates.h"
#include "Constants.h"
#include "Animation.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <string>
using namespace std;

// ==================== LoadingScreen ====================
LoadingScreen::LoadingScreen(sf::Font& f) : font(f) {}

void LoadingScreen::update(float dt) {
    if (done) return;
    progress += dt * 30.f;
    if (progress >= 100.f) { progress = 100.f; done = true; }
}

void LoadingScreen::draw(sf::RenderWindow& w) {
    sf::RectangleShape bg(sf::Vector2f{ (float)W, (float)H });
    bg.setFillColor(sf::Color::Black);
    w.draw(bg);
    sf::Text t(font);
    t.setString(L"ЗАГРУЗКА");
    t.setCharacterSize(48);
    t.setFillColor(sf::Color::White);
    sf::FloatRect tb = t.getLocalBounds();
    t.setOrigin(sf::Vector2f{ tb.size.x / 2.f, tb.size.y / 2.f });
    t.setPosition(sf::Vector2f{ W / 2.f, H / 2.f - 60.f });
    w.draw(t);
    sf::RectangleShape barBg(sf::Vector2f{ 400.f, 30.f });
    barBg.setPosition(sf::Vector2f{ W / 2.f - 200.f, H / 2.f });
    barBg.setFillColor(sf::Color(60, 60, 60));
    barBg.setOutlineThickness(2.f);
    barBg.setOutlineColor(sf::Color(150, 150, 150));
    w.draw(barBg);
    sf::RectangleShape barFill(sf::Vector2f{ 400.f * (progress / 100.f), 30.f });
    barFill.setPosition(sf::Vector2f{ W / 2.f - 200.f, H / 2.f });
    barFill.setFillColor(sf::Color(255, 200, 50));
    w.draw(barFill);
    sf::Text pt(font);
    pt.setString(to_wstring((int)progress) + L"%");
    pt.setCharacterSize(20);
    pt.setFillColor(sf::Color::White);
    sf::FloatRect pb = pt.getLocalBounds();
    pt.setOrigin(sf::Vector2f{ pb.size.x / 2.f, 0.f });
    pt.setPosition(sf::Vector2f{ W / 2.f, H / 2.f + 40.f });
    w.draw(pt);
}

// ==================== Level ====================
void Level::spawnWave(int d) {
    enemies.clear();
    int cnt = 2 + wave + id;
    for (int i = 0; i < cnt; ++i) {
        float x = (rand() % 2 == 0) ? (float)(W - 60 - rand() % 200) : (float)(rand() % 200 + W / 2);
        int type = (wave >= 1 && (rand() % 100) < 30) ? 1 : 0;
        enemies.emplace_back(x, GROUND - 64.f, false, type);
        int bh = 0;
        if (wave == 0) bh = 1;
        else if (wave == 1) bh = 3;
        else if (wave == 2) bh = 5;
        else bh = 7;
        enemies.back().hp = ENEMY_BASE_HP + id * 3 + d * 3 + bh;
        if (type == 1) enemies.back().hp -= 2;
        enemies.back().maxHp = enemies.back().hp;
        enemies.back().damage = 1 + d / 2;
    }
}

void Level::spawnBoss(int d) {
    enemies.clear();
    enemies.emplace_back(800.f, GROUND - 110.f, true);
    enemies[0].hp = 20 + id * 10 + d * 10;
    enemies[0].maxHp = enemies[0].hp;
    enemies[0].damage = 2 + d;
}

// ==================== MenuState ====================
MenuState::MenuState(sf::Font& f, float& vol, int& diff, bool& hb, int& coins, GlobalStats& st)
    : font(f), volume(vol), difficulty(diff), showHitboxes(hb), totalCoins(coins), stats(st) {
}

void MenuState::handleInput(sf::Event& e, bool& start, State& s) {
    if (auto* kp = e.getIf<sf::Event::KeyPressed>()) {
        if (s == State::Menu) {
            if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W)
                selectedItem = (selectedItem - 1 + menuItems) % menuItems;
            if (kp->code == sf::Keyboard::Key::Down || kp->code == sf::Keyboard::Key::S)
                selectedItem = (selectedItem + 1) % menuItems;
            if (kp->code == sf::Keyboard::Key::Enter) {
                if (selectedItem == 0) start = true;
                else if (selectedItem == 1) s = State::Shop;
                else if (selectedItem == 2) s = State::Settings;
                else if (selectedItem == 3) s = State::About;
                else if (selectedItem == 4) s = State::Help;
                else if (selectedItem == 5) exit(0);
            }
        }
        else if (s == State::Shop) {
            if (kp->code == sf::Keyboard::Key::Escape) { s = State::Menu; return; }
            if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W)
                shopSelectedItem = (shopSelectedItem - 1 + shopItems) % shopItems;
            if (kp->code == sf::Keyboard::Key::Down || kp->code == sf::Keyboard::Key::S)
                shopSelectedItem = (shopSelectedItem + 1) % shopItems;
            if (kp->code == sf::Keyboard::Key::Enter) {
                int costs[] = { 200, 200, 150 };
                if (totalCoins >= costs[shopSelectedItem]) {
                    totalCoins -= costs[shopSelectedItem];
                    if (shopSelectedItem == 0) stats.bonusMaxHp += 1;
                    else if (shopSelectedItem == 1) stats.bonusDamage += 1;
                }
            }
        }
        else if (s == State::Settings) {
            if (kp->code == sf::Keyboard::Key::Escape) { s = State::Menu; return; }
            if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W)
                selectedSetting = (selectedSetting - 1 + settingsCount) % settingsCount;
            if (kp->code == sf::Keyboard::Key::Down || kp->code == sf::Keyboard::Key::S)
                selectedSetting = (selectedSetting + 1) % settingsCount;
            if (selectedSetting == 0) {
                if (kp->code == sf::Keyboard::Key::Left || kp->code == sf::Keyboard::Key::A) volume = max(0.f, volume - 10.f);
                if (kp->code == sf::Keyboard::Key::Right || kp->code == sf::Keyboard::Key::D) volume = min(100.f, volume + 10.f);
            }
            else if (selectedSetting == 1) {
                if (kp->code == sf::Keyboard::Key::Left || kp->code == sf::Keyboard::Key::A) {
                    if (difficulty > 0) { difficulty--; totalCoins = 0; stats.bonusMaxHp = 0; stats.bonusDamage = 0; }
                }
                if (kp->code == sf::Keyboard::Key::Right || kp->code == sf::Keyboard::Key::D) {
                    if (difficulty < 3) { difficulty++; totalCoins = 0; stats.bonusMaxHp = 0; stats.bonusDamage = 0; }
                }
            }
            else if (selectedSetting == 2) {
                if (kp->code == sf::Keyboard::Key::Left || kp->code == sf::Keyboard::Key::Right || kp->code == sf::Keyboard::Key::Enter)
                    showHitboxes = !showHitboxes;
            }
        }
        else if (s == State::About || s == State::Help) {
            if (kp->code == sf::Keyboard::Key::Escape) s = State::Menu;
        }
    }
}

void MenuState::draw(sf::RenderWindow& w, State s) {
    static sf::Texture* bgTexPtr = nullptr;
    static sf::Sprite* bgSpritePtr = nullptr;
    if (!bgTexPtr) {
        bgTexPtr = new sf::Texture();
        if (bgTexPtr->loadFromFile("images/background_menu.png")) {
            bgSpritePtr = new sf::Sprite(*bgTexPtr);
            float sx = (float)W / bgTexPtr->getSize().x;
            float sy = (float)H / bgTexPtr->getSize().y;
            bgSpritePtr->setScale(sf::Vector2f{ sx, sy });
        }
    }
    if (bgSpritePtr) w.draw(*bgSpritePtr);

    if (s == State::Menu) {
        drawText(w, font, L"URSA vs PUCK", W / 2.f, 100.f, 72, sf::Color(255, 200, 50), true);
        drawText(w, font, L"Искажённый Лес", W / 2.f, 180.f, 36, sf::Color(160, 230, 130), true);
        drawText(w, font, L"Монеты: " + to_wstring(totalCoins), W / 2.f, 230.f, 24, sf::Color(255, 220, 80), true);
        const wchar_t* items[] = { L"Начать игру", L"Магазин", L"Настройки", L"Об авторе", L"Справка", L"Выход" };
        for (int i = 0; i < menuItems; i++) {
            float y = 300.f + i * 65.f;
            drawBox(w, W / 2.f - 200.f, y, 400.f, 50.f, sf::Color(60, 30, 10, 200),
                i == selectedItem ? sf::Color(255, 200, 50) : sf::Color(100, 100, 100));
            drawText(w, font, items[i], W / 2.f, y + 12.f, 24,
                i == selectedItem ? sf::Color(255, 220, 80) : sf::Color(200, 200, 200), true);
        }
        drawText(w, font, L"Стрелки - выбор   ENTER - подтвердить", W / 2.f, 700.f, 20, sf::Color(180, 180, 255), true);
    }
    else if (s == State::Shop) {
        drawText(w, font, L"МАГАЗИН", W / 2.f, 80.f, 44, sf::Color(255, 200, 50), true);
        drawText(w, font, L"Монеты: " + to_wstring(totalCoins), W / 2.f, 140.f, 28, sf::Color(255, 220, 80), true);
        drawText(w, font, L"Бонус HP: +" + to_wstring(stats.bonusMaxHp) + L"  Урон: +" + to_wstring(stats.bonusDamage),
            W / 2.f, 180.f, 18, sf::Color(200, 200, 200), true);
        const wchar_t* items[] = { L"+1 макс. HP (200)", L"+1 урона (200)", L"+2 HP (150)" };
        int costs[] = { 200, 200, 150 };
        for (int i = 0; i < shopItems; i++) {
            float y = 230.f + i * 90.f;
            drawBox(w, W / 2.f - 250.f, y, 500.f, 60.f,
                shopSelectedItem == i ? sf::Color(60, 80, 100) : sf::Color(40, 50, 70),
                shopSelectedItem == i ? sf::Color(255, 200, 50) : sf::Color(100, 100, 150));
            drawText(w, font, items[i], W / 2.f - 230.f, y + 15.f, 22,
                totalCoins >= costs[i] ? sf::Color::White : sf::Color(150, 150, 150));
        }
        drawText(w, font, L"ENTER - купить   ESC - назад", W / 2.f, 550.f, 20, sf::Color(180, 180, 180), true);
    }
    else if (s == State::Settings) {
        drawText(w, font, L"НАСТРОЙКИ", W / 2.f, 100.f, 48, sf::Color(255, 200, 50), true);
        float y1 = 200.f;
        drawBox(w, W / 2.f - 300.f, y1, 600.f, 80.f,
            selectedSetting == 0 ? sf::Color(60, 60, 80, 200) : sf::Color(40, 40, 60, 200),
            selectedSetting == 0 ? sf::Color(255, 200, 50) : sf::Color(100, 100, 150));
        drawText(w, font, L"Громкость: " + to_wstring((int)volume) + L"%", W / 2.f - 280.f, y1 + 10.f, 24, sf::Color::White);
        drawBox(w, W / 2.f - 200.f, y1 + 45.f, 400.f, 20.f, sf::Color(40, 40, 40));
        drawBox(w, W / 2.f - 200.f, y1 + 45.f, 400.f * (volume / 100.f), 20.f, sf::Color(100, 200, 100));
        float y2 = 310.f;
        drawBox(w, W / 2.f - 300.f, y2, 600.f, 80.f,
            selectedSetting == 1 ? sf::Color(60, 60, 80, 200) : sf::Color(40, 40, 60, 200),
            selectedSetting == 1 ? sf::Color(255, 200, 50) : sf::Color(100, 100, 150));
        const wchar_t* diffs[] = { L"Казуал", L"Легко", L"Нормально", L"Сложно" };
        drawText(w, font, L"Сложность: " + wstring(diffs[difficulty]), W / 2.f - 280.f, y2 + 25.f, 24, sf::Color::White);
        float y3 = 420.f;
        drawBox(w, W / 2.f - 300.f, y3, 600.f, 80.f,
            selectedSetting == 2 ? sf::Color(60, 60, 80, 200) : sf::Color(40, 40, 60, 200),
            selectedSetting == 2 ? sf::Color(255, 200, 50) : sf::Color(100, 100, 150));
        drawText(w, font, L"Хитбоксы: " + wstring(showHitboxes ? L"Вкл" : L"Выкл"), W / 2.f - 280.f, y3 + 25.f, 24, sf::Color::White);
        drawText(w, font, L"Стрелки вверх/вниз - выбор   Влево/вправо - изменить   ESC - назад", W / 2.f, 550.f, 18, sf::Color(200, 200, 200), true);
    }
    else if (s == State::About) {
        drawText(w, font, L"ОБ АВТОРЕ", W / 2.f, 200.f, 48, sf::Color(255, 200, 50), true);
        drawText(w, font, L"Сделал ILYA", W / 2.f, 350.f, 36, sf::Color::White, true);
        drawText(w, font, L"SFML 3.0 | C++ | 2026", W / 2.f, 420.f, 24, sf::Color(180, 180, 255), true);
        drawText(w, font, L"ESC - назад", W / 2.f, 520.f, 22, sf::Color(200, 200, 200), true);
    }
    else if (s == State::Help) {
        drawText(w, font, L"СПРАВКА", W / 2.f, 100.f, 48, sf::Color(255, 200, 50), true);
        drawText(w, font, L"Управление в меню:", W / 2.f, 200.f, 24, sf::Color::White, true);
        drawText(w, font, L"Стрелки - навигация   ENTER - подтвердить   ESC - назад", W / 2.f, 240.f, 20, sf::Color(200, 200, 200), true);
        drawText(w, font, L"Управление в игре:", W / 2.f, 300.f, 24, sf::Color::White, true);
        drawText(w, font, L"A/D или Стрелки - движение   W/Пробел - прыжок", W / 2.f, 340.f, 20, sf::Color(200, 200, 200), true);
        drawText(w, font, L"Z/J - атака   Q - ярость   ESC - пауза", W / 2.f, 370.f, 20, sf::Color(200, 200, 200), true);
        drawText(w, font, L"ESC - назад", W / 2.f, 520.f, 22, sf::Color(200, 200, 200), true);
    }
}

// ==================== PauseMenu ====================
PauseMenu::PauseMenu(sf::Font& f) : font(f) {}

void PauseMenu::handleInput(sf::Event& e, bool& resume, bool& retMenu) {
    if (auto* kp = e.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W)
            selectedItem = (selectedItem - 1 + menuItems) % menuItems;
        if (kp->code == sf::Keyboard::Key::Down || kp->code == sf::Keyboard::Key::S)
            selectedItem = (selectedItem + 1) % menuItems;
        if (kp->code == sf::Keyboard::Key::Enter) {
            if (selectedItem == 0) resume = true;
            else if (selectedItem == 1) retMenu = true;
        }
        if (kp->code == sf::Keyboard::Key::Escape) resume = true;
    }
}

void PauseMenu::draw(sf::RenderWindow& w) {
    drawBox(w, 0, 0, (float)W, (float)H, sf::Color(10, 5, 20, 220));
    drawText(w, font, L"ПАУЗА", W / 2.f, 250.f, 48, sf::Color(255, 200, 50), true);
    const wchar_t* items[] = { L"Продолжить", L"Выйти в главное меню" };
    for (int i = 0; i < menuItems; i++) {
        float y = 400.f + i * 80.f;
        drawBox(w, W / 2.f - 200.f, y, 400.f, 55.f,
            i == selectedItem ? sf::Color(60, 50, 20, 220) : sf::Color(40, 30, 10, 200),
            i == selectedItem ? sf::Color(255, 200, 50) : sf::Color(150, 150, 150));
        drawText(w, font, items[i], W / 2.f, y + 14.f, 24,
            i == selectedItem ? sf::Color(255, 220, 80) : sf::Color(200, 200, 200), true);
    }
}

// ==================== ShopState ====================
ShopState::ShopState(sf::Font& f) : font(f) {}

void ShopState::handleInput(sf::Event& e, Player& p, int& coins, bool& done) {
    if (auto* kp = e.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W)
            selectedItem = (selectedItem - 1 + maxItems) % maxItems;
        if (kp->code == sf::Keyboard::Key::Down || kp->code == sf::Keyboard::Key::S)
            selectedItem = (selectedItem + 1) % maxItems;
        if (kp->code == sf::Keyboard::Key::Enter) {
            int costs[] = { 200, 200, 150 };
            if (coins >= costs[selectedItem]) {
                coins -= costs[selectedItem];
                if (selectedItem == 0) { p.maxHp += 1; p.hp = min(p.hp + 1, p.maxHp); }
                else if (selectedItem == 1) p.attackDamage += 1;
                else if (selectedItem == 2) p.hp = min(p.hp + 2, p.maxHp);
            }
        }
        if (kp->code == sf::Keyboard::Key::Space) done = true;
    }
}

void ShopState::draw(sf::RenderWindow& w, Player& p, int coins) {
    drawBox(w, 0, 0, (float)W, (float)H, sf::Color(20, 15, 30, 240));
    drawText(w, font, L"МАГАЗИН (за золото)", W / 2.f, 60.f, 40, sf::Color(255, 200, 50), true);
    drawText(w, font, L"Золото: " + to_wstring(coins), W / 2.f, 110.f, 24, sf::Color(255, 220, 80), true);
    drawText(w, font, L"HP: " + to_wstring(p.hp) + L" / " + to_wstring(p.maxHp), W / 2.f, 145.f, 20, sf::Color(200, 200, 200), true);
    const wchar_t* items[] = { L"+1 макс. HP (200 золота)", L"+1 урон (200 золота)", L"+2 HP (150 золота)" };
    int costs[] = { 200, 200, 150 };
    for (int i = 0; i < maxItems; i++) {
        float y = 190.f + i * 90.f;
        drawBox(w, W / 2.f - 250.f, y, 500.f, 60.f,
            selectedItem == i ? sf::Color(60, 80, 100) : sf::Color(40, 50, 70),
            selectedItem == i ? sf::Color(255, 200, 50) : sf::Color(100, 100, 150));
        drawText(w, font, items[i], W / 2.f - 230.f, y + 15.f, 22,
            coins >= costs[i] ? sf::Color::White : sf::Color(150, 150, 150));
    }
    drawBox(w, W / 2.f - 150.f, 500.f, 300.f, 50.f, sf::Color(60, 100, 60), sf::Color(100, 255, 100));
    drawText(w, font, L"ПРОБЕЛ - Продолжить", W / 2.f, 512.f, 22, sf::Color(200, 255, 200), true);
    drawText(w, font, L"ENTER - купить   Стрелки - выбор", W / 2.f, 570.f, 18, sf::Color(180, 180, 180), true);
}

// ==================== PlayState ====================
PlayState::PlayState(sf::Font& f, float& vol, int& diff, bool& hb, int& coins, GlobalStats& st)
    : font(f), puck(800.f, GROUND - 70.f, true, 0), pauseMenu(f),
    volume(vol), difficulty(diff), showHitboxes(hb), totalCoins(coins), stats(st) {

    (void)texWalkNormal.loadFromFile("images/ursa_walk.png");
    (void)texWalkNormalRev.loadFromFile("images/ursa_walk_reverse.png");
    (void)texWalkAgressive.loadFromFile("images/ursa_walk_agressive.png");
    (void)texWalkAgressiveRev.loadFromFile("images/ursa_walk_agressive_reverse.png");
    (void)texAttackNormal.loadFromFile("images/ursa_attack.png");
    (void)texAttackNormalRev.loadFromFile("images/ursa_attack_reverse.png");
    (void)texAttackAgressive.loadFromFile("images/ursa_attack_agressive.png");
    (void)texAttackAgressiveRev.loadFromFile("images/ursa_attack_agressive_reverse.png");
    (void)texFon1.loadFromFile("images/fon_level1.png");
    (void)texFon2.loadFromFile("images/fon_level2.png");
    (void)texFon3.loadFromFile("images/fon_level3.png");

    player.loadTextures(texWalkNormal, texWalkNormalRev,
        texWalkAgressive, texWalkAgressiveRev,
        texAttackNormal, texAttackNormalRev,
        texAttackAgressive, texAttackAgressiveRev);

    if (diff == 0) { player.maxHp = 15; player.hp = 15; player.attackDamage = 100; }
    else if (diff == 1) { player.maxHp = 10; player.hp = 10; player.attackDamage = 2; }
    else if (diff == 2) { player.maxHp = 7; player.hp = 7; player.attackDamage = 1; }
    else { player.maxHp = 4; player.hp = 4; player.attackDamage = 1; }

    player.maxHp += stats.bonusMaxHp;
    player.hp = player.maxHp;
    player.attackDamage += stats.bonusDamage;

    puck.body.setFillColor(sf::Color(60, 0, 160));
    puck.body.setSize(sf::Vector2f{ 70.f, 70.f });
    puck.W_ = 70.f; puck.H_ = 70.f; puck.hp = 100; puck.maxHp = 100;

    currentLevel = new Level(1);
    currentLevel->spawnWave(difficulty);
}

PlayState::~PlayState() { delete currentLevel; }

void PlayState::continueAfterShop() { if (curLvl >= totalLevels) spawnPuck(); else nextLevel(); }

void PlayState::nextLevel() {
    curLvl++; delete currentLevel;
    currentLevel = new Level(curLvl);
    currentLevel->spawnWave(difficulty);
    state = State::Playing;
}

void PlayState::spawnPuck() {
    puck = Enemy(800.f, GROUND - 70.f, true, 0);
    puck.body.setFillColor(sf::Color(60, 0, 160));
    puck.body.setSize(sf::Vector2f{ 70.f, 70.f });
    puck.W_ = 70.f; puck.H_ = 70.f;
    puck.hp = 100 + player.score / 10;
    puck.maxHp = puck.hp;
    puckActive = true;
    state = State::BossFight;
}

void PlayState::handleWaveComplete() {
    totalCoins += player.score; player.score = 0;
    currentLevel->wave++;
    if (currentLevel->wave >= currentLevel->totalWaves) {
        currentLevel->spawnBoss(difficulty);
        state = State::BossFight;
    }
    else { currentLevel->spawnWave(difficulty); }
}

void PlayState::handleInput(sf::Event& e, bool& retMenu, bool& openShop) {
    if (auto* kp = e.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Escape && (state == State::Playing || state == State::BossFight)) {
            paused = !paused; if (paused) pauseMenu.reset(); return;
        }
        if (paused) {
            bool resume = false, ret = false;
            pauseMenu.handleInput(e, resume, ret);
            if (resume) paused = false;
            if (ret) { totalCoins += player.score; retMenu = true; }
            return;
        }
        if ((state == State::GameOver || state == State::Win) && kp->code == sf::Keyboard::Key::Enter) {
            totalCoins += player.score; retMenu = true;
        }
        if (state == State::LevelClear && kp->code == sf::Keyboard::Key::Enter) openShop = true;
        if ((state == State::Playing || state == State::BossFight) && player.onGround &&
            (kp->code == sf::Keyboard::Key::Space || kp->code == sf::Keyboard::Key::W)) player.vy = JUMPVY;
        if ((state == State::Playing || state == State::BossFight) &&
            (kp->code == sf::Keyboard::Key::Z || kp->code == sf::Keyboard::Key::J)) player.attack();
        if ((state == State::Playing || state == State::BossFight) && kp->code == sf::Keyboard::Key::Q)
            player.activateRage();
    }
}

void PlayState::update(float dt, bool& retMenu, bool& openShop) {
    if (paused) return;
    if (state != State::Playing && state != State::BossFight) return;

    player.vx = 0.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
    {
        player.vx = -PSPEED; player.facingRight = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
    {
        player.vx = PSPEED; player.facingRight = true;
    }

    player.update(dt);

    if (!puckActive) {
        for (auto& e : currentLevel->enemies) {
            e.update(dt, player.hitbox.getPosition().x + Player::HIT_W / 2.f);
            if (e.enemyType == 1 && e.alive)
                e.tryRangedAttack(player.hitbox.getPosition().x + Player::HIT_W / 2.f,
                    player.hitbox.getPosition().y + Player::HIT_H / 2.f);
            if (e.projectileActive && e.checkProjectileHit(player.getBounds())) player.takeDamage(e.damage);
            if (e.tryAttack(player.getBounds(), dt)) player.takeDamage(e.damage);
            if (player.canHit(e)) {
                particles.emit(e.body.getPosition().x + e.W_ / 2.f, e.body.getPosition().y + e.H_ / 2.f, 6);
                e.takeDamage(player.attackDamage);
                if (!e.alive) {
                    if (e.isBoss) player.score += BOSS_SCORE;
                    else if (e.enemyType == 1) player.score += MAGE_SCORE;
                    else player.score += ENEMY_SCORE;
                }
            }
        }
        if (player.hp <= 0) state = State::GameOver;
        if (currentLevel->allDead()) {
            if (state == State::BossFight) {
                totalCoins += player.score; player.score = 0;
                state = State::LevelClear;
                storyText = L"Босс повержен!\nЗолото: " + to_wstring(totalCoins);
            }
            else handleWaveComplete();
        }
    }
    else {
        puckFloat += dt;
        puck.update(dt, player.hitbox.getPosition().x + Player::HIT_W / 2.f);
        puck.body.setPosition(sf::Vector2f{ puck.body.getPosition().x, GROUND - 120.f + sin(puckFloat * 2.f) * 50.f });
        puck.tryRangedAttack(player.hitbox.getPosition().x + Player::HIT_W / 2.f,
            player.hitbox.getPosition().y + Player::HIT_H / 2.f);
        if (puck.projectileActive && puck.checkProjectileHit(player.getBounds())) player.takeDamage(puck.damage);
        if (puck.tryAttack(player.getBounds(), dt)) player.takeDamage(puck.damage);
        if (player.canHit(puck)) {
            particles.emit(puck.body.getPosition().x + 35.f, puck.body.getPosition().y + 35.f, 8);
            puck.takeDamage(player.attackDamage);
            if (!puck.alive) {
                player.score += PUCK_SCORE; totalCoins += player.score;
                stats.bonusMaxHp = 0; stats.bonusDamage = 0; state = State::Win;
            }
        }
        if (player.hp <= 0) state = State::GameOver;
    }
    particles.update(dt);
}

void PlayState::draw(sf::RenderWindow& w) {
    sf::Sprite bg(texFon1);
    if (curLvl == 1) bg.setTexture(texFon1);
    else if (curLvl == 2) bg.setTexture(texFon2);
    else bg.setTexture(texFon3);
    float sx = (float)W / bg.getTexture().getSize().x;
    float sy = (float)H / bg.getTexture().getSize().y;
    bg.setScale(sf::Vector2f{ sx, sy });
    w.draw(bg);

    sf::RectangleShape gnd(sf::Vector2f{ (float)W, (float)(H - GROUND) });
    gnd.setPosition(sf::Vector2f{ 0.f, GROUND });
    gnd.setFillColor(sf::Color(30, 55, 25, 200));
    w.draw(gnd);
    sf::RectangleShape gndLine(sf::Vector2f{ (float)W, 4.f });
    gndLine.setPosition(sf::Vector2f{ 0.f, GROUND });
    gndLine.setFillColor(sf::Color(60, 100, 40, 200));
    w.draw(gndLine);

    if (!puckActive) { for (auto& e : currentLevel->enemies) e.draw(w, font); }
    else puck.draw(w, font);

    particles.draw(w); player.draw(w);
    if (!showHitboxes) player.hitbox.setOutlineThickness(0.f);

    drawText(w, font, L"HP:", 10.f, 10.f, 20);
    for (int i = 0; i < player.maxHp; ++i) {
        sf::RectangleShape bar(sf::Vector2f{ 22.f, 18.f });
        bar.setPosition(sf::Vector2f{ 60.f + i * 26.f, 12.f });
        bar.setFillColor(i < player.hp ? sf::Color(220, 60, 60) : sf::Color(80, 30, 30));
        bar.setOutlineThickness(1.f); bar.setOutlineColor(sf::Color(200, 200, 200, 80));
        w.draw(bar);
    }

    drawText(w, font, L"Очки: " + to_wstring(player.score), 10.f, 40.f, 20);
    drawText(w, font, L"Золото: " + to_wstring(totalCoins), 10.f, 65.f, 18, sf::Color(255, 220, 80));
    drawText(w, font, L"Уровень: " + to_wstring(curLvl) + L" / " + to_wstring(totalLevels), W - 200.f, 10.f, 20);
    if (!puckActive && state != State::BossFight)
        drawText(w, font, L"Волна: " + to_wstring(currentLevel->wave + 1) + L" / " + to_wstring(currentLevel->totalWaves), W - 200.f, 40.f, 18);

    const wchar_t* diffs[] = { L"Казуал", L"Легко", L"Нормально", L"Сложно" };
    drawText(w, font, L"Сложность: " + wstring(diffs[difficulty]), W - 200.f, 65.f, 16, sf::Color(200, 200, 200));

    if (player.attackCooldown > 0.f) {
        drawBox(w, 10.f, 90.f, 100.f, 8.f, sf::Color(40, 40, 40));
        drawBox(w, 10.f, 90.f, 100.f * (1.f - min(1.f, player.attackCooldown / ATTACK_COOLDOWN)), 8.f, sf::Color(255, 180, 0));
    }

    drawText(w, font, L"Z/J - удар   Q - ярость   Пробел - прыжок   ESC - пауза", W - 500.f, H - 20.f, 13, sf::Color(160, 160, 160));

    float rx = 10.f, ry = H - 90.f;
    if (player.rageCooldown > 0.f) {
        drawBox(w, rx, ry, 55.f, 55.f, sf::Color(60, 60, 60), sf::Color(120, 120, 120));
        drawText(w, font, L"Q", rx + 18.f, ry + 8.f, 22, sf::Color(150, 150, 150));
        drawText(w, font, to_wstring((int)ceil(player.rageCooldown)) + L"с", rx + 8.f, ry + 35.f, 14, sf::Color(255, 200, 100));
    }
    else if (player.rageActive) {
        drawBox(w, rx, ry, 55.f, 55.f, sf::Color(255, 100, 30), sf::Color(255, 180, 80));
        drawText(w, font, L"Q", rx + 18.f, ry + 8.f, 22, sf::Color::White);
        drawText(w, font, to_wstring((int)ceil(player.rageTimer)) + L"с", rx + 10.f, ry + 35.f, 14, sf::Color::White);
    }
    else {
        drawBox(w, rx, ry, 55.f, 55.f, sf::Color(255, 180, 40), sf::Color(255, 255, 120));
        drawText(w, font, L"Q", rx + 18.f, ry + 8.f, 22, sf::Color::White);
        drawText(w, font, L"Ярость", rx + 3.f, ry + 35.f, 12, sf::Color::White);
    }

    if (puckActive && puck.alive) {
        drawText(w, font, L"ПАК - Финальный босс", W / 2.f, 18.f, 22, sf::Color(200, 100, 255), true);
        drawBox(w, W / 2.f - 200.f, 46.f, 400.f, 14.f, sf::Color(60, 0, 80));
        drawBox(w, W / 2.f - 200.f, 46.f, 400.f * ((float)puck.hp / (float)puck.maxHp), 14.f, sf::Color(160, 40, 220));
    }
    else if (state == State::BossFight && !currentLevel->enemies.empty() && currentLevel->enemies[0].alive) {
        drawText(w, font, L"БОСС УРОВНЯ", W / 2.f, 18.f, 22, sf::Color(200, 70, 70), true);
        drawBox(w, W / 2.f - 200.f, 46.f, 400.f, 14.f, sf::Color(60, 0, 0));
        drawBox(w, W / 2.f - 200.f, 46.f, 400.f * ((float)currentLevel->enemies[0].hp / (float)currentLevel->enemies[0].maxHp), 14.f, sf::Color(200, 30, 30));
    }

    if (state == State::LevelClear) {
        drawBox(w, W / 2.f - 280.f, 160.f, 560.f, 240.f, sf::Color(20, 30, 10, 220), sf::Color(120, 200, 80));
        drawText(w, font, L"УРОВЕНЬ ПРОЙДЕН!", W / 2.f, 180.f, 30, sf::Color(120, 255, 80), true);
        drawText(w, font, storyText, W / 2.f, 240.f, 24, sf::Color(220, 220, 180), true);
        drawText(w, font, L"ENTER - Магазин (за золото)", W / 2.f, 340.f, 22, sf::Color(180, 255, 150), true);
    }
    else if (state == State::Win) {
        drawBox(w, W / 2.f - 310.f, 100.f, 620.f, 350.f, sf::Color(20, 10, 40, 220), sf::Color(200, 100, 255));
        drawText(w, font, L"ПОБЕДА!", W / 2.f, 130.f, 44, sf::Color(255, 220, 50), true);
        drawText(w, font, L"Урса повергает Пака!\nЛес спасён!", W / 2.f, 210.f, 24, sf::Color(210, 200, 255), true);
        drawText(w, font, L"Итого золота: " + to_wstring(totalCoins), W / 2.f, 300.f, 28, sf::Color(255, 220, 80), true);
        drawText(w, font, L"ENTER - В меню", W / 2.f, 370.f, 22, sf::Color(180, 180, 255), true);
    }
    else if (state == State::GameOver) {
        drawBox(w, W / 2.f - 260.f, 160.f, 520.f, 240.f, sf::Color(40, 5, 5, 220), sf::Color(200, 40, 40));
        drawText(w, font, L"ПОРАЖЕНИЕ", W / 2.f, 188.f, 38, sf::Color(200, 40, 40), true);
        drawText(w, font, L"Пак победил...", W / 2.f, 258.f, 22, sf::Color(200, 180, 180), true);
        drawText(w, font, L"Золото: " + to_wstring(totalCoins), W / 2.f, 306.f, 24, sf::Color(255, 160, 50), true);
        drawText(w, font, L"ENTER - В меню", W / 2.f, 356.f, 20, sf::Color(255, 180, 180), true);
    }
    if (paused) pauseMenu.draw(w);
}