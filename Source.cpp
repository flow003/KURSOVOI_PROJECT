#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include "Constants.h"
#include "GameStates.h"

int main() {
    srand((unsigned)time(nullptr));

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u{ (unsigned)W, (unsigned)H }), "SpectralDust");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("fonts/arial.ttf")) return -1;

    sf::Music menuMusic;
    bool musicLoaded = menuMusic.openFromFile("sounds/main_sound.ogg");
    if (musicLoaded) {
        menuMusic.setLooping(true);
        menuMusic.setVolume(50.f);
        menuMusic.play();
    }

    LoadingScreen loadingScreen(font);
    sf::Clock loadClock;
    bool loadingDone = false;

    while (window.isOpen() && !loadingDone) {
        float dt = loadClock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;

        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        loadingScreen.update(dt);
        loadingDone = loadingScreen.isDone();

        window.clear();
        loadingScreen.draw(window);
        window.display();
    }

    float globalVolume = 100.f;
    int globalDifficulty = 1;
    bool globalShowHitboxes = true;
    int globalCoins = 0;
    GlobalStats globalStats;

    MenuState menuState(font, globalVolume, globalDifficulty, globalShowHitboxes, globalCoins, globalStats);
    PlayState* playState = nullptr;
    ShopState shopState(font);

    enum class AppState { MainMenu, Settings, About, Shop, Help, Playing, GameShop };
    AppState appState = AppState::MainMenu;
    State menuScreen = State::Menu;
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;

        if (musicLoaded) {
            menuMusic.setVolume(globalVolume);
        }

        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (appState == AppState::MainMenu || appState == AppState::Settings ||
                appState == AppState::About || appState == AppState::Shop || appState == AppState::Help) {
                bool startGame = false;
                menuState.handleInput(*event, startGame, menuScreen);
                if (menuScreen == State::Menu) appState = AppState::MainMenu;
                else if (menuScreen == State::Settings) appState = AppState::Settings;
                else if (menuScreen == State::About) appState = AppState::About;
                else if (menuScreen == State::Shop) appState = AppState::Shop;
                else if (menuScreen == State::Help) {
                    ShellExecute(NULL, L"open", L"spravka.chm", NULL, NULL, SW_SHOW);
                    menuScreen = State::Menu;
                    appState = AppState::MainMenu;
                }
                if (startGame) {
                    if (playState) delete playState;
                    if (musicLoaded) menuMusic.stop();
                    playState = new PlayState(font, globalVolume, globalDifficulty, globalShowHitboxes, globalCoins, globalStats);
                    appState = AppState::Playing;
                }
            }
            else if (appState == AppState::Playing && playState) {
                bool returnToMenu = false;
                bool openShop = false;
                playState->handleInput(*event, returnToMenu, openShop);
                if (returnToMenu) {
                    delete playState;
                    playState = nullptr;
                    appState = AppState::MainMenu;
                    menuScreen = State::Menu;
                    if (musicLoaded) menuMusic.play();
                }
                if (openShop) appState = AppState::GameShop;
            }
            else if (appState == AppState::GameShop && playState) {
                bool shopDone = false;
                shopState.handleInput(*event, playState->getPlayer(), globalCoins, shopDone);
                if (shopDone) {
                    playState->continueAfterShop();
                    appState = AppState::Playing;
                }
            }
        }

        if (appState == AppState::Playing && playState) {
            bool returnToMenu = false;
            bool openShop = false;
            playState->update(dt, returnToMenu, openShop);
            if (returnToMenu) {
                delete playState;
                playState = nullptr;
                appState = AppState::MainMenu;
                menuScreen = State::Menu;
                if (musicLoaded) menuMusic.play();
            }
            if (openShop) appState = AppState::GameShop;
        }

        window.clear(sf::Color(10, 5, 30));
        if (appState == AppState::MainMenu || appState == AppState::Settings ||
            appState == AppState::About || appState == AppState::Shop || appState == AppState::Help)
            menuState.draw(window, menuScreen);
        else if (appState == AppState::Playing && playState)
            playState->draw(window);
        else if (appState == AppState::GameShop && playState)
            shopState.draw(window, playState->getPlayer(), globalCoins);
        window.display();
    }

    if (playState) delete playState;
    return 0;
}