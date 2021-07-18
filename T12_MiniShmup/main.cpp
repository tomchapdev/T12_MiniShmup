#include <assert.h>
#include <string>
#include "Game.h"
#include "SFML/Graphics.hpp"

using namespace sf;
using namespace std;

int main()
{
	// Create the main window
	RenderWindow window(VideoMode(GC::SCREEN_RES.x, GC::SCREEN_RES.y), "T12_MiniShmup");
	window.setFramerateLimit(GC::FRAMERATE_MAX);

	Game game;
	game.Init(window);
	//PlaceRocks(window, texRock, objects);

	Clock clock;

	// Start the game loop 
	while (window.isOpen())
	{
		bool fire = false;
		// Process events
		Event event;
		while (window.pollEvent(event))
		{
			// Close window: exit
			if (event.type == Event::Closed)
				window.close();
			else if (event.type == Event::TextEntered)
			{
				if (event.text.unicode == GC::ESCAPE_KEY)
					window.close();
			}
			else if (event.type == Event::KeyReleased)
			{
				if (event.key.code == Keyboard::Space)
					fire = true;
			}
		}

		// Clear screen
		window.clear();

		float elapsed = clock.getElapsedTime().asSeconds();
		clock.restart();

		game.Update(window, elapsed, fire);
		game.Render(window);

		// Update the window
		window.display();
	}

	return EXIT_SUCCESS;
}