#include "Component.h"

class Object
{
protected:
public:
	List<Component*> components;

	void Start()
	{
		for (uint i = 0; i < components.count; i++)
			components.ptr[i]->Start();
	}

	void Update(float deltaTime)
	{
		for (uint i = 0; i < components.count; i++)
			components.ptr[i]->Update(deltaTime);
	}

	void RenderUpdate(float deltaTime)
	{
		for (uint i = 0; i < components.count; i++)
			components.ptr[i]->RenderUpdate(deltaTime);
	}

	void End()
	{
		for (uint i = 0; i < components.count; i++)
		{
			components.ptr[i]->End();
			delete components.ptr[i];
		}

		components.End();
	}

	template<typename T>
	void AddComponent()
	{
		components.Add(T(this));
	}



	Object() :
		components(List<Component*>())
	{}

	Object(List<Component*> components) :
		components(components)
	{}
};