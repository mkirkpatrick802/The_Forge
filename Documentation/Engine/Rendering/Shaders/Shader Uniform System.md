---
tags:
  - Rendering
  - Shaders
---
## ShaderUniformProvider Interface

### Overview
The `IShaderUniformProvider` interface allows components to provide shader uniforms dynamically. This enables game components to influence rendering without tightly coupling game logic to the renderer.
### Interface Definition
```
class IShaderUniformProvider  
{
public:  
    virtual ~IShaderUniformProvider() = default;  
    virtual void CollectUniforms(ShaderUniformData& data) = 0;  
};
```

### Usage
Any component that wants to provide shader uniforms must inherit from `IShaderUniformProvider` and implement `CollectUniforms`.
#### Example: Destructible Component
```
class DestructibleComponent : public Component, public IShaderUniformProvider {
public:
    void CollectUniforms(ShaderUniformData& data) override {
        data.floatUniforms["health"] = GetHealthPercent();
    }
};
```

---
## ShaderUniformSystem

### Overview
The `ShaderUniformSystem` gathers shader uniform data from all components within a game object that implement `IShaderUniformProvider` and passes it to the rendering system.
### System Definition
```
class ShaderUniformSystem {
public:
    static ShaderUniformData CollectUniforms(GameObject* gameObject)
    {  
	    ShaderUniformData data;  
	    for (auto* component : go->GetAllComponents())  
		    if (const auto provider = dynamic_cast<IShaderUniformProvider
															        (component)) 
	            provider->CollectUniforms(data);  
  
	    return data;  
	}
};
```

### Usage
The `CollectUniforms` function should be called before rendering to gather uniform data from all relevant components.
#### Example: Using in Rendering
```
if (!_renderList.empty())  
    for (const auto& val : _renderList | std::views::values)  
    {       
	    auto data = ShaderUniformSystem::CollectUniforms(val->gameObject);  
	    val->Render(data);  
    }
```

### Benefits
- **Decouples Rendering and Game Logic**: The rendering system doesn’t need to know about individual components.
- **Extensibility**: Any component can provide shader uniforms without modifying the rendering system.
- **Efficiency**: Uniform data is gathered once per object per frame, minimizing overhead.

---
## ShaderUniformData Structure

### Overview
`ShaderUniformData` is a container for uniform values that need to be passed to shaders.
### Structure Definition
```
struct ShaderUniformData  
{  
    std::unordered_map<std::string, float> floatUniforms;  
    std::unordered_map<std::string, int> intUniforms;  
    std::unordered_map<std::string, glm::vec2> vec2Uniforms;  
    std::unordered_map<std::string, glm::vec3> vec3Uniforms;  
    std::unordered_map<std::string, glm::vec4> vec4Uniforms;  
    std::unordered_map<std::string, glm::mat4> mat4Uniforms;  
};
```

### Example Usage
```
ShaderUniformData data;
data.floatUniforms["u_Health"] = 0.75f;
data.vec3Uniforms["u_LightColor"] = glm::vec3(1.0f, 0.8f, 0.6f);
```

---
## Summary

- `IShaderUniformProvider` is an interface for components that provide shader uniforms.
- `ShaderUniformSystem` gathers uniforms and sends them to shaders.
- `ShaderUniformData` stores the uniform values in a structured way.
- This system keeps rendering flexible, modular, and efficient.