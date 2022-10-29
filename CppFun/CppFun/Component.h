#include "LowLevelClasses.h"

class Component
{
protected:
	void* object;
	const uint index = 0;
public:
	virtual void Awake() {}

	virtual void Start() {}

	virtual void Update(float deltaTime) {}

	virtual void LateUpdate(float deltaTime) {}

	virtual void RenderUpdate(float deltaTime) {}

	virtual void End() {}

	virtual void Print() { printf("COMPONENT HAS NO PRINT FUNCTION\n"); }

	virtual void Print2() { printf("COMPONENT HAS NO PRINT2 FUNCTION"); }

	Component(void* object):
		object(object)
	{}
};