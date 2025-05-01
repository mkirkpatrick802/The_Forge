#### 1. Code Reflection

**This would allow for smaller component files and streamline the process of serialization for both game saving and packet construction.**
```` cpp
public class Foo : public Component
{
	REFLECT() // Macro to declare that the class should be reflected

public:

	float temp; // This would be reflected but not replicated

	REPLICATE()
	float count; // This would be reflected and replicated
}

REGISTER_COMPONENT(Foo)
````

#### 2. More Dynamic Component Pools