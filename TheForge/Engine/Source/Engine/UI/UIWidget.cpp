#include "UIWidget.h"

#include "imgui.h"
#include "UIActionRegistry.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIRoot.h"
#include "UIText.h"
#include "Engine/LaunchOptions.h"
#include "Engine/System.h"

namespace
{
    // Serialized keys. Named rather than positional so a level authored before a
    // property existed still loads -- every read below is guarded by contains().
    const std::string KEY_TYPE = "UI Type";
    const std::string KEY_ANCHOR = "UI Anchor";
    const std::string KEY_OFFSET_X = "UI Offset X";
    const std::string KEY_OFFSET_Y = "UI Offset Y";
    const std::string KEY_SIZE_X = "UI Size X";
    const std::string KEY_SIZE_Y = "UI Size Y";
    const std::string KEY_PIVOT_X = "UI Pivot X";
    const std::string KEY_PIVOT_Y = "UI Pivot Y";
    const std::string KEY_COLOR_R = "UI Color R";
    const std::string KEY_COLOR_G = "UI Color G";
    const std::string KEY_COLOR_B = "UI Color B";
    const std::string KEY_OPACITY = "UI Opacity";
    const std::string KEY_SPRITE = "UI Sprite";
    const std::string KEY_TEXT = "UI Text";
    const std::string KEY_FONT_SIZE = "UI Font Size";
    const std::string KEY_ALIGN = "UI Align";
    const std::string KEY_ACTION = "UI Action";
}

const char* Engine::UIWidgetTypeName(const EUIWidgetType type)
{
    switch (type)
    {
    case EUIWidgetType::Image:  return "Image";
    case EUIWidgetType::Text:   return "Text";
    case EUIWidgetType::Button: return "Button";
    default:                    return "?";
    }
}

Engine::UIWidget::UIWidget()
{
    _rect.size = glm::vec2(64.0f, 16.0f);
}

Engine::UIWidget::~UIWidget()
{
    // Belt and braces alongside OnDeactivation. The canvas holds a borrowed pointer to
    // an element this component owns, so it must not outlive it by any path.
    Unregister();
}

void Engine::UIWidget::OnActivation()
{
    RebuildElement();
    Register();
}

void Engine::UIWidget::OnDeactivation()
{
    Unregister();
}

void Engine::UIWidget::Register()
{
    if (_registered || _element == nullptr) return;

    // A dedicated server has no canvas to draw on and no player to draw for. The element
    // still exists as data -- it has to, because the level is the same file either way --
    // it simply is not shown.
    if (GetLaunchOptions().headless) return;

    UIRoot::GetWorldCanvas().AddBorrowed(_element.get());
    _registered = true;
}

void Engine::UIWidget::Unregister()
{
    if (!_registered) return;

    UIRoot::GetWorldCanvas().RemoveBorrowed(_element.get());
    _registered = false;
}

void Engine::UIWidget::RebuildElement()
{
    // The canvas is holding a pointer to the old element, so it has to be taken back
    // before the element is replaced.
    const bool wasRegistered = _registered;
    Unregister();

    switch (_type)
    {
    case EUIWidgetType::Text:   _element = std::make_unique<UIText>(); break;
    case EUIWidgetType::Button: _element = std::make_unique<UIButton>(); break;
    default:                    _element = std::make_unique<UIImage>(); break;
    }

    ApplyProperties();

    if (wasRegistered || _element != nullptr)
        Register();
}

void Engine::UIWidget::ApplyProperties() const
{
    if (_element == nullptr) return;

    _element->Rect() = _rect;
    _element->SetTint(_color);
    _element->SetOpacity(_opacity);

    if (const auto text = dynamic_cast<UIText*>(_element.get()))
    {
        text->SetText(_text);
        text->SetFontSize(_fontSize);
        text->SetAlign(static_cast<EUITextAlign>(_align));
    }

    if (const auto image = dynamic_cast<UIImage*>(_element.get()))
        image->SetSprite(_spritePath);

    if (const auto button = dynamic_cast<UIButton*>(_element.get()))
    {
        button->SetLabel(_text);
        button->Label().SetFontSize(_fontSize);

        // Captured by value, not by `this`. A component lives in a pool and can be moved
        // as the pool grows; the action is a name and copying it is cheap.
        const std::string action = _action;
        button->SetOnClick([action]
        {
            if (action.empty())
            {
                DEBUG_LOG("UI: a button was clicked but has no action set.")
                return;
            }

            UIActionRegistry::Invoke(action);
        });
    }
}

void Engine::UIWidget::Deserialize(const json& data)
{
    if (data.contains(KEY_TYPE)) _type = static_cast<EUIWidgetType>(data[KEY_TYPE].get<int>());
    if (data.contains(KEY_ANCHOR)) _rect.anchor = static_cast<EUIAnchor>(data[KEY_ANCHOR].get<int>());
    if (data.contains(KEY_OFFSET_X)) _rect.offset.x = data[KEY_OFFSET_X];
    if (data.contains(KEY_OFFSET_Y)) _rect.offset.y = data[KEY_OFFSET_Y];
    if (data.contains(KEY_SIZE_X)) _rect.size.x = data[KEY_SIZE_X];
    if (data.contains(KEY_SIZE_Y)) _rect.size.y = data[KEY_SIZE_Y];
    if (data.contains(KEY_PIVOT_X)) _rect.pivot.x = data[KEY_PIVOT_X];
    if (data.contains(KEY_PIVOT_Y)) _rect.pivot.y = data[KEY_PIVOT_Y];
    if (data.contains(KEY_COLOR_R)) _color.r = data[KEY_COLOR_R];
    if (data.contains(KEY_COLOR_G)) _color.g = data[KEY_COLOR_G];
    if (data.contains(KEY_COLOR_B)) _color.b = data[KEY_COLOR_B];
    if (data.contains(KEY_OPACITY)) _opacity = data[KEY_OPACITY];
    if (data.contains(KEY_SPRITE)) _spritePath = data[KEY_SPRITE];
    if (data.contains(KEY_TEXT)) _text = data[KEY_TEXT];
    if (data.contains(KEY_FONT_SIZE)) _fontSize = data[KEY_FONT_SIZE];
    if (data.contains(KEY_ALIGN)) _align = data[KEY_ALIGN];
    if (data.contains(KEY_ACTION)) _action = data[KEY_ACTION];

    // No GL work here, and none reachable from here. Setting a sprite path only records
    // it; the texture and any font atlas are built on the first draw, which is what lets
    // a headless server load this level at all.
    if (_element != nullptr)
        ApplyProperties();
}

nlohmann::json Engine::UIWidget::Serialize()
{
    nlohmann::json data;
    data[KEY_TYPE] = static_cast<int>(_type);
    data[KEY_ANCHOR] = static_cast<int>(_rect.anchor);
    data[KEY_OFFSET_X] = _rect.offset.x;
    data[KEY_OFFSET_Y] = _rect.offset.y;
    data[KEY_SIZE_X] = _rect.size.x;
    data[KEY_SIZE_Y] = _rect.size.y;
    data[KEY_PIVOT_X] = _rect.pivot.x;
    data[KEY_PIVOT_Y] = _rect.pivot.y;
    data[KEY_COLOR_R] = _color.r;
    data[KEY_COLOR_G] = _color.g;
    data[KEY_COLOR_B] = _color.b;
    data[KEY_OPACITY] = _opacity;
    data[KEY_SPRITE] = _spritePath;
    data[KEY_TEXT] = _text;
    data[KEY_FONT_SIZE] = _fontSize;
    data[KEY_ALIGN] = _align;
    data[KEY_ACTION] = _action;

    return data;
}

void Engine::UIWidget::Write(NetCode::OutputByteStream& stream) const
{
    stream.Write(static_cast<uint8_t>(_type));
    stream.Write(_rect.offset);
    stream.Write(_rect.size);
    stream.Write(_rect.pivot);
    stream.Write(static_cast<uint8_t>(_rect.anchor));
    // Three floats, not the vec3: ByteStream supports vec2 and the primitives, and its
    // generic Write static_asserts on anything else rather than silently writing raw
    // bytes.
    stream.Write(_color.r);
    stream.Write(_color.g);
    stream.Write(_color.b);
    stream.Write(_opacity);
    stream.Write(_spritePath);
    stream.Write(_text);
    stream.Write(static_cast<uint32_t>(_fontSize));
    stream.Write(_align);
    stream.Write(_action);
}

void Engine::UIWidget::Read(NetCode::InputByteStream& stream)
{
    uint8_t type = 0, anchor = 0;
    stream.Read(type);
    stream.Read(_rect.offset);
    stream.Read(_rect.size);
    stream.Read(_rect.pivot);
    stream.Read(anchor);
    stream.Read(_color.r);
    stream.Read(_color.g);
    stream.Read(_color.b);
    stream.Read(_opacity);
    stream.Read(_spritePath);
    stream.Read(_text);
    uint32_t fontSize = 0;
    stream.Read(fontSize);
    _fontSize = fontSize;
    stream.Read(_align);
    stream.Read(_action);

    _rect.anchor = static_cast<EUIAnchor>(anchor);

    // The type decides the element's class, so a type that arrived over the wire needs
    // the element built again rather than merely reconfigured.
    if (static_cast<EUIWidgetType>(type) != _type || _element == nullptr)
    {
        _type = static_cast<EUIWidgetType>(type);
        RebuildElement();
    }
    else
    {
        ApplyProperties();
    }
}

void Engine::UIWidget::DrawDetails()
{
    bool rebuild = false;

    int type = static_cast<int>(_type);
    const char* types[] = {"Image", "Text", "Button"};
    if (ImGui::Combo("Widget Type", &type, types, IM_ARRAYSIZE(types)))
    {
        _type = static_cast<EUIWidgetType>(type);
        rebuild = true;
    }

    int anchor = static_cast<int>(_rect.anchor);
    const char* anchors[] = {
        "Top Left", "Top Center", "Top Right",
        "Middle Left", "Middle Center", "Middle Right",
        "Bottom Left", "Bottom Center", "Bottom Right",
    };
    if (ImGui::Combo("Anchor", &anchor, anchors, IM_ARRAYSIZE(anchors)))
        _rect.anchor = static_cast<EUIAnchor>(anchor);

    // Units are the 320x180 reference canvas, not pixels -- say so, because entering
    // 1920 here and seeing nothing is otherwise a baffling first experience.
    ImGui::TextDisabled("positions are in 320x180 canvas units");

    ImGui::DragFloat2("Offset", &_rect.offset.x, 0.5f);
    ImGui::DragFloat2("Size", &_rect.size.x, 0.5f);
    ImGui::DragFloat2("Pivot", &_rect.pivot.x, 0.01f, 0.0f, 1.0f);
    ImGui::ColorEdit3("Color", &_color.r);
    ImGui::SliderFloat("Opacity", &_opacity, 0.0f, 1.0f);

    if (_type == EUIWidgetType::Image || _type == EUIWidgetType::Button)
    {
        char sprite[260];
        strncpy_s(sprite, _spritePath.c_str(), sizeof(sprite));
        sprite[sizeof(sprite) - 1] = '\0';
        if (ImGui::InputText("Sprite", sprite, sizeof(sprite)))
            _spritePath = sprite;
    }

    if (_type == EUIWidgetType::Text || _type == EUIWidgetType::Button)
    {
        char text[256];
        strncpy_s(text, _text.c_str(), sizeof(text));
        text[sizeof(text) - 1] = '\0';
        if (ImGui::InputText("Text", text, sizeof(text)))
            _text = text;

        int fontSize = static_cast<int>(_fontSize);
        if (ImGui::InputInt("Font Size", &fontSize))
            _fontSize = static_cast<unsigned int>(fontSize < 1 ? 1 : fontSize);

        const char* aligns[] = {"Left", "Center", "Right"};
        ImGui::Combo("Align", &_align, aligns, IM_ARRAYSIZE(aligns));
    }

    if (_type == EUIWidgetType::Button)
    {
        // Chosen from what is actually registered rather than typed free-hand, so a
        // button cannot be authored with an action name that will never resolve.
        const std::vector<std::string> names = UIActionRegistry::GetNames();

        std::string preview = _action.empty() ? "(none)" : _action;
        if (!_action.empty() && !UIActionRegistry::Has(_action))
            preview = _action + " (not registered)";

        if (ImGui::BeginCombo("Action", preview.c_str()))
        {
            if (ImGui::Selectable("(none)", _action.empty()))
                _action.clear();

            for (const std::string& name : names)
            {
                if (ImGui::Selectable(name.c_str(), name == _action))
                    _action = name;
            }

            ImGui::EndCombo();
        }
    }

    if (rebuild)
        RebuildElement();
    else
        ApplyProperties();
}
