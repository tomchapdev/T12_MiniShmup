#pragma once

#include <vector>
#include "SFML/Graphics.hpp"

//dimensions in 2D that are whole numbers
struct Dim2Di
{
	int x, y;
};

//dimensions in 2D that are floating point numbers
struct Dim2Df
{
	float x, y;
};

/*
A box to put Games Constants in.
These are special numbers with important meanings (screen width,
ascii code for the escape key, number of lives a player starts with,
the name of the title screen music track, etc.
*/
namespace GC
{
	//game play related constants to tweak
	const Dim2Di SCREEN_RES{800,600};	//game window dimensions
	const int FRAMERATE_MAX = 60;		//maximum framerate
	const float SPEED = 250.f;			//ship speed
	const float SCREEN_EDGE = 0.6f;		//how close to the edge the ship can get
	const char ESCAPE_KEY{27};
	const float ROCK_MIN_DIST = 2.15f;	//used when placing rocks to stop them getting too close
	const int NUM_ROCKS = 500;			//how many to place
	const int PLACE_TRIES = 10;			//how many times to try and place before giving up
	const float ROCK_SPEED = 150.f;		//max speed of asteroids

	const float BG_SPEED_MAX = 50.f;		//max speed of background sprites
	const float BG_SPEED_MIN = 7.5f;		//min speed of background sprites
	const float BG_SCALE_MAX = 1.0f;			//max size of background sprites
	const float BG_SCALE_MIN = 0.3f;			//min size of background sprites
	const float BG_SCALE_RANGE = BG_SCALE_MAX - BG_SCALE_MIN;	//Difference between max and min background sprite sizes
	const int BG_NUM_MAX = 12;				//maximum number of rng background images
	const int BG_NUM_MIN = 6;				//minimum number of rng background images
	const int BG_PNG_NUM = 8;				//8 background sprites on spritesheet
	const int BG_PNG_RNG = 6;				//6 background sprites used dynamically
	const Dim2Di BG_PNG_SIZE = {512,256};	//size (x,y) of background images
	const Dim2Df BG_SCALE_RATIO = {			//scale ratio to fill screen
		((float)SCREEN_RES.x / (float)BG_PNG_SIZE.x),
		((float)SCREEN_RES.y / (float)BG_PNG_SIZE.y)
	};
	const int BG_Z_MAX = 32 * BG_SCALE_RATIO.y;	//max depth (mountain base height)
	const float BG_Z_FAR = 0.8f;				//Past this use a dark far away texture
	const bool REPEAT = true;
}

/*
a background object
*/
struct Background
{
	float z = 0;						//faked 3D depth - done using parallax and scaling
	float speed = GC::BG_SPEED_MIN;		//speed of this background object
	sf::Sprite spr;						//image and position

	void Update(sf::RenderWindow& window, float elapsed);
};

/*
A game object that could be a rock or the player
Objects are anything with a sprite that can move around the screen
and collide with other objets.
*/
struct Object
{
	sf::Sprite spr;	//main image
	float radius = 0;	//collision radius
	enum class ObjectT { Ship, Rock, Bullet };	//what is this?
	ObjectT type = ObjectT::Rock;
	bool colliding = false; //did we hit something on the last update
	bool active = false;	//should we be updating and rendering this one?
	int health = 1;			//go inactive if health <= 0

	/*
	Call this to setup your object
	window - sfml render window
	tex - texture to use on the sprite
	type - what is it meant to be
	*/
	void Init(sf::RenderWindow& window, sf::Texture& tex, ObjectT type_);
	//called by Init as needed
	void InitShip(sf::RenderWindow& window, sf::Texture& tex);
	//called by Init as needed
	void InitRock(sf::RenderWindow& window, sf::Texture& tex);
	//move and update logic
	void Update(sf::RenderWindow& window, float elapsed, std::vector<Object>& objects, bool fire);
	//draw
	void Render(sf::RenderWindow& window);
	//handle moving the ship around
	void PlayerControl(const sf::Vector2u& screenSz, float elapsed, std::vector<Object>& objects, bool fire);
	//rocks all move left, when leave the left edge of the screen they deactivate
	void MoveRock(float elapsed);
	//bullets move right
	void MoveBullet(const sf::Vector2u& screenSz, float elapsed);
	//called by Init to set up a bullet
	void InitBullet(sf::RenderWindow& window, sf::Texture& tex);
	//find an inactive bullet, activate it, set its position to start it flying
	void FireBullet(const sf::Vector2f& pos, std::vector<Object>& objects);
	//work out what to do when two objects hit each other
	void Hit(Object& other);
	//reduce health and then deactivate when it hits zero
	void TakeDamage(int amount);
};

/*
Manage the asteroid dodging game
*/
struct Game
{
	sf::Texture texShip;
	sf::Texture texRock;
	sf::Texture texBullet;
	std::vector<Object> objects;	//anything moving around
	float spawnTimer;				//a clock
	float spawnDelay;				//how long to wait before another asteroid comes in, decrease to make harder
	float rockShipClearance = 2.f;	//when placing an asteroid, how many ship lengths away from other rocks should it be, harder = smaller

	sf::Texture texBgSky, texBgGround;		//static background texture
	std::vector<Background> backgrounds;	//parallax backgrounds

	//initialize Backgrounds
	void GenerateBgTextures();
	//Generates the randomized parallax background
	void GenerateBgRandom();
	//load textures, create ship and rocks, set all rocks initially inactive
	void Init(sf::RenderWindow& window);
	//move the ship and rocks, spawn new rocks 
	void Update(sf::RenderWindow& window, float elapsed, bool fire);
	//draw everything
	void Render(sf::RenderWindow& window);
};

/*
Update every object to see if it is colliding with any other - sets the colliding flag true
objects - any could be colliding
debug - if true, draw the collision radius and mark any collisions in red
*/
void CheckCollisions(std::vector<Object>& objects, sf::RenderWindow& window, bool debug = true);

/*
Draws circle for debugging
*/
void DrawCircle(sf::RenderWindow& window, const sf::Vector2f& pos, float radius, sf::Color col);

/*
file - path and file name and extension
tex - set this up with the texture
*/
bool LoadTexture(const std::string& file, sf::Texture& tex);

/*
Check if two circles are touching
pos1,pos2 - two centres
minDist - minimum colliding distance
*/
bool CircleToCircle(const sf::Vector2f& pos1, const sf::Vector2f& pos2, float minDist);

/*
Test one object against an array of other objects to see if it collides
It's OK if the object happens to be in the array, it won't test against itself
*/
bool IsColliding(Object& obj, std::vector<Object>& objects);

/*
Setup a new rock to fly in from the right
Look through the objects array, find an inactive rock, pick a new starting position
for it just off screen to the right. Check it is at least extraClearance units away
from anything else and mark active.
If it does collide with something then don't spawn and return false.
*/
bool SpawnRock(sf::RenderWindow& window, std::vector<Object>& objects, float extraClearance);
#pragma once