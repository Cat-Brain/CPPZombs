#include "Object.h"

class ObjectWorld
{
public:
	List<Object*> objects;

	void Start()
	{
		for (uint i = 0; i < objects.count; i++)
			objects.ptr[i]->Start();
	}

	void Update(float deltaTime)
	{
		for (uint i = 0; i < objects.count; i++)
			objects.ptr[i]->Update(deltaTime);
	}

	void RenderUpdate(float deltaTime)
	{
		for (uint i = 0; i < objects.count; i++)
			objects.ptr[i]->RenderUpdate(deltaTime);
	}

	void TempEnd()
	{
		for (uint i = 0; i < objects.count; i++)
			objects.ptr[i]->End();
	}

	void End()
	{
		for (uint i = 0; i < objects.count; i++)
		{
			objects.ptr[i]->End();
			delete objects.ptr[i];
		}

		objects.End();
	}

	void AddObject(Object* object)
	{
		objects.Add(object);
	}

	void EvaporateComponent(uint index)
	{
		objects.ptr[index]->End();
		delete(objects.ptr[index]);
		objects.RemoveAt(index);
	}

	ObjectWorld() :
		objects(List<Object*>())
	{ }
};