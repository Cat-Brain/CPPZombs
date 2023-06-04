#include "Vec.h"

struct KeyPress
{
	bool pressed = false, held = false, released = false;
};

struct Key : public KeyPress
{
	int keycode = -1;
	Key(int keycode = -1) :
		keycode(keycode) {}
};

struct Inputs
{
	Key up = Key(GLFW_KEY_W), left = Key(GLFW_KEY_A), down = Key(GLFW_KEY_S), right = Key(GLFW_KEY_D), jump = Key(GLFW_KEY_SPACE), // Movement keys
		primary = Key(GLFW_MOUSE_BUTTON_LEFT), secondary = Key(GLFW_MOUSE_BUTTON_RIGHT), utility = Key(GLFW_KEY_LEFT_SHIFT), // Ability keys
		crouch = Key(GLFW_KEY_Z), inventory = Key(GLFW_KEY_TAB), // Player interaction buttons
		enter = Key(GLFW_KEY_ENTER), hideUI = Key(GLFW_KEY_C), zoomIn = Key(GLFW_KEY_Q), zoomOut = Key(GLFW_KEY_E), pause = Key(GLFW_KEY_ESCAPE), // Technical buttons and dev buttons
		comma, period, slash, phase; // Command keys.
	int mouseScroll = 0;
	Vec2 mousePosition = vZero2, screenMousePosition = vZero2, mouseOffset = vZero2;
	Vec3 mousePosition3 = vZero; // Just mousePosition but with a 0 for the z value.
	int bindingButton = -1;


	Inputs() = default;

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
		UpdateKey(window, up);
		UpdateKey(window, left);
		UpdateKey(window, down);
		UpdateKey(window, right);

		UpdateKey(window, crouch);
		UpdateKey(window, inventory);

		UpdateKey(window, enter);
		UpdateKey(window, hideUI);
		UpdateKey(window, zoomIn);
		UpdateKey(window, zoomOut);
		UpdateKey(window, pause);
		UpdateKey(window, jump);
		UpdateKey(window, utility);

		UpdateKey(window, comma);
		UpdateKey(window, period);
		UpdateKey(window, slash);
		UpdateKey(window, phase);
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
		Vec2 oldMousePosition = mousePosition;
		mousePosition.x = static_cast<float>(xPos - zoom * screenRatio);
		mousePosition.y = static_cast<float>(yPos - zoom);
		mouseOffset = mousePosition - oldMousePosition;
		mousePosition3 = Vec3(mousePosition, 0);
		
		UpdateMouse(window, primary);
		UpdateMouse(window, secondary);
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