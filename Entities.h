#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <optional>
#include "Animation.h"
#include "Constants.h"

class Enemy {
public:
    sf::RectangleShape body;
    sf::RectangleShape eyeShape;
    sf::CircleShape pupilShape;
    sf::RectangleShape hpBarBg;
    sf::RectangleShape hpBarFill;
    int maxHp = 5;

    float vx = 0, vy = 0;
    int hp = 5;
    bool alive = true;
    bool isBoss = false;
    bool onGround = false;
    float attackTimer = 0.f;
    float hurtTimer = 0.f;
    int damage = 1;
    float W_ = 64.f, H_ = 64.f;

    float retreatTimer = 0.f;
    bool retreating = false;
    float retreatDirection = 0.f;

    int enemyType = 0;
    float rangedAttackTimer = 0.f;
    sf::CircleShape projectile;
    bool projectileActive = false;
    float projectileVx = 0.f;
    float projectileVy = 0.f;
    float projectileHitTimer = 0.f;

    Enemy(float x, float y, bool boss = false, int type = 0);
    sf::FloatRect getBounds() const;
    void draw(sf::RenderWindow& win, const sf::Font& font);
    void update(float dt, float playerX);
    bool tryAttack(sf::FloatRect playerBounds, float dt);
    void tryRangedAttack(float playerX, float playerY);
    void updateProjectile(float dt);
    void drawProjectile(sf::RenderWindow& win);
    bool checkProjectileHit(sf::FloatRect playerBounds);
    void takeDamage(int dmg);
};

class Player {
public:
    static const int SPR_W = 96;
    static const int SPR_H = 96;
    static const int HIT_W = 96;
    static const int HIT_H = 96;

    std::optional<sf::Sprite> sprite;
    sf::Texture* texWalkNormal = nullptr;
    sf::Texture* texWalkNormalRev = nullptr;
    sf::Texture* texWalkAgressive = nullptr;
    sf::Texture* texWalkAgressiveRev = nullptr;
    sf::Texture* texAttackNormal = nullptr;
    sf::Texture* texAttackNormalRev = nullptr;
    sf::Texture* texAttackAgressive = nullptr;
    sf::Texture* texAttackAgressiveRev = nullptr;

    Anim animWalk, animAttack;
    bool useSprite = false;

    sf::RectangleShape hitbox;
    float vx = 0, vy = 0;
    bool onGround = false;
    bool facingRight = true;
    bool isAttacking = false;
    float attackCooldown = 0.f;
    float hurtTimer = 0.f;
    float invincible = 0.f;
    int hp = 10, maxHp = 10, score = 0;
    int attackDamage = 1;
    bool attackDealt = false;

    bool rageActive = false;
    float rageTimer = 0.f;
    float rageCooldown = 0.f;

    sf::SoundBuffer* hitSoundBuffer = nullptr;
    sf::Sound* hitSound = nullptr;

    Player();
    ~Player();
    void loadTextures(sf::Texture& walkNormal, sf::Texture& walkNormalRev,
        sf::Texture& walkAgressive, sf::Texture& walkAgressiveRev,
        sf::Texture& attackNormal, sf::Texture& attackNormalRev,
        sf::Texture& attackAgressive, sf::Texture& attackAgressiveRev);
    sf::FloatRect getBounds() const;
    void draw(sf::RenderWindow& win);
    void update(float dt);
    void attack();
    void activateRage();
    void takeDamage(int dmg);
    bool canHit(Enemy& e);

private:
    void updateSpriteTexture();
};

class Particle {
public:
    sf::CircleShape shape;
    float vx, vy, life;
    Particle(float x, float y);
    bool update(float dt);
};

class ParticleSystem {
private:
    std::vector<Particle> particles;
public:
    void emit(float x, float y, int count);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    void clear();
};