#include "Engine/UI/UISystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/UI/Widgets/LabelWidget.hpp"
#include "Engine/UI/Widgets/ButtonWidget.hpp"
#include "Engine/Input/InputSystem.hpp"

UISystem* UISystem::instance = nullptr;

//-----------------------------------------------------------------------------------
UISystem::UISystem()
{
    LoadAndParseUIXML();
    WidgetBase* button = CreateWidget("Button");
    button->SetProperty<std::string>("Name", "CodeButton");
    button->SetProperty<std::string>("Text", "I AM FROM CODE!");
    button->SetProperty<std::string>("OnClick", "StartGame");
    button->SetProperty("BackgroundColor", RGBA::TURQUOISE, WidgetState::HIGHLIGHTED_WIDGET_STATE);
    button->SetProperty("Offset", Vector2(700, 20));
    button->SetProperty("BorderWidth", 5.0f);
    button->m_currentState = WidgetState::ACTIVE_WIDGET_STATE;
    AddWidget(button);

    WidgetBase* child = CreateWidget("Button");
    child->SetProperty<std::string>("Name", "CodeLabel");
    child->SetProperty<std::string>("Text", "I AM el Nino!");
    child->SetProperty<std::string>("OnClick", "StartGame");
    child->SetProperty("BackgroundColor", RGBA::GBDARKGREEN, WidgetState::HIGHLIGHTED_WIDGET_STATE);
    child->SetProperty("Offset", Vector2(0, 50));
    child->SetProperty("FontSize", 0.5f);
    child->SetProperty("BorderWidth", 5.0f);
    child->m_currentState = WidgetState::ACTIVE_WIDGET_STATE;
    button->AddChild(child);
}

//-----------------------------------------------------------------------------------
UISystem::~UISystem()
{
    DeleteAllUI();
}

//-----------------------------------------------------------------------------------
void UISystem::Update(float deltaSeconds)
{
    if (InputSystem::instance->WasKeyJustPressed('U'))
    {
        ReloadUI();
    }

    WidgetBase* newHighlightedWidget = FindHighlightedWidget();
    if (newHighlightedWidget != m_highlightedWidget)
    {
        if (m_highlightedWidget && m_highlightedWidget->IsClickable())
        {
            m_highlightedWidget->UnsetHighlighted();
        }
        if (newHighlightedWidget && newHighlightedWidget->IsClickable())
        {
            newHighlightedWidget->SetHighlighted();
        }
        m_highlightedWidget = newHighlightedWidget;
    }
    if (InputSystem::instance->IsMouseButtonDown(InputSystem::LEFT_MOUSE_BUTTON) && m_highlightedWidget && m_highlightedWidget->IsClickable())
    {
        m_highlightedWidget->SetPressed();
    }

    if (InputSystem::instance->WasMouseButtonJustReleased(InputSystem::LEFT_MOUSE_BUTTON))
    {
        if (m_highlightedWidget && m_highlightedWidget->IsClickable())
        {
            m_highlightedWidget->OnClick();
        }
    }

    for (WidgetBase* widget : m_childWidgets)
    {
        if (widget->IsHidden())
        {
            continue;
        }
        else 
        {
            widget->Update(deltaSeconds);
        }
    }
}

//-----------------------------------------------------------------------------------
void UISystem::Render() const
{
    Renderer::instance->ClearDepth();
    Renderer::instance->m_defaultMaterial->m_renderState.depthTestingMode = RenderState::DepthTestingMode::OFF;
    Renderer::instance->BeginOrtho(Vector2::ZERO, Vector2(1600, 900));
    {
        for (WidgetBase* widget : m_childWidgets)
        {
            if (widget->IsHidden())
            {
                continue;
            }
            else
            {
                widget->Render();
            }
        }
    }
    Renderer::instance->EndOrtho();
    Renderer::instance->m_defaultMaterial->m_renderState.depthTestingMode = RenderState::DepthTestingMode::ON;
}

//-----------------------------------------------------------------------------------
void UISystem::LoadAndParseUIXML()
{
    XMLNode root = XMLUtils::OpenXMLDocument("Data/UI/Widget.xml");
    std::vector<XMLNode> children = XMLUtils::GetChildren(root);
    for (XMLNode& node : children)
    {
        if (!node.isEmpty())
        {
            AddWidget(CreateWidget(node));
        }
    }
}

//-----------------------------------------------------------------------------------
void UISystem::ReloadUI()
{
    DeleteAllUI();
    LoadAndParseUIXML();
}

//-----------------------------------------------------------------------------------
void UISystem::DeleteAllUI()
{
    for (WidgetBase* child : m_childWidgets)
    {
        delete child;
    }
    m_childWidgets.clear();
}

//-----------------------------------------------------------------------------------
WidgetBase* UISystem::CreateWidget(XMLNode& node)
{
    std::string nodeName = node.getName();
    WidgetBase* widget = CreateWidget(nodeName);
    widget->BuildFromXMLNode(node);
    return widget;
}

//-----------------------------------------------------------------------------------
WidgetBase* UISystem::CreateWidget(const std::string& name)
{
    if (name == "Label")
    {
        return static_cast<WidgetBase*>(new LabelWidget());
    }
    else if (name == "Button")
    {
        return static_cast<WidgetBase*>(new ButtonWidget());
    }
    return nullptr;
}

//-----------------------------------------------------------------------------------
bool UISystem::SetWidgetVisibility(const std::string& name, bool setHidden)
{
    for (WidgetBase* child : m_childWidgets)
    {
        if (child->GetProperty<std::string>("Name") == name)
        {
            setHidden ? child->SetHidden() : child->SetVisible();
            return true;
        }
        else if(child->SetWidgetVisibility(name, setHidden))
        {
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------------
Vector2 UISystem::ScreenToUIVirtualCoords(const Vector2& point)
{
#pragma todo("Make this relative too, please ;w;")
    float adjustedY = 900 - point.y;
    return Vector2(point.x, adjustedY);
}

//-----------------------------------------------------------------------------------
WidgetBase* UISystem::FindHighlightedWidget()
{
    Vector2 cursorVirtualPos = GetCursorVirtualPos();
    WidgetBase* widgetAtPos = nullptr;

    for (WidgetBase* widget : m_childWidgets)
    {
        WidgetBase* foundWidget = widget->GetWidgetPointIsInside(cursorVirtualPos);
        if (foundWidget)
        {
            widgetAtPos = foundWidget;
            break;
        }
    }
    
    return widgetAtPos;
}

//-----------------------------------------------------------------------------------
Vector2 UISystem::GetCursorVirtualPos()
{
    Vector2 cursorPos = static_cast<Vector2>(InputSystem::instance->GetMousePos());
    Vector2 virtualCursorPos = ScreenToUIVirtualCoords(cursorPos);
    DebuggerPrintf("X: %f, Y:%f\n", virtualCursorPos.x, virtualCursorPos.y);
    return virtualCursorPos;
}

//-----------------------------------------------------------------------------------
void UISystem::AddWidget(WidgetBase* newWidget)
{
    ASSERT_OR_DIE(newWidget, "Attempted to add a nullptr as a widget.");
    m_childWidgets.push_back(newWidget);
    newWidget->RecalculateBounds();
}

