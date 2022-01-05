#include <SFML/Graphics.hpp>
#include <iostream>

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
	sf::RenderWindow window(sf::VideoMode(800, 600), "Bezier Curve Visualisation!");
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

	bool undoWasPressed = false;
	bool mouseWasPressed = false;
	sf::Vector2f* pointSelected;
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
				if (lerpAmt > 0)
					lerpAmt -= 0.02;
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
				if (lerpAmt <= 0.98)
					lerpAmt += 0.02;

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
			{
				points.clear();
				curve = calculateCurve(points);
			}

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
			line[0].color = sf::Color::Green;
			line[1].color = sf::Color::Green;

			circle.setOutlineColor(sf::Color::Green);

			auto newPoints = points;
			while (newPoints.size() > 1)
			{
				std::vector<sf::Vector2f> tempPoints;
				auto startingPoint = newPoints[0];
				line[0].position = newPoints[0];
				for (size_t i = 1; i < newPoints.size(); i++)
				{
					const auto& point = newPoints[i];
					sf::Vector2f newPoint(naive_lerp(startingPoint.x, point.x, lerpAmt), naive_lerp(startingPoint.y, point.y, lerpAmt));
					tempPoints.push_back(newPoint);
					startingPoint = point;
					line[1].position = point;
					window.draw(line, 2, sf::Lines);
					line[0].position = line[1].position;
				}
				newPoints = tempPoints;
			}
			circle.setPosition(newPoints[0]);
			window.draw(circle);

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
				line[1].position = point;
				window.draw(line, 2, sf::Lines);
				line[0].position = line[1].position;
				if (drawStartPoints)
				{
					circle.setPosition(point);
					window.draw(circle);
				}
			}
		}

		window.display();
	}
}