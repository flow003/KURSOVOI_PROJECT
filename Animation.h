#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct Anim {
    sf::Texture* tex = nullptr;
    int frameW = 64, frameH = 64;
    int cols = 2, total = 2;
    float fps = 6.f, timer = 0.f;
    int cur = 0;
    bool loop = true, done = false;

    void update(float dt);
    void reset();
    sf::IntRect rect() const;
};

struct Tree {
    float x = 0, y = 0, scale = 1;
    sf::Color col;
};

std::vector<Tree> genTrees(int seed);
void drawBox(sf::RenderWindow& w, float x, float y, float bw, float bh,
    sf::Color fill, sf::Color outline = sf::Color::White);
void drawText(sf::RenderWindow& w, const sf::Font& f, const std::wstring& s,
    float x, float y, unsigned sz = 22,
    sf::Color c = sf::Color::White, bool center = false);