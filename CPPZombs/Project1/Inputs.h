#include "Vec.h"

struct Key
{
	bool pressed = false, held = false, released = false;
};

struct Inputs
{
	Key up, left, down, right, crouch, // Movement keys
		enter, c, q, e, escape, space, shift,
		leftMouse, rightMouse, middleMouse, // Mouse buttons
		comma, period, slash, phase; // Command keys.
	int mouseScroll = 0;
	Vec2 mousePosition = vZero, screenMousePosition = vZero;
	Vec3 mousePosition3 = vZero; // Just mousePosition but with a 0 for the z value.

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

	Vec3 MoveDir()
	{
		return Normalized(Vec3(float(right.held) - float(left.held), float(up.held) - float(down.held), 0));
	}

	void Update(GLFWwindow* window)
	{
		UpdateKey(window, up, GLFW_KEY_W);
		UpdateKey(window, left, GLFW_KEY_A);
		UpdateKey(window, down, GLFW_KEY_S);
		UpdateKey(window, right, GLFW_KEY_D);
		UpdateKey(window, crouch, GLFW_KEY_Z);

		UpdateKey(window, enter, GLFW_KEY_ENTER);
		UpdateKey(window, c, GLFW_KEY_C);
		UpdateKey(window, q, GLFW_KEY_Q);
		UpdateKey(window, e, GLFW_KEY_E);
		UpdateKey(window, escape, GLFW_KEY_ESCAPE);
		UpdateKey(window, space, GLFW_KEY_SPACE);
		UpdateKey(window, shift, GLFW_KEY_LEFT_SHIFT);

		UpdateKey(window, comma, GLFW_KEY_COMMA);
		UpdateKey(window, period, GLFW_KEY_PERIOD);
		UpdateKey(window, slash, GLFW_KEY_SLASH);
		UpdateKey(window, phase, GLFW_KEY_RIGHT_SHIFT);
	}

	void UpdateMouse(GLFWwindow* window, float zoom)
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
		mousePosition3 = Vec3(mousePosition, 0);
		
		UpdateMouse(window, leftMouse, GLFW_MOUSE_BUTTON_LEFT);
		UpdateMouse(window, rightMouse, GLFW_MOUSE_BUTTON_RIGHT);
		UpdateMouse(window, middleMouse, GLFW_MOUSE_BUTTON_MIDDLE);
	}
};

class Cursor
{
public:
	GLFWcursor* c;

	Cursor(GLFWimage* image)
	{
		c = glfwCreateCursor(image, -image->width / 2, -image->height / 2);
	}

	~Cursor()
	{
		glfwDestroyCursor(c);
	}
};