#include "Entities.h"
#include "Constants.h"
#include "Animation.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <string>

using namespace std;

Enemy::Enemy(float x, float y, bool boss, int type) : isBoss(boss), enemyType(type) {
    W_ = boss ? 90.f : 64.f;
    H_ = boss ? 110.f : 64.f;
    body.setSize(sf::Vector2f{ W_, H_ });
    body.setPosition(sf::Vector2f{ x, y });

    if (enemyType == 1) body.setFillColor(sf::Color(100, 50, 200));
    else body.setFillColor(boss ? sf::Color(120, 20, 20) : sf::Color(60, 160, 60));

    hp = boss ? 30 : (enemyType == 1 ? ENEMY_BASE_HP - 2 : ENEMY_BASE_HP);
    maxHp = hp;
    damage = boss ? 2 : 1;

    eyeShape.setSize(sf::Vector2f{ 18.f, 12.f });
    eyeShape.setFillColor(sf::Color::White);
    pupilShape.setRadius(4.f);
    pupilShape.setFillColor(sf::Color::Black);

    hpBarBg.setSize(sf::Vector2f{ W_, 6.f });
    hpBarBg.setFillColor(sf::Color(40, 40, 40));
    hpBarFill.setSize(sf::Vector2f{ W_, 6.f });
    hpBarFill.setFillColor(sf::Color(255, 50, 50));

    projectile.setRadius(8.f);
    projectile.setFillColor(sf::Color(200, 100, 255));
}

sf::FloatRect Enemy::getBounds() const { return body.getGlobalBounds(); }

void Enemy::draw(sf::RenderWindow& win, const sf::Font& font) {
    if (!alive) return;

    body.setFillColor(hurtTimer > 0.f ? sf::Color::White :
        (enemyType == 1 ? sf::Color(100, 50, 200) :
            (isBoss ? sf::Color(120, 20, 20) : sf::Color(60, 160, 60))));
    win.draw(body);

    float bx = body.getPosition().x;
    float by = body.getPosition().y;

    hpBarBg.setPosition(sf::Vector2f{ bx, by - 10.f });
    float hpRatio = (float)hp / (float)maxHp;
    hpBarFill.setSize(sf::Vector2f{ W_ * hpRatio, 6.f });
    hpBarFill.setPosition(sf::Vector2f{ bx, by - 10.f });
    win.draw(hpBarBg);
    win.draw(hpBarFill);

    sf::Text hpTxt(font);
    hpTxt.setString(to_string(hp) + "/" + to_string(maxHp));
    hpTxt.setCharacterSize(10);
    hpTxt.setFillColor(sf::Color::White);
    hpTxt.setPosition(sf::Vector2f{ bx, by - 22.f });
    win.draw(hpTxt);

    eyeShape.setPosition(sf::Vector2f{ bx + 10.f, by + 12.f });
    pupilShape.setPosition(sf::Vector2f{ bx + 18.f, by + 13.f });
    win.draw(eyeShape);
    win.draw(pupilShape);

    drawProjectile(win);
}

void Enemy::update(float dt, float playerX) {
    if (!alive) return;
    hurtTimer -= dt;
    rangedAttackTimer -= dt;
    projectileHitTimer -= dt;

    float myX = body.getPosition().x + W_ / 2.f;
    float speed = isBoss ? 130.f : (enemyType == 1 ? 70.f : 90.f);

    if (enemyType == 1 && !retreating) {
        float distToPlayer = abs(myX - playerX);
        if (distToPlayer < 200.f) {
            retreating = true;
            retreatTimer = 0.3f;
            retreatDirection = (playerX < myX) ? 1.f : -1.f;
        }
    }
    else if (!retreating) {
        retreatTimer -= dt;
        if (retreatTimer <= 0.f) {
            if (abs(myX - playerX) < 100.f) {
                retreating = true;
                retreatTimer = 0.5f;
                retreatDirection = (playerX < myX) ? 1.f : -1.f;
            }
        }
    }
    else {
        retreatTimer -= dt;
        if (retreatTimer <= 0.f) {
            retreating = false;
            retreatTimer = 1.5f + (float)(rand() % 100) / 100.f;
        }
    }

    if (retreating) vx = retreatDirection * speed * 0.8f;
    else vx = (playerX < myX) ? -speed : speed;

    if (!onGround) vy += GRAV * dt;
    body.move(sf::Vector2f{ vx * dt, vy * dt });

    float bottom = body.getPosition().y + H_;
    if (bottom >= GROUND) {
        body.setPosition(sf::Vector2f{ body.getPosition().x, GROUND - H_ });
        vy = 0.f;
        onGround = true;
    }
    else onGround = false;

    updateProjectile(dt);
}

bool Enemy::tryAttack(sf::FloatRect playerBounds, float dt) {
    attackTimer -= dt;
    if (attackTimer > 0.f) return false;

    sf::FloatRect hitbox = getBounds();
    hitbox.position.x -= 10.f;
    hitbox.size.x += 20.f;

    if (hitbox.findIntersection(playerBounds)) {
        attackTimer = isBoss ? 1.5f : 2.0f;
        return true;
    }
    return false;
}

void Enemy::tryRangedAttack(float playerX, float playerY) {
    if (rangedAttackTimer > 0.f || projectileActive) return;

    float myX = body.getPosition().x + W_ / 2.f;
    float myY = body.getPosition().y + H_ / 2.f;
    float dist = (float)sqrt(pow(playerX - myX, 2) + pow(playerY - myY, 2));

    if (dist < 500.f) {
        projectileActive = true;
        projectile.setPosition(sf::Vector2f{ myX, myY });
        float angle = atan2(playerY - myY, playerX - myX);
        float speed = 300.f;
        projectileVx = cos(angle) * speed;
        projectileVy = sin(angle) * speed;
        rangedAttackTimer = 2.0f;
    }
}

void Enemy::updateProjectile(float dt) {
    if (!projectileActive) return;
    projectile.move(sf::Vector2f{ projectileVx * dt, projectileVy * dt });
    if (projectile.getPosition().x < 0 || projectile.getPosition().x > W || projectile.getPosition().y > GROUND)
        projectileActive = false;
}

void Enemy::drawProjectile(sf::RenderWindow& win) { if (projectileActive) win.draw(projectile); }

bool Enemy::checkProjectileHit(sf::FloatRect playerBounds) {
    if (!projectileActive || projectileHitTimer > 0.f) return false;
    if (projectile.getGlobalBounds().findIntersection(playerBounds)) {
        projectileActive = false;
        projectileHitTimer = 0.5f;
        return true;
    }
    return false;
}

void Enemy::takeDamage(int dmg) { hp -= dmg; hurtTimer = 0.12f; if (hp <= 0) alive = false; }

// ==================== Player ====================
Player::Player() {
    hitbox.setSize(sf::Vector2f{ (float)HIT_W, (float)HIT_H });
    hitbox.setPosition(sf::Vector2f{ 120.f, GROUND - (float)HIT_H });
    hitbox.setFillColor(sf::Color(139, 90, 43, 30));
    hitbox.setOutlineColor(sf::Color(255, 255, 0, 100));
    hitbox.setOutlineThickness(1.f);

    hitSoundBuffer = new sf::SoundBuffer();
    (void)hitSoundBuffer->loadFromFile("sounds/hit_ursa.flac");
    hitSound = new sf::Sound(*hitSoundBuffer);
}

Player::~Player() { if (hitSound) delete hitSound; if (hitSoundBuffer) delete hitSoundBuffer; }

void Player::loadTextures(sf::Texture& walkNormal, sf::Texture& walkNormalRev,
    sf::Texture& walkAgressive, sf::Texture& walkAgressiveRev,
    sf::Texture& attackNormal, sf::Texture& attackNormalRev,
    sf::Texture& attackAgressive, sf::Texture& attackAgressiveRev) {
    texWalkNormal = &walkNormal; texWalkNormalRev = &walkNormalRev;
    texWalkAgressive = &walkAgressive; texWalkAgressiveRev = &walkAgressiveRev;
    texAttackNormal = &attackNormal; texAttackNormalRev = &attackNormalRev;
    texAttackAgressive = &attackAgressive; texAttackAgressiveRev = &attackAgressiveRev;

    animWalk.tex = texWalkNormal; animWalk.frameW = SPR_W; animWalk.frameH = SPR_H;
    animWalk.cols = 1; animWalk.total = 2; animWalk.fps = 4.f; animWalk.loop = true;

    animAttack.tex = texAttackNormal; animAttack.frameW = SPR_W; animAttack.frameH = SPR_H;
    animAttack.cols = 2; animAttack.total = 4; animAttack.fps = 12.f; animAttack.loop = false;

    sprite.emplace(*texWalkNormal); useSprite = true;
}

void Player::updateSpriteTexture() {
    if (!useSprite || !sprite.has_value()) return;
    if (isAttacking) {
        if (rageActive) animAttack.tex = facingRight ? texAttackAgressive : texAttackAgressiveRev;
        else animAttack.tex = facingRight ? texAttackNormal : texAttackNormalRev;
    }
    else {
        if (rageActive) animWalk.tex = facingRight ? texWalkAgressive : texWalkAgressiveRev;
        else animWalk.tex = facingRight ? texWalkNormal : texWalkNormalRev;
    }
}

sf::FloatRect Player::getBounds() const { return hitbox.getGlobalBounds(); }

void Player::draw(sf::RenderWindow& win) {
    float bx = hitbox.getPosition().x;
    float by = hitbox.getPosition().y;

    if (useSprite && sprite.has_value()) {
        updateSpriteTexture();
        Anim& a = isAttacking ? animAttack : animWalk;
        sprite->setTexture(*a.tex);
        sprite->setTextureRect(a.rect());

        if (hurtTimer > 0.f && (int)(hurtTimer * 10) % 2 == 0)
            sprite->setColor(sf::Color(255, 100, 100, 200));
        else sprite->setColor(sf::Color::White);

        float scaleX = (float)HIT_W / SPR_W;
        float scaleY = (float)HIT_H / SPR_H;
        sprite->setScale(sf::Vector2f{ scaleX, scaleY });
        sprite->setPosition(sf::Vector2f{ bx, by });
        win.draw(*sprite);
    }
    win.draw(hitbox);
}

void Player::update(float dt) {
    hurtTimer -= dt; attackCooldown -= dt; invincible -= dt; rageCooldown -= dt;
    if (rageActive) { rageTimer -= dt; if (rageTimer <= 0.f) rageActive = false; }

    Anim& a = isAttacking ? animAttack : animWalk;
    a.update(dt);

    if (isAttacking && animAttack.done) {
        isAttacking = false; attackDealt = false;
        animWalk.reset(); attackCooldown = ATTACK_COOLDOWN;
    }

    if (!onGround) vy += GRAV * dt;
    hitbox.move(sf::Vector2f{ vx * dt, vy * dt });

    float bottom = hitbox.getPosition().y + HIT_H;
    if (bottom >= GROUND) {
        hitbox.setPosition(sf::Vector2f{ hitbox.getPosition().x, GROUND - (float)HIT_H });
        vy = 0.f; onGround = true;
    }
    else onGround = false;

    float px = hitbox.getPosition().x;
    if (px < 0.f) hitbox.setPosition(sf::Vector2f{ 0.f, hitbox.getPosition().y });
    if (px + (float)HIT_W > W) hitbox.setPosition(sf::Vector2f{ (float)W - (float)HIT_W, hitbox.getPosition().y });
}

void Player::attack() {
    if (isAttacking || attackCooldown > 0.f) return;
    isAttacking = true; attackDealt = false; animAttack.reset();
    if (hitSound) hitSound->play();
}

void Player::activateRage() {
    if (rageCooldown > 0.f || rageActive) return;
    rageActive = true; rageTimer = RAGE_DURATION; rageCooldown = RAGE_COOLDOWN;
}

void Player::takeDamage(int dmg) {
    if (rageActive) return;
    if (invincible > 0.f) return;
    hp -= dmg; hurtTimer = 0.5f; invincible = 1.5f;
}

bool Player::canHit(Enemy& e) {
    if (!isAttacking || attackDealt) return false;
    if (animAttack.cur < 1) return false;

    float px = hitbox.getPosition().x; float pw = (float)HIT_W;
    float ex = e.body.getPosition().x; float ew = e.W_;
    float myEdge = facingRight ? px + pw : px;
    float enemyNear = facingRight ? ex : ex + ew;
    float dist = abs(myEdge - enemyNear);

    if (dist < 90.f) { attackDealt = true; return true; }
    return false;
}

// ==================== Particle ====================
Particle::Particle(float x, float y) {
    shape.setRadius(4.f); shape.setFillColor(sf::Color(255, 220, 50)); shape.setPosition(sf::Vector2f{ x, y });
    vx = (float)(rand() % 200 - 100) / 50.f * 120.f; vy = (float)(rand() % 100) / 50.f * (-180.f) - 60.f;
    life = 0.35f + (float)(rand() % 30) / 100.f;
}
bool Particle::update(float dt) {
    vy += 600.f * dt; shape.move(sf::Vector2f{ vx * dt, vy * dt }); life -= dt;
    uint8_t a = (uint8_t)max(0.f, life / 0.6f * 255.f); shape.setFillColor(sf::Color(255, 220, 50, a));
    return life > 0.f;
}
void ParticleSystem::emit(float x, float y, int count) { for (int i = 0; i < count; ++i) particles.emplace_back(x, y); }
void ParticleSystem::update(float dt) { for (auto it = particles.begin(); it != particles.end(); ) { if (it->update(dt)) ++it; else it = particles.erase(it); } }
void ParticleSystem::draw(sf::RenderWindow& w) { for (auto& p : particles) w.draw(p.shape); }
void ParticleSystem::clear() { particles.clear(); }