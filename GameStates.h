#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Entities.h"
#include "Animation.h

struct GlobalStats { int bonusMaxHp = 0; int bonusDamage = 0; };

struct Level {
    int id, wave = 0, totalWaves = 2;
    std::vector<Enemy> enemies;
    Level(int i) : id(i) {}
    void spawnWave(int difficulty = 1);
    void spawnBoss(int difficulty = 1);
    bool allDead() const { for (const auto& e : enemies) if (e.alive) return false; return true; }
};

enum class State { Menu, Settings, About, Playing, LevelClear, BossFight, Win, GameOver, Shop, Paused, Help };

class LoadingScreen {
    sf::Font& font; float progress = 0.f; bool done = false;
public: LoadingScreen(sf::Font& f); void update(float dt); void draw(sf::RenderWindow& w); bool isDone() const { return done; }
};

class MenuState {
    sf::Font& font; int selectedItem = 0; static const int menuItems = 6;
    float& volume; int& difficulty; bool& showHitboxes; int& totalCoins; GlobalStats& stats;
    int selectedSetting = 0; static const int settingsCount = 3;
    int shopSelectedItem = 0; static const int shopItems = 3;
public:
    MenuState(sf::Font& f, float& vol, int& diff, bool& hb, int& coins, GlobalStats& st);
    void handleInput(sf::Event& e, bool& start, State& st);
    void draw(sf::RenderWindow& w, State st);
    int getTotalCoins() const { return totalCoins; }
};

class ShopState {
    sf::Font& font; int selectedItem = 0; static const int maxItems = 3;
public: ShopState(sf::Font& f); void handleInput(sf::Event& e, Player& p, int& coins, bool& done); void draw(sf::RenderWindow& w, Player& p, int coins);
};

class PauseMenu {
    sf::Font& font; int selectedItem = 0; static const int menuItems = 2;
public: PauseMenu(sf::Font& f); void handleInput(sf::Event& e, bool& resume, bool& ret); void draw(sf::RenderWindow& w); void reset() { selectedItem = 0; }
};

class PlayState {
    sf::Font& font;
    sf::Texture texWalkNormal, texWalkNormalRev, texWalkAgressive, texWalkAgressiveRev;
    sf::Texture texAttackNormal, texAttackNormalRev, texAttackAgressive, texAttackAgressiveRev;
    sf::Texture texFon1, texFon2, texFon3;
    Player player; Level* currentLevel = nullptr; ParticleSystem particles;
    Enemy puck; bool puckActive = false; float puckFloat = 0.f;
    State state = State::Playing; std::wstring storyText; int curLvl = 1; static const int totalLevels = 3;
    bool paused = false; PauseMenu pauseMenu;
    float& volume; int& difficulty; bool& showHitboxes; int& totalCoins; GlobalStats& stats;
    void nextLevel(); void spawnPuck(); void handleWaveComplete();
public:
    PlayState(sf::Font& f, float& vol, int& diff, bool& hb, int& coins, GlobalStats& st);
    ~PlayState();
    void handleInput(sf::Event& e, bool& retMenu, bool& openShop);
    void update(float dt, bool& retMenu, bool& openShop);
    void draw(sf::RenderWindow& w);
    Player& getPlayer() { return player; }
    State getState() const { return state; }
    void continueAfterShop();
};
