#include "Engine/Renderer/2D/Renderable2D.hpp"
#include "SpriteGameRenderer.hpp"

//-----------------------------------------------------------------------------------
Renderable2D::Renderable2D(int orderingLayer, bool isEnabled)
    : m_isEnabled(false)
    , m_orderingLayer(-1)
    , m_viewableBy((uchar)SpriteGameRenderer::PlayerVisibility::ALL)
{
    m_orderingLayer = orderingLayer;
    if (isEnabled)
    {
        Enable();
    }
}

//-----------------------------------------------------------------------------------
Renderable2D::~Renderable2D()
{
    Disable();
}

//-----------------------------------------------------------------------------------
void Renderable2D::ChangeLayer(int layer)
{
    //Disable if already connected.
    Disable();
    m_orderingLayer = layer;
    Enable();
}

//-----------------------------------------------------------------------------------
void Renderable2D::Enable()
{
    if (!m_isEnabled)
    {
        SpriteGameRenderer::instance->RegisterRenderable2D(this);
        m_isEnabled = true;
    }
}

//-----------------------------------------------------------------------------------
void Renderable2D::Disable()
{
    if (m_isEnabled)
    {
        SpriteGameRenderer::instance->UnregisterRenderable2D(this);
        m_isEnabled = false;
    }
}

//-----------------------------------------------------------------------------------
void Renderable2D::Render(BufferedMeshRenderer& )
{

}

//-----------------------------------------------------------------------------------
AABB2 Renderable2D::GetBounds()
{
    return AABB2();
}
