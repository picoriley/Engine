#include "Engine/UI/WidgetBase.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Core/Events/EventSystem.hpp"
#include "UISystem.hpp"

//-----------------------------------------------------------------------------------
WidgetBase::WidgetBase()
    : m_name("Unnamed Widget")
{
    m_propertiesForAllStates.Set<std::string>("Name", m_name);
    m_propertiesForAllStates.Set<Vector2>("Offset", Vector2::ZERO);
    m_propertiesForAllStates.Set<Vector2>("Size", Vector2::ONE);
    m_propertiesForAllStates.Set<Vector2>("Padding", Vector2::ZERO);
    m_propertiesForAllStates.Set<Vector2>("Margin", Vector2::ZERO);
    m_propertiesForAllStates.Set<RGBA>("BackgroundColor", RGBA::LIGHT_GRAY);
    m_propertiesForAllStates.Set<RGBA>("BorderColor", RGBA::GRAY);
    m_propertiesForAllStates.Set<float>("Scale", 1.0f);
    m_propertiesForAllStates.Set<float>("Opacity", 1.0f);
    m_propertiesForAllStates.Set<float>("BorderWidth", 0.0f);
    SetProperty("BorderColor", RGBA::BLACK, DISABLED_WIDGET_STATE);
    SetProperty("TextColor", RGBA::GRAY, DISABLED_WIDGET_STATE);
    SetProperty("BackgroundColor", RGBA::DARK_GRAY, DISABLED_WIDGET_STATE);
}

//-----------------------------------------------------------------------------------
WidgetBase::~WidgetBase()
{
    for (WidgetBase* child : m_children)
    {
        delete child;
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::Update(float deltaSeconds)
{
    if (IsHidden())
    {
        return;
    }
    UpdateChildren(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void WidgetBase::UpdateChildren(float deltaSeconds)
{
    m_timeInState += deltaSeconds;
    for (WidgetBase* child : m_children)
    {
        child->Update(deltaSeconds);
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::Render() const
{
    if (IsHidden())
    {
        return;
    }
    float borderWidth = GetProperty<float>("BorderWidth");
    RGBA bgColor = GetProperty<RGBA>("BackgroundColor");
    RGBA borderColor = GetProperty<RGBA>("BorderColor");
    float opacity = GetProperty<float>("Opacity");

    opacity *= GetParentOpacities();
    bgColor.alpha = (uchar)((((float)bgColor.alpha / 255.0f) * opacity) * 255.0f);
    borderColor.alpha = (uchar)((((float)borderColor.alpha / 255.0f) * opacity) * 255.0f);

    Texture* texture = Renderer::instance->m_defaultTexture;

    if (m_texture)
    {
        texture = m_texture;
    }

    if (m_material)
    {
        //TODO: Once we redo how drawing works, add support for materials!
    }

    float targetScale = GetProperty<float>("Scale");
    m_currentScale = Lerp<float>(Clamp01(m_timeInState * 8.0f), m_currentScale, targetScale);

    if (borderWidth > 0.0f && borderColor.alpha != 0x00)
    {
        AABB2 finalBounds = m_borderedBounds * m_currentScale;
        finalBounds = finalBounds + m_position;
        Renderer::instance->DrawTexturedAABB(finalBounds, Vector2(0, 1), Vector2(1, 0), Renderer::instance->m_defaultTexture, borderColor);
    }
    if (bgColor.alpha != 0x00)
    {
        AABB2 finalBounds = m_borderlessBounds * m_currentScale;
        finalBounds = finalBounds + m_position;
        Renderer::instance->DrawTexturedAABB(finalBounds, Vector2(0, 1), Vector2(1, 0), texture, bgColor);
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::RenderChildren() const
{
    for (WidgetBase* child : m_children)
    {
        child->Render();
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::AddChild(WidgetBase* child)
{
    m_children.push_back(child);
    child->m_parent = this;
    child->RecalculateBounds();
    RecalculateBounds();
}

//-----------------------------------------------------------------------------------
AABB2 WidgetBase::GetSmallestBoundsAroundChildren()
{
    AABB2 smallestBounds = m_children.size() > 0 ? m_children.at(0)->m_bounds : AABB2(Vector2::ZERO, Vector2::ZERO);

    for (WidgetBase* child : m_children)
    {
        AABB2 childBounds = child->GetBounds();
        smallestBounds.mins.x = (childBounds.mins.x < smallestBounds.mins.x) ? childBounds.mins.x : smallestBounds.mins.x;
        smallestBounds.mins.y = (childBounds.mins.y < smallestBounds.mins.y) ? childBounds.mins.y : smallestBounds.mins.y;
        smallestBounds.maxs.x = (childBounds.maxs.x > smallestBounds.maxs.x) ? childBounds.maxs.x : smallestBounds.maxs.x;
        smallestBounds.maxs.y = (childBounds.maxs.y > smallestBounds.maxs.y) ? childBounds.maxs.y : smallestBounds.maxs.y;
    }

    return smallestBounds;
}

//-----------------------------------------------------------------------------------
//Fun fact: You CAN implement a pure virtual function. This is here to keep the base functionality 
//consistent for recalculating bounds if a subclass doesn't need to override it.
void WidgetBase::RecalculateBounds()
{
    ApplyOffsetProperty();
    ApplySizeProperty();
    ApplyDockingProperty();
    ApplyPaddingProperty();
    m_borderlessBounds = m_bounds;

    ApplyBorderProperty();
    m_borderedBounds = m_bounds;

    ApplyMarginProperty();
}

//-----------------------------------------------------------------------------------
void WidgetBase::ApplyBorderProperty()
{
    float borderWidth = GetProperty<float>("BorderWidth");
    m_bounds.mins += Vector2(-borderWidth);
    m_bounds.maxs += Vector2(borderWidth);
}

//-----------------------------------------------------------------------------------
void WidgetBase::ApplySizeProperty()
{
    //If our innards aren't big enough to meet the minimum size requirement, stretch to fit that.
    Vector2 boundsSize = Vector2(m_bounds.GetWidth(), m_bounds.GetHeight());
    Vector2 minimumSize = m_dimensions.GetMinSize();

    Vector2 sizeDifference = minimumSize - boundsSize;
    float halfSizeDifferenceX = sizeDifference.x * 0.5f;
    m_bounds.mins.x -= halfSizeDifferenceX;
    m_bounds.maxs.x += halfSizeDifferenceX;
    float halfSizeDifferenceY = sizeDifference.y * 0.5f;
    m_bounds.mins.y -= halfSizeDifferenceY;
    m_bounds.maxs.y += halfSizeDifferenceY;
}

//-----------------------------------------------------------------------------------
void WidgetBase::ApplyPaddingProperty()
{
    m_bounds.mins -= m_propertiesForAllStates.Get<Vector2>("Padding");
    m_bounds.maxs += m_propertiesForAllStates.Get<Vector2>("Padding");
}

//-----------------------------------------------------------------------------------
void WidgetBase::ApplyMarginProperty()
{
    //This modifies the bounds after the border has been set.
    m_bounds.mins -= m_propertiesForAllStates.Get<Vector2>("Margin");
    m_bounds.maxs += m_propertiesForAllStates.Get<Vector2>("Margin");
}

//-----------------------------------------------------------------------------------
void WidgetBase::ApplyDockingProperty()
{
    switch (m_dockType)
    {
    case BOTTOM_DOCKED:
        m_bounds.mins.x = 0;
        m_bounds.maxs.x = 1600;
        m_bounds.mins.y = 0;
        break;
    case TOP_DOCKED:
        m_bounds.mins.x = 0;
        m_bounds.maxs.x = 1600;
        m_bounds.maxs.y = 900;
        m_bounds.mins.y += 900;
        break;
    case LEFT_DOCKED:
        m_bounds.mins.x = 0;
        m_bounds.mins.y = 0;
        m_bounds.maxs.y = 900;
        break;
    case RIGHT_DOCKED:
        m_bounds.mins.x += 1600;
        m_bounds.maxs.x = 1600;
        m_bounds.mins.y = 0;
        m_bounds.maxs.y = 900;
        break;
    case FILL_DOCKED:
        m_bounds.mins.x = 0;
        m_bounds.maxs.x = 1600;
        m_bounds.mins.y = 0;
        m_bounds.maxs.y = 900;
        break;
    case NOT_DOCKED:
    case NUM_DOCKING_POSITIONS:
    default:
        break;
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::BuildFromXMLNode(XMLNode& node)
{
    const char* nameAttribute = node.getAttribute("Name");
    const char* horizontalOffset = node.getAttribute("HorizontalOffset");
    const char* verticalOffset = node.getAttribute("VerticalOffset");
    const char* backgroundColorAttribute = node.getAttribute("BackgroundColor");
    const char* borderColorAttribute = node.getAttribute("BorderColor");
    const char* borderWidthAttribute = node.getAttribute("BorderWidth");
    const char* onClickAttribute = node.getAttribute("OnClick");
    const char* opacityAttribute = node.getAttribute("Opacity");
    const char* sizeAttribute = node.getAttribute("Size");
    const char* offsetAttribute = node.getAttribute("Offset");
    const char* paddingAttribute = node.getAttribute("Padding");
    const char* marginAttribute = node.getAttribute("Margin");
    const char* textureAttribute = node.getAttribute("Texture");
    const char* materialAttribute = node.getAttribute("Material");
    const char* dockAttribute = node.getAttribute("Dock");

    Vector2 offset = m_propertiesForAllStates.Get<Vector2>("Offset");
    RGBA bgColor = m_propertiesForAllStates.Get<RGBA>("BackgroundColor");
    RGBA edgeColor = m_propertiesForAllStates.Get<RGBA>("BorderColor");

    if (paddingAttribute)
    {
        Vector2 padding = Vector2::CreateFromString(paddingAttribute);
        m_propertiesForAllStates.Set<Vector2>("Padding", padding);
    }
    if (marginAttribute)
    {
        Vector2 margin = Vector2::CreateFromString(marginAttribute);
        m_propertiesForAllStates.Set<Vector2>("Margin", margin);
    }
    if (offsetAttribute)
    {
        offset = Vector2::CreateFromString(offsetAttribute);
    }
    if (horizontalOffset)
    {
        offset.x = GetFloatFromString(horizontalOffset);
    }
    if (verticalOffset)
    {
        offset.y = GetFloatFromString(verticalOffset);
    }
    if (backgroundColorAttribute)
    {
        bgColor = RGBA::CreateFromString(backgroundColorAttribute);
        m_propertiesForAllStates.Set<RGBA>("BackgroundColor", bgColor);
    }
    if (borderColorAttribute)
    {
        edgeColor = RGBA::CreateFromString(borderColorAttribute);
        m_propertiesForAllStates.Set<RGBA>("BorderColor", edgeColor);
    }
    if (borderWidthAttribute)
    {
        m_propertiesForAllStates.Set<float>("BorderWidth", GetFloatFromString(borderWidthAttribute));
    }
    if (onClickAttribute)
    {
        std::string eventName = std::string(onClickAttribute);
        m_propertiesForAllStates.Set("OnClick", eventName);
    }
    if (nameAttribute)
    {
        std::string name = std::string(nameAttribute);
        m_propertiesForAllStates.Set("Name", name);
        m_name = name;
    }
    if (opacityAttribute)
    {
        m_propertiesForAllStates.Set<float>("Opacity", GetFloatFromString(opacityAttribute));
    }
    if (sizeAttribute)
    {
        Vector2 size = Vector2::CreateFromString(sizeAttribute);
        m_propertiesForAllStates.Set<Vector2>("Size", size);
        m_dimensions.SetMinSize(size);
    }
    if (textureAttribute)
    {
        m_texture = Texture::CreateOrGetTexture(std::string(textureAttribute));
    }
    if (materialAttribute)
    {
        if (m_material)
        {
            delete m_material->m_shaderProgram;
            delete m_material;
        }
        //TODO: Have materials load from a file that says what links in. For now they just load a custom fragment shader.
        m_material = new Material(new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", materialAttribute), Renderer::instance->m_defaultMaterial->m_renderState);
    }
    if (dockAttribute)
    {
        std::string dockType = std::string(dockAttribute);
        ToLower(dockType);
        if (dockType == "bottom")
        {
            m_dockType = BOTTOM_DOCKED;
        }
        else if (dockType == "top")
        {
            m_dockType = TOP_DOCKED;
        }
        else if (dockType == "left")
        {
            m_dockType = LEFT_DOCKED;
        }
        else if (dockType == "right")
        {
            m_dockType = RIGHT_DOCKED;
        }
        else if (dockType == "fill")
        {
            m_dockType = FILL_DOCKED;
        }
        else
        {
            ERROR_RECOVERABLE("Warning: Dock label was detected in a widget with an invalid value. Valid values include Top, Bottom, Left, Right");
        }
    }

    m_propertiesForAllStates.Set<Vector2>("Offset", offset);

    std::vector<XMLNode> children = XMLUtils::GetChildren(node);
    for (XMLNode& child : children)
    {
        if (!child.isEmpty())
        {
            AddChild(UISystem::instance->CreateWidget(child));
        }
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::OnClick()
{
    std::string clickEvent;
    PropertyGetResult state = m_propertiesForAllStates.Get("OnClick", clickEvent);
    if (state == PGR_SUCCESS)
    {
        EventSystem::FireEvent(clickEvent);
    }
    UnsetPressed();
}

//-----------------------------------------------------------------------------------
Vector2 WidgetBase::GetParentOffsets() const
{
    Vector2 parentOffsets = Vector2::ZERO;
    WidgetBase* parent = m_parent;
    while (parent)
    {
        parentOffsets += parent->m_propertiesForAllStates.Get<Vector2>("Offset");
        parent = parent->m_parent;
    }
    return parentOffsets;
}

//-----------------------------------------------------------------------------------
float WidgetBase::GetParentOpacities() const
{
    float parentOpacities = 1.0f;
    WidgetBase* parent = m_parent;
    while (parent)
    {
        parentOpacities *= parent->m_propertiesForAllStates.Get<float>("Opacity");
        parent = parent->m_parent;
    }
    return parentOpacities;
}

//-----------------------------------------------------------------------------------
Matrix4x4 WidgetBase::GetModelMatrix() const
{
    Matrix4x4 model = Matrix4x4::IDENTITY;
    Matrix4x4::MatrixMakeTranslation(&model, Vector3(m_propertiesForAllStates.Get<Vector2>("Offset"), 0.0f));
    return model;
}

//-----------------------------------------------------------------------------------
void WidgetBase::SetState(WidgetState newState, bool updatePreviousState)
{
    if (updatePreviousState)
    {
        m_previousState = m_currentState;
    }
    m_currentState = newState;
    m_timeInState = 0.0f;
}

//-----------------------------------------------------------------------------------
void WidgetBase::RevertToPreviousState()
{
    m_currentState = m_previousState;
    m_timeInState = 0.0f;
}

//-----------------------------------------------------------------------------------
bool WidgetBase::SetWidgetVisibility(const std::string& name, bool setHidden)
{
    for (WidgetBase* child : m_children)
    {
        if (child->GetProperty<std::string>("Name") == name)
        {
            setHidden ? child->SetHidden() : child->SetVisible();
            return true;
        }
        else if (child->SetWidgetVisibility(name, setHidden))
        {
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------------
void WidgetBase::SetHidden()
{
    m_currentState = HIDDEN_WIDGET_STATE;
    for (WidgetBase* child : m_children)
    {
        child->SetHidden();
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::SetVisible()
{
    m_currentState = ACTIVE_WIDGET_STATE;
    for (WidgetBase* child : m_children)
    {
        child->SetVisible();
    }
}

//-----------------------------------------------------------------------------------
bool WidgetBase::IsClickable()
{
    return !(m_currentState == DISABLED_WIDGET_STATE || m_currentState == HIDDEN_WIDGET_STATE);
}

//-----------------------------------------------------------------------------------
WidgetBase* WidgetBase::GetWidgetPointIsInside(const Vector2& point)
{
    for (WidgetBase* child : m_children) //Ask in the appropriate drawing order
    {
        WidgetBase* widget = child->GetWidgetPointIsInside(point);
        if (widget)
        {
            return widget;
        }
    }
    return GetWorldBounds().IsPointOnOrInside(point) ? this : nullptr;
}

//-----------------------------------------------------------------------------------
WidgetBase* WidgetBase::FindWidgetByName(const char* widgetName)
{
    WidgetBase* foundWidget = nullptr;

    for (WidgetBase* widget : m_children)
    {
        if (widget->m_name == widgetName)
        {
            foundWidget = widget;
            break;
        }

        foundWidget = widget->FindWidgetByName(widgetName);
        if (foundWidget)
        {
            break;
        }
    }
    return foundWidget;
}
