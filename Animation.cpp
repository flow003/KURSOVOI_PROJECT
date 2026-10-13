#include "Animation.h"
#include "Constants.h"
#include <cstdlib>
#include <algorithm>

void Anim::update(float dt) {
    if (done) return;
    timer += dt;
    if (timer >= 1.f / fps) {
        timer = 0.f;
        cur++;
        if (cur >= total) {
            if (loop) cur = 0;
            else { cur = total - 1; done = true; }
        }
    }
}

void Anim::reset() { cur = 0; timer = 0.f; done = false; }

sf::IntRect Anim::rect() const {
    int col = cur % cols;
    int row = cur / cols;
    return sf::IntRect(sf::Vector2i{ col * frameW, row * frameH }, sf::Vector2i{ frameW, frameH });
}

std::vector<Tree> genTrees(int seed) {
    srand(seed);
    std::vector<Tree> v;
    for (int i = 0; i < 18; ++i) {
        Tree t;
        t.x = (float)(rand() % (W - 40));
        t.y = GROUND - 20.f - rand() % 80;
        t.scale = 0.7f + (rand() % 60) / 100.f;
        int g = 30 + rand() % 80;
        t.col = sf::Color((uint8_t)(10 + rand() % 40), (uint8_t)g, (uint8_t)(10 + rand() % 30), 200);
        v.push_back(t);
    }
    return v;
}

void drawBox(sf::RenderWindow& w, float x, float y, float bw, float bh,
    sf::Color fill, sf::Color outline) {
    sf::RectangleShape r(sf::Vector2f{ bw, bh });
    r.setPosition(sf::Vector2f{ x, y });
    r.setFillColor(fill);
    r.setOutlineThickness(2.f);
    r.setOutlineColor(outline);
    w.draw(r);
}

void drawText(sf::RenderWindow& w, const sf::Font& f, const std::wstring& s,
    float x, float y, unsigned sz, sf::Color c, bool center) {
    sf::Text t(f);
    t.setString(s);
    t.setCharacterSize(sz);
    t.setFillColor(c);
    if (center) {
        sf::FloatRect bounds = t.getLocalBounds();
        t.setOrigin(sf::Vector2f{ bounds.size.x / 2.f, 0.f });
    }
    t.setPosition(sf::Vector2f{ x, y });
    w.draw(t);
}