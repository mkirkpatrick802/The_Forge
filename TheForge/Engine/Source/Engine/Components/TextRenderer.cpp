#include "TextRenderer.h"

#include "imgui.h"
#include "Engine/GameEngine.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Rendering/Renderer.h"

Engine::TextRenderer::TextRenderer()
{
    
}

Engine::TextRenderer::~TextRenderer()
{
    GetRenderer().RemoveComponentFromRenderList(this);
}

void Engine::TextRenderer::OnActivation()
{
    InitRenderable(gameObject);
}

void Engine::TextRenderer::Render(const ShaderUniformData& data)
{
    if (_font)
        _font->RenderText(_text, _screenPos, 1, glm::vec3(1,1,1));
}

void Engine::TextRenderer::Deserialize(const json& data)
{
    IRenderable::Deserialize(data);
    
    if (data.contains(JsonKeywords::TEXT_RENDERER_TEXT))
        _text = data[JsonKeywords::TEXT_RENDERER_TEXT];

    if (data.contains(JsonKeywords::TEXT_RENDERER_SIZE))
        _fontSize = data[JsonKeywords::TEXT_RENDERER_SIZE];

    if (data.contains(JsonKeywords::TEXT_RENDERER_SCREEN_POS_X) && data.contains(JsonKeywords::TEXT_RENDERER_SCREEN_POS_Y))
    {
        _screenPos.x = data[JsonKeywords::TEXT_RENDERER_SCREEN_POS_X];
       _screenPos.y = data[JsonKeywords::TEXT_RENDERER_SCREEN_POS_Y];
    }

    _font = std::make_unique<Font>("Assets/Engine Assets/Fonts/Consolas.ttf", _fontSize);
}

nlohmann::json Engine::TextRenderer::Serialize()
{
    nlohmann::json data = IRenderable::Serialize();
    data[JsonKeywords::TEXT_RENDERER_TEXT] = _text;
    data[JsonKeywords::TEXT_RENDERER_SIZE]= _fontSize;
    data[JsonKeywords::TEXT_RENDERER_SCREEN_POS_X] = _screenPos.x;
    data[JsonKeywords::TEXT_RENDERER_SCREEN_POS_Y] = _screenPos.y;
    return data;
}

void Engine::TextRenderer::DrawDetails()
{
    ImGui::PushItemWidth(50);
    ImGui::Text("Screen Position:");
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10); // Small padding
    ImGui::InputFloat("##TEXT X", &_screenPos.x, 0, 0);
    ImGui::SameLine();
    ImGui::InputFloat("##TEXT Y", &_screenPos.y, 0, 0);
    ImGui::PopItemWidth();
    
    //std::string label = "Sorting Layer##" + std::to_string((uintptr_t)this);
    //ImGui::InputInt(label.c_str(), &sortingLayer);
    ImGui::Spacing();

    char buffer[256];  // Temporary buffer
    strncpy_s(buffer, _text.c_str(), sizeof(buffer));  // Copy string into buffer
    buffer[sizeof(buffer) - 1] = '\0'; // Ensure null-termination

    if (ImGui::InputText("Text", buffer, sizeof(buffer)))
        _text = buffer;  // Update _text if the user modified the input
    ImGui::Spacing();

    ImGui::InputInt("Font Size", &_fontSize);
    {
        if (_font)
            _font->SetFontSize(_fontSize);
    }
    ImGui::Spacing();
}

void Engine::TextRenderer::Write(NetCode::OutputByteStream& stream) const
{
    stream.Write(_screenPos);
    stream.Write(_text);
    stream.Write(_fontSize);
}

void Engine::TextRenderer::Read(NetCode::InputByteStream& stream)
{
    stream.Read(_screenPos);
    stream.Read(_text);
    stream.Read(_fontSize);

    if(!_font)
        _font = std::make_unique<Font>("Assets/Engine Assets/Fonts/Consolas.ttf", _fontSize);
}
