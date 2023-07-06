#include "Vec.h"

struct KeyPress
{
	bool pressed = false, held = false, released = false;

	struct KeyPress(bool pressed = false, bool held = false, bool released = false) :
		pressed(pressed), held(held), released(released) {}

	void Reset()
	{
		pressed = false,
			held = false,
			released = false;
	}
};

struct Key : public KeyPress
{
	int keycode = -1;
	Key(int keycode = -1) :
		keycode(keycode) {}

	Key& operator= (const KeyPress& press)
	{
		pressed = press.pressed;
		held = press.held;
		released = press.released;
		return *this;
	}
};

namespace KeyCode
{
	enum
	{
		UP, LEFT, DOWN, RIGHT,
		JUMP, PRIMARY, SECONDARY, UTILITY, 
		BUILD, CROUCH, INVENTORY,
		ENTER, HIDEUI, ROW_LEFT, ROW_RIGHT, PAUSE,
		COMMA, PERIOD, SLASH, PHASE
	};
}

class Inputs
{
public:
	vector<Key> keys = { Key(GLFW_KEY_W), Key(GLFW_KEY_A), Key(GLFW_KEY_S), Key(GLFW_KEY_D), Key(GLFW_KEY_SPACE), // Movement keys
		Key(GLFW_MOUSE_BUTTON_LEFT), Key(GLFW_MOUSE_BUTTON_RIGHT), Key(GLFW_KEY_LEFT_SHIFT), // Ability keys
		Key(GLFW_KEY_F), Key(GLFW_KEY_Z), Key(GLFW_KEY_TAB), // Player interaction buttons
		Key(GLFW_KEY_ENTER), Key(GLFW_KEY_C), Key(GLFW_KEY_Q), Key(GLFW_KEY_E), Key(GLFW_KEY_ESCAPE), // Technical buttons and dev buttons
		Key(GLFW_KEY_COMMA), Key(GLFW_KEY_PERIOD), Key(GLFW_KEY_SLASH), Key(GLFW_KEY_RIGHT_SHIFT) };
	int mouseScroll = 0;
	float mouseScrollF = 0;
	
	Vec2 mousePosition = vZero2, screenMousePosition = vZero2, mouseOffset = vZero2;
	Vec3 mousePosition3 = vZero; // Just mousePosition but with a 0 for the z value.
	int bindingButton = -1;


	Inputs() = default;

	Inputs& operator= (const Inputs& inputs)
	{
		for (int i = 0; i < keys.size(); i++)
			keys[i] = inputs.keys[i];
		return *this;
	}

	void SetKey(int keycode)
	{

	}

	static void ResetKey(Key& key)
	{
		key.pressed = false;
		key.held = false;
		key.released = false;
	}

	static void UpdateKey(GLFWwindow* window, Key& key)
	{
		bool held = glfwGetKey(window, key.keycode) == GLFW_PRESS;
		key.pressed = !key.held && held;
		key.released = key.held && !held;
		key.held = held;
	}

	static void UpdateKey2(GLFWwindow* window, Key& key)
	{
		bool held = glfwGetKey(window, key.keycode) == GLFW_PRESS;
		key.pressed = !key.held && held;
		key.released = key.held && !held;
		key.held |= held;
	}

	static void UpdateMouse(GLFWwindow* window, Key& key)
	{
		bool held = glfwGetMouseButton(window, key.keycode) == GLFW_PRESS;
		key.pressed = !key.held && held;
		key.released = key.held && !held;
		key.held = held;
	}

	Vec3 MoveDir();

	void Update(GLFWwindow* window)
	{
		UpdateKey(window, keys[KeyCode::UP]);
		UpdateKey(window, keys[KeyCode::LEFT]);
		UpdateKey(window, keys[KeyCode::DOWN]);
		UpdateKey(window, keys[KeyCode::RIGHT]);
		UpdateKey(window, keys[KeyCode::JUMP]);

		// Primary and secondary are bound to mouse buttons and must be handled seperately.
		UpdateKey(window, keys[KeyCode::UTILITY]);

		UpdateKey(window, keys[KeyCode::BUILD]);
		UpdateKey(window, keys[KeyCode::CROUCH]);
		UpdateKey(window, keys[KeyCode::INVENTORY]);

		UpdateKey(window, keys[KeyCode::ENTER]);
		UpdateKey(window, keys[KeyCode::HIDEUI]);
		UpdateKey(window, keys[KeyCode::ROW_LEFT]);
		UpdateKey(window, keys[KeyCode::ROW_RIGHT]);
		UpdateKey(window, keys[KeyCode::PAUSE]);

		UpdateKey(window, keys[KeyCode::COMMA]);
		UpdateKey(window, keys[KeyCode::PERIOD]);
		UpdateKey(window, keys[KeyCode::SLASH]);
		UpdateKey(window, keys[KeyCode::PHASE]);
	}

	void UpdateMouse(GLFWwindow* window, float sensitivity)
	{
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		screenMousePosition = { static_cast<int>(xPos), static_cast<int>(trueScreenHeight) - static_cast<int>(yPos) };
		xPos /= trueScreenHeight;
		yPos = (trueScreenHeight - yPos) / trueScreenHeight;
		Vec2 oldMousePosition = mousePosition;
		mousePosition.x = static_cast<float>(xPos * sensitivity);
		mousePosition.y = static_cast<float>(yPos * sensitivity);
		mouseOffset = mousePosition - oldMousePosition;
		mousePosition3 = Vec3(mousePosition, 0);
		
		UpdateMouse(window, keys[KeyCode::PRIMARY]);
		UpdateMouse(window, keys[KeyCode::SECONDARY]);
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