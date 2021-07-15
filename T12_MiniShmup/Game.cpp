#include <assert.h>
#include <string>
#include <math.h>
#include <sstream>
#include <algorithm>

#include "Game.h"

using namespace std;
using namespace sf;

/*
Seed random number generator
s=a fixed value to use (for debugging perhaps) or
let it default to -1 and time is used instead
*/
void Seed(int s = -1)
{
	if (s == -1)
		srand((unsigned int)time(0));
	else
		srand(s);
}

/*
float value between min and max inclusive
*/
float GetRandRange(float min, float max)
{
	float alpha = rand() / (float)RAND_MAX;
	alpha = min + (max - min) * alpha;
	return alpha;
}

/*
integer variant
*/
int GetRandRange(int min, int max)
{
	float alpha = GetRandRange(0.f, 1.f);
	return min + (int)round((max - min) * alpha);
}

/*
Bubble sorter
sprites - a vector of GameObjects to sort by z
*/
void Bubble(vector<Background>& sprites)
{
	if (sprites.size() <= 1)
		return;
	bool busy;
	do
	{
		busy = false;
		for (size_t i = 0; i < (sprites.size() - 1); ++i)
		{
			if (sprites[i].z > sprites[i + 1].z)
			{
				Background o = sprites[i];
				sprites[i] = sprites[i + 1];
				sprites[i + 1] = o;
				busy = true;
			}
		}

	} while (busy);
}


void Background::Update(RenderWindow& window, float elapsed)
{
	Vector2f pos = spr.getPosition();
	pos.x -= elapsed * GC::BG_MAX_SPEED * z;
	if (pos.x < -spr.getTextureRect().width)
	{
		if ((*pTex).isRepeated())
			pos.x += spr.getTextureRect().width;
		else
			pos.x = (float)(window.getSize().x);
	}
	spr.setPosition(pos);
}

void Object::InitShip(RenderWindow& window, Texture& tex)
{
	spr.setTexture(tex, true);
	const IntRect& texRect = spr.getTextureRect();
	spr.setOrigin(texRect.width / 2.f, texRect.height / 2.f);
	spr.setScale(0.2f, 0.2f);
	spr.setRotation(90);
	spr.setPosition(spr.getGlobalBounds().width * 0.6f, window.getSize().y / 2.f);
	type = ObjectT::Ship;
	radius = 25.f;
	active = true;
}

void Object::InitRock(RenderWindow& window, Texture& tex)
{
	spr.setTexture(tex);
	IntRect texR(0, 0, 96, 96);
	spr.setTextureRect(texR);
	spr.setOrigin(texR.width / 2.f, texR.height / 2.f);
	radius = 10.f + (float)(rand() % 30);
	float scale = 0.75f * (radius / 25.f);
	spr.setScale(scale, scale);
	health = (int)(5 * scale);
	active = false;
	type = ObjectT::Rock;
}

void Object::InitBullet(RenderWindow& window, Texture& tex)
{
	spr.setTexture(tex);
	IntRect texR(0, 0, 32, 32);
	spr.setTextureRect(texR);
	spr.setOrigin(texR.width / 2.f, texR.height / 2.f);
	radius = 5.f;
	float scale = 0.5f;
	spr.setScale(scale, scale);
	active = false;
	type = ObjectT::Bullet;
}

void Object::Init(RenderWindow& window, Texture& tex, ObjectT type_)
{
	switch (type_)
	{
	case ObjectT::Ship:
		InitShip(window, tex);
		break;
	case ObjectT::Rock:
		InitRock(window, tex);
		break;
	case ObjectT::Bullet:
		InitBullet(window, tex);
		break;
	default:
		assert(false);
	}
}

void Object::Update(RenderWindow& window, float elapsed, vector<Object>& objects, bool fire)
{
	if (active)
	{
		colliding = false;
		switch (type)
		{
		case ObjectT::Ship:
			PlayerControl(window.getSize(), elapsed, objects, fire);
			break;
		case ObjectT::Rock:
			MoveRock(elapsed);
			break;
		case ObjectT::Bullet:
			MoveBullet(window.getSize(), elapsed);
			break;
		}
	}
}

void Object::MoveRock(float elapsed)
{
	const Vector2f& pos = spr.getPosition();
	float x = pos.x - GC::ROCK_SPEED * elapsed;
	if (x < -spr.getGlobalBounds().width / 2.f)
		active = false;
	spr.setPosition(x, pos.y);
}

void Object::MoveBullet(const sf::Vector2u& screenSz, float elapsed)
{
	const Vector2f& pos = spr.getPosition();
	float x = pos.x + 250 * elapsed;
	if (x > (screenSz.x + spr.getGlobalBounds().width / 2.f))
		active = false;
	spr.setPosition(x, pos.y);
}

void Object::Render(RenderWindow& window)
{
	if (active)
		window.draw(spr);
}

Vector2f Decay(Vector2f& currentVal, float rate, float perSec, float dTimeS)
{
	float mod = 1.0f - rate * (dTimeS / perSec);
	Vector2f alpha(currentVal.x * mod, currentVal.y * mod);
	return alpha;
}

void Object::PlayerControl(const Vector2u& screenSz, float elapsed, std::vector<Object>& objects, bool fire)
{
	Vector2f pos = spr.getPosition();
	const float SPEED = 250.f;
	FloatRect rect = spr.getGlobalBounds();

	static Vector2f thrust{ 0,0 };

	if (Keyboard::isKeyPressed(Keyboard::Up) ||
		Keyboard::isKeyPressed(Keyboard::Down) ||
		Keyboard::isKeyPressed(Keyboard::Left) ||
		Keyboard::isKeyPressed(Keyboard::Right))
	{
		if (Keyboard::isKeyPressed(Keyboard::Up))
			thrust.y = -SPEED;
		else if (Keyboard::isKeyPressed(Keyboard::Down))
			thrust.y = SPEED;
		if (Keyboard::isKeyPressed(Keyboard::Left))
			thrust.x = -SPEED;
		else if (Keyboard::isKeyPressed(Keyboard::Right))
			thrust.x = SPEED;
	}

	pos += thrust * elapsed;
	thrust = Decay(thrust, 0.1f, 0.02f, elapsed);

	if (pos.y < (rect.height * 0.6f))
		pos.y = rect.height * 0.6f;
	if (pos.y > (screenSz.y - rect.height * 0.6f))
		pos.y = screenSz.y - rect.height * 0.6f;
	if (pos.x < (rect.width * 0.6f))
		pos.x = rect.width * 0.6f;
	if (pos.x > (screenSz.x - rect.width * 0.6f))
		pos.x = screenSz.x - rect.width * 0.6f;

	spr.setPosition(pos);

	if (fire)
	{
		Vector2f pos(pos.x + spr.getGlobalBounds().width / 2.f, pos.y);
		FireBullet(pos, objects);
	}
}

void Object::FireBullet(const Vector2f& pos, std::vector<Object>& objects)
{
	size_t idx = 0;
	bool found = false;
	while (idx < objects.size() && !found)
	{
		if (!objects[idx].active && objects[idx].type == ObjectT::Bullet)
			found = true;
		else
			++idx;
	}
	if (idx < objects.size())
	{
		objects[idx].active = true;
		objects[idx].spr.setPosition(pos);
	}
}

void Object::Hit(Object& other)
{
	switch (type)
	{
	case ObjectT::Ship:
		if (other.type == ObjectT::Rock)
		{
			TakeDamage(1);
			other.TakeDamage(999);
		}
		break;
	case ObjectT::Bullet:
		if (other.type == ObjectT::Rock)
		{
			TakeDamage(1);
			other.TakeDamage(1);
		}
		break;
	case ObjectT::Rock:
		break;
	default:
		assert(false);
	}
}

void Object::TakeDamage(int amount)
{
	health -= amount;
	if (health <= 0)
		active = false;
}

bool LoadTexture(const string& file, Texture& tex)
{
	if (tex.loadFromFile(file))
	{
		tex.setSmooth(true);
		return true;
	}
	assert(false);
	return false;
}

bool LoadTextureImg(const Image& img, Texture& tex, const IntRect& rect, const bool& repeat)
{
	if (tex.loadFromImage(img, rect))
	{
		tex.setSmooth(true);
		tex.setRepeated(repeat);
		return true;
	}
	assert(false);
	return false;
}

void DrawCircle(RenderWindow& window, const Vector2f& pos, float radius, Color col)
{
	CircleShape c;
	c.setRadius(radius);
	c.setPointCount(20);
	c.setOutlineColor(col);
	c.setOutlineThickness(2);
	c.setFillColor(Color::Transparent);
	c.setPosition(pos);
	c.setOrigin(radius, radius);
	window.draw(c);
}

bool CircleToCircle(const Vector2f& pos1, const Vector2f& pos2, float minDist)
{
	float dist = (pos1.x - pos2.x) * (pos1.x - pos2.x) +
		(pos1.y - pos2.y) * (pos1.y - pos2.y);
	dist = sqrtf(dist);
	return dist <= minDist;
}

void CheckCollisions(vector<Object>& objects, RenderWindow& window, bool debug)
{
	if (objects.size() > 1)
	{
		for (size_t i = 0; i < objects.size(); ++i)
		{
			Object& a = objects[i];
			if (a.active)
			{
				if (i < (objects.size() - 1))
					for (size_t ii = i + 1; ii < (objects.size()); ++ii)
					{
						Object& b = objects[ii];
						if (b.active)
						{
							if (CircleToCircle(a.spr.getPosition(), b.spr.getPosition(), a.radius + b.radius))
							{
								a.colliding = true;
								b.colliding = true;
								a.Hit(b);
								b.Hit(a);
							}
						}
					}
				if (debug)
				{
					Color col = Color::Green;
					if (a.colliding)
						col = Color::Red;
					DrawCircle(window, a.spr.getPosition(), a.radius, col);
				}
			}
		}
	}
}


bool IsColliding(Object& obj, vector<Object>& objects)
{
	assert(obj.active);
	size_t idx = 0;
	bool colliding = false;
	while (idx < objects.size() && !colliding) {

		if (&obj != &objects[idx] && objects[idx].active)
		{
			const Vector2f& posA = obj.spr.getPosition();
			const Vector2f& posB = objects[idx].spr.getPosition();
			float dist = obj.radius + objects[idx].radius;
			colliding = CircleToCircle(posA, posB, dist);
		}
		++idx;
	}
	return colliding;
}


void PlaceRocks(RenderWindow& window, Texture& tex, vector<Object>& objects)
{
	bool space = true;
	int ctr = GC::NUM_ROCKS;
	while (space && ctr)
	{
		Object rock;
		rock.Init(window, tex, Object::ObjectT::Rock);
		rock.radius *= GC::ROCK_MIN_DIST;
		int tries = 0;
		do {
			tries++;
			float x = (float)(rand() % window.getSize().x);
			float y = (float)(rand() % window.getSize().y);
			rock.spr.setPosition(x, y);
		} while (tries < GC::PLACE_TRIES && IsColliding(rock, objects));
		rock.radius *= 1 / GC::ROCK_MIN_DIST;
		if (tries != GC::PLACE_TRIES)
			objects.push_back(rock);
		else
			space = false;
		--ctr;
	}
}

bool SpawnRock(RenderWindow& window, vector<Object>& objects, float extraClearance)
{
	size_t idx = 0;
	bool found = false;
	while (idx < objects.size() && !found)
	{
		Object& obj = objects[idx];
		if (!obj.active && obj.type == Object::ObjectT::Rock)
			found = true;
		else
			++idx;
	}

	if (found)
	{
		Object& obj = objects[idx];
		obj.active = true;
		obj.radius += extraClearance;
		FloatRect r = obj.spr.getGlobalBounds();
		float y = (r.height / 2.f) + (rand() % (int)(window.getSize().y - r.height));
		obj.spr.setPosition(window.getSize().x + r.width, y);
		if (IsColliding(obj, objects))
		{
			found = false;
			obj.active = false;
		}
		obj.radius -= extraClearance;
	}
	return found;
}

void GenerateBgTextures(const Image& img, Texture& tex, const char& type)
{
	switch (type)
	{
	case 'b': //non-random background
		if (!tex.create(GC::SCREEN_RES.x, GC::SCREEN_RES.y))
			assert(false);
		LoadTextureImg(img, tex, GC::BG_SPR[1], !GC::REPEAT);
		LoadTextureImg(img, tex, GC::BG_SPR[0], GC::REPEAT);
		break;
	case 'm': //random mountains

	case 'c': //random clouds

	};
}

void GenerateBgRandom(vector<Background>& bg)
{
	int i;
	switch (bg[i].type)
	{
	case 'a': //non-random background
		/*if (!tex.create(GC::SCREEN_RES.x, GC::SCREEN_RES.y))
			assert(false);
		LoadTextureImg(img, tex, GC::BG_SPR[1], !GC::REPEAT);
		LoadTextureImg(img, tex, GC::BG_SPR[0], GC::REPEAT);
		break;*/
	case 'm': //random mountains

	case 'c': //random clouds

	};
}

void Game::Init(sf::RenderWindow& window)
{
	LoadTexture("data/ship.png", texShip);
	LoadTexture("data/asteroid.png", texRock);
	LoadTexture("data/missile-01.png", texBullet);

	objects.clear();
	Object rock;
	rock.Init(window, texRock, Object::ObjectT::Rock);
	objects.insert(objects.begin(), GC::NUM_ROCKS + 1, rock);
	objects[0].Init(window, texShip, Object::ObjectT::Ship);
	for (size_t i = 1; i < objects.size(); ++i)
		objects[i].Init(window, texRock, Object::ObjectT::Rock);

	Object bullet;
	bullet.Init(window, texBullet, Object::ObjectT::Bullet);
	objects.insert(objects.end(), 50, bullet);

	spawnTimer = 0;
	spawnDelay = 0.01f;
	rockShipClearance = objects[0].spr.getGlobalBounds().width * 2.f;

	//Seed();

	bgSpriteSheet.loadFromFile("data\\bgSpriteSheet.png");

	/*backgrounds.insert(trees.begin(), GC::NUM_TREES, Tree());
	for (size_t i = 0; i < trees.size(); ++i)
	{
		Tree& o = trees[i];
		o.spr.setTexture(texTrees);
		int idx = GetRandRange(0, GC::MAX_TREEGFX - 1);
		o.spr.setTextureRect(GC::TREE_SPR[idx]);
		o.spr.setOrigin(GC::TREE_SPR[0].width / 2.f, GC::TREE_SPR[0].height / 2.f);
		o.z = GetRandRange(0.25f, 1.f);
		o.spr.setScale(o.z * 1.5f, o.z * 1.5f);
		o.spr.setPosition(GetRandRange(0.f, (float)window.getSize().x + o.spr.getTextureRect().width),
			(float)window.getSize().y * 0.9f - (150 * (1.f - o.z)));

	}*/
	/*
	std::sort(sprites.begin(), sprites.end(), [](GameObj& o1, GameObj& o2){
		return o1.z < o2.z;
	});
	*/
	//Bubble(trees);
}

void Game::Update(sf::RenderWindow& window, float elapsed, bool fire)
{
	spawnTimer += elapsed;
	if (spawnTimer >= spawnDelay)
	{
		if (SpawnRock(window, objects, rockShipClearance))
			spawnTimer = 0;
	}

	CheckCollisions(objects, window, false);
	for (size_t i = 0; i < objects.size(); ++i)
		objects[i].Update(window, elapsed, objects, fire);

	for (size_t i = 0; i < backgrounds.size(); ++i)
		backgrounds[i].Update(window, elapsed);
}

void Game::Render(sf::RenderWindow& window)
{
	for (size_t i = 0; i < objects.size(); ++i)
		objects[i].Render(window);

	for (size_t i = 0; i < backgrounds.size(); ++i)
		window.draw(backgrounds[i].spr);
}
