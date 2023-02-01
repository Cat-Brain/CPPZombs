#include "Vec2.h"

struct Key
{
	bool pressed = false, held = false, released = false;
};

struct Inputs
{
	Key w, a, s, d,
		enter, c, q, e, space,
		up, left, down, right,
		leftMouse, rightMouse, middleMouse;
	int mouseScroll;
	Vec2 mousePosition = vZero;

	Inputs() = default;

	static void ResetKey(Key& key)
	{
		key.pressed = false;
		key.held = false;
		key.released = false;
	}

	static void UpdateKey(GLFWwindow* window, Key& key, int keycode)
	{
		bool held = glfwGetKey(window, keycode) == GLFW_PRESS;
		key.pressed = !key.held && held;
		key.released = key.held && !held;
		key.held = held;
	}

	static void UpdateMouse(GLFWwindow* window, Key& key, int keycode)
	{
		bool held = glfwGetMouseButton(window, keycode) == GLFW_PRESS;
		key.pressed = !key.held && held;
		key.released = key.held && !held;
		key.held = held;
	}

	void Update(GLFWwindow* window)
	{
		UpdateKey(window, w, GLFW_KEY_W);
		UpdateKey(window, a, GLFW_KEY_A);
		UpdateKey(window, s, GLFW_KEY_S);
		UpdateKey(window, d, GLFW_KEY_D);

		UpdateKey(window, up, GLFW_KEY_UP);
		UpdateKey(window, left, GLFW_KEY_LEFT);
		UpdateKey(window, down, GLFW_KEY_DOWN);
		UpdateKey(window, right, GLFW_KEY_RIGHT);

		UpdateKey(window, enter, GLFW_KEY_ENTER);
		UpdateKey(window, c, GLFW_KEY_C);
		UpdateKey(window, q, GLFW_KEY_Q);
		UpdateKey(window, e, GLFW_KEY_E);
		UpdateKey(window, space, GLFW_KEY_SPACE);

		UpdateMouse(window, leftMouse, GLFW_MOUSE_BUTTON_LEFT);
		UpdateMouse(window, rightMouse, GLFW_MOUSE_BUTTON_RIGHT);
		UpdateMouse(window, middleMouse, GLFW_MOUSE_BUTTON_MIDDLE);
	}

	void FindMousePos(GLFWwindow* window)
	{
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		xPos /= trueScreenWidth;
		xPos *= ScrWidth(); // So broken with weird screen ratios.
		yPos = (trueScreenHeight - yPos) / trueScreenHeight;
		yPos *= ScrHeight();
		mousePosition.x = round(xPos - ScrWidth() / 2.0);
		mousePosition.y = round(yPos - ScrHeight() / 2.0);
	}
};