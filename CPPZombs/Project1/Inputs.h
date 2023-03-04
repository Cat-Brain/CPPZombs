#include "Vec2.h"

struct Key
{
	bool pressed = false, held = false, released = false;
};

struct Inputs
{
	Key w, a, s, d,
		enter, c, q, e, escape, space,
		up, left, down, right,
		leftMouse, rightMouse, middleMouse,
		comma, period, slash; // <- Command keys.
	int mouseScroll = 0;
	Vec2 mousePosition = vZero, screenMousePosition = vZero;

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

	static void UpdateKey2(GLFWwindow* window, Key& key, int keycode)
	{
		bool held = glfwGetKey(window, keycode) == GLFW_PRESS;
		key.pressed = !key.held && held;
		key.released = key.held && !held;
		key.held |= held;
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
		UpdateKey(window, escape, GLFW_KEY_ESCAPE);
		UpdateKey(window, space, GLFW_KEY_SPACE);

		UpdateKey(window, comma, GLFW_KEY_COMMA);
		UpdateKey(window, period, GLFW_KEY_PERIOD);
		UpdateKey(window, slash, GLFW_KEY_SLASH);
		
		UpdateMouse(window, leftMouse, GLFW_MOUSE_BUTTON_LEFT);
		UpdateMouse(window, rightMouse, GLFW_MOUSE_BUTTON_RIGHT);
		UpdateMouse(window, middleMouse, GLFW_MOUSE_BUTTON_MIDDLE);
	}

	void FindMousePos(GLFWwindow* window, float zoom)
	{
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		screenMousePosition = { static_cast<int>(xPos), static_cast<int>(trueScreenHeight) - static_cast<int>(yPos) };
		xPos /= trueScreenHeight;
		xPos *= zoom * 2;
		yPos = (trueScreenHeight - yPos) / trueScreenHeight;
		yPos *= zoom * 2;
		mousePosition.x = static_cast<float>(xPos - zoom * screenRatio);
		mousePosition.y = static_cast<float>(yPos - zoom);
	}
};