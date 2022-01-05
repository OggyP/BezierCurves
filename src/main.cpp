#include "Platform/Platform.hpp"

#define CURVE_RESOLUTION 100 // How many lines are used to make up the curve

std::vector<sf::Vector2f> curve;
std::vector<sf::Vector2f> points;

float naive_lerp(float a, float b, float t);
float naive_lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

std::vector<sf::Vector2f> calculateCurve(std::vector<sf::Vector2f> points);
std::vector<sf::Vector2f> calculateCurve(std::vector<sf::Vector2f> points)
{

	std::vector<sf::Vector2f> curvePoints;
	if (!points.size())
		return curvePoints;
	curvePoints.reserve(CURVE_RESOLUTION);

	for (uint lerpAmtCount = 0; lerpAmtCount <= CURVE_RESOLUTION; lerpAmtCount++)
	{
		const float lerpAmt = (float)lerpAmtCount / (float)CURVE_RESOLUTION;
		auto newPoints = points;
		while (newPoints.size() > 1)
		{
			std::vector<sf::Vector2f> tempPoints;
			auto startingPoint = newPoints[0];
			for (size_t i = 1; i < newPoints.size(); i++)
			{
				const auto& point = newPoints[i];
				sf::Vector2f newPoint(naive_lerp(startingPoint.x, point.x, lerpAmt), naive_lerp(startingPoint.y, point.y, lerpAmt));
				tempPoints.push_back(newPoint);
				startingPoint = point;
			}
			newPoints = tempPoints;
		}
		curvePoints.push_back(newPoints[0]);
	}
	return curvePoints;
}

int main()
{
	util::Platform platform;

#if defined(_DEBUG)
	std::cout << "Hello World!" << std::endl;
#endif

	sf::RenderWindow window;
	// in Windows at least, this must be called before creating the window
	float screenScalingFactor = platform.getScreenScalingFactor(window.getSystemHandle());
	// Use the screenScalingFactor
	window.create(sf::VideoMode(1280.0f * screenScalingFactor, 720.0f * screenScalingFactor), "Bezier Curve Visualiser!");
	platform.setIcon(window.getSystemHandle());
	window.setFramerateLimit(60);

	sf::Vertex line[2];
	line[0].color = sf::Color::White;
	line[1].color = sf::Color::White;
	sf::CircleShape circle;
	circle.setRadius(5);
	circle.setOrigin(5, 5);
	circle.setFillColor(sf::Color::Transparent);
	circle.setOutlineThickness(1);

	sf::Cursor clickCursor;
	sf::Cursor normalCursor;
	bool canSetCursor = false;
	if (clickCursor.loadFromSystem(sf::Cursor::Hand) && normalCursor.loadFromSystem(sf::Cursor::Arrow))
		canSetCursor = true;

	bool LWasPressesed = false;
	bool undoWasPressed = false;
	bool mouseWasPressed = false;

	bool showLines = true;

	auto randomPoint = sf::Vector2f(0, 0); // placeholder to stop gcc saying it is not initialised

	sf::Vector2f* pointSelected = &randomPoint;
	bool pointIsSelected = false;

	const bool drawStartPoints = true;
	float lerpAmt = 0.5;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::Resized)
			{
				// update the view to the new size of the window
				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				window.setView(sf::View(visibleArea));
			}
		}

		window.clear();

		if (window.hasFocus())
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
				if (lerpAmt >= 0.01)
					lerpAmt -= 0.01;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
				if (lerpAmt <= 0.99)
					lerpAmt += 0.01;

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
			{
				points.clear();
				curve = calculateCurve(points);
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::L))
			{
				if (!LWasPressesed)
				{
					showLines = !showLines;
					LWasPressesed = true;
					lerpAmt = 0.5;
				}
			}
			else
				LWasPressesed = false;

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::U))
			{
				if (!undoWasPressed)
				{
					undoWasPressed = true;
					if (points.size())
					{
						points.pop_back();
						curve = calculateCurve(points);
					}
				}
			}
			else
				undoWasPressed = false;
		}

		auto mousePos = sf::Mouse::getPosition(window);
		if (canSetCursor)
		{
			window.setMouseCursor(normalCursor);
			for (auto& point : points)
				if (pow(mousePos.x - point.x, 2) + pow(mousePos.y - point.y, 2) < 100)
				{
					window.setMouseCursor(clickCursor);
					break;
				}
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && window.hasFocus() && mousePos.x > 0 && mousePos.y > 0)
		{
			if (!mouseWasPressed)
			{
				bool foundPoint = false;
				for (auto& point : points)
					if (pow(mousePos.x - point.x, 2) + pow(mousePos.y - point.y, 2) < 100)
					{
						foundPoint = true;
						pointIsSelected = true;
						pointSelected = &point;
					}
				mouseWasPressed = true;

				if (!foundPoint)
				{
					points.push_back(sf::Vector2f(mousePos));
					curve = calculateCurve(points);
				}
			}
			else
			{
				if (pointIsSelected)
				{
					(*pointSelected).x = mousePos.x;
					(*pointSelected).y = mousePos.y;
					curve = calculateCurve(points);
				}
			}
		}
		else
		{
			mouseWasPressed = false;
			pointIsSelected = false;
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && window.hasFocus())
		{
			for (size_t i = 0; i < points.size(); i++)
			{
				const auto& point = points[i];
				if (pow(mousePos.x - point.x, 2) + pow(mousePos.y - point.y, 2) < 100)
				{
					points.erase(points.begin() + i);
					curve = calculateCurve(points);
				}
			}
		}

		if (points.size())
		{

			if (showLines)
			{
				circle.setOutlineColor(sf::Color::Green);

				auto newPoints = points;

				const float colourChangeAmt = 205.0f / (float)(points.size() - 2);
				int lerpLayer = -1;
				while (newPoints.size() > 1)
				{
					const float showColour = 255.0f - lerpLayer * colourChangeAmt;
					std::vector<sf::Vector2f> tempPoints;
					auto startingPoint = newPoints[0];
					line[0].position = newPoints[0];
					for (size_t i = 1; i < newPoints.size(); i++)
					{
						const auto& point = newPoints[i];
						sf::Vector2f newPoint(naive_lerp(startingPoint.x, point.x, lerpAmt), naive_lerp(startingPoint.y, point.y, lerpAmt));
						tempPoints.push_back(newPoint);
						startingPoint = point;
						if (lerpLayer != -1)
						{
							line[0].color = sf::Color(0, (float)showColour, 0);
							line[1].color = sf::Color(0, (float)showColour, 0);
							line[1].position = point;
							window.draw(line, 2, sf::Lines);
							line[0].position = line[1].position;
						}
					}
					newPoints = tempPoints;
					lerpLayer++;
				}
				circle.setPosition(newPoints[0]);
				window.draw(circle);
			}

			line[0].color = sf::Color::Red;
			line[1].color = sf::Color::Red;

			circle.setOutlineColor(sf::Color::Red);

			if (curve.size())
			{
				line[0].position = curve[0];
				for (size_t i = 1; i < curve.size(); i++)
				{
					const auto& point = curve[i];
					line[1].position = point;
					window.draw(line, 2, sf::Lines);
					line[0].position = line[1].position;
					// uncomment to draw each vertex
					// circle.setPosition(point);
					// window.draw(circle);
				}
			}

			line[0].color = sf::Color::White;
			line[1].color = sf::Color::White;

			circle.setOutlineColor(sf::Color::Blue);

			line[0].position = points[0];
			if (drawStartPoints)
			{
				circle.setPosition(points[0]);
				window.draw(circle);
			}

			for (size_t i = 1; i < points.size(); i++)
			{
				const auto& point = points[i];

				if (drawStartPoints)
				{
					circle.setPosition(point);
					window.draw(circle);
				}
				if (showLines)
				{
					line[1].position = point;
					window.draw(line, 2, sf::Lines);
					line[0].position = line[1].position;
				}
			}
		}

		window.display();
	}
}