#pragma once
#include <string>
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Math/MatrixStack4x4.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "../Math/Vector2Int.hpp"
#include "UniformBuffer.hpp"

//-----------------------------------------------------------------------------------
#ifdef CHECK_GL_ERRORS
    #define GL_CHECK_ERROR()  Renderer::GLCheckError(__FILE__, __LINE__);
#else
    #define GL_CHECK_ERROR()
#endif

//-----------------------------------------------------------------------------------
class AABB2;
class AABB3;
class Texture;
class Face;
class BitmapFont;
class ShaderProgram;
class Material;
class MeshRenderer;
class Framebuffer;
class TextBox;
struct Vertex_PCT;
struct Vertex_PCUTB;

class Renderer
{
public:
    //ENUMS//////////////////////////////////////////////////////////////////////////
    enum class DrawMode : unsigned int
    {
        QUADS,
        QUAD_STRIP,
        POINTS,
        LINES,
        LINE_LOOP,
        POLYGON,
        TRIANGLES,
        TRIANGLE_STRIP,
        NUM_DRAW_MODES
    };

    //TYPEDEFS//////////////////////////////////////////////////////////////////////////
    typedef unsigned int GLuint;
    typedef int GLint;
    typedef int GLsizei;
    typedef unsigned int GLenum;
    typedef bool GLboolean;


    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    Renderer(const Vector2Int& windowSize);
    ~Renderer();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void ClearScreen(float red, float green, float blue);
    void ClearScreen(const RGBA& color);
    void ClearColor(const RGBA& color);
    void PushMatrix();
    void PopMatrix();
    void Translate(float x, float y, float z);
    void Translate(const Vector2& xy);
    void Translate(const Vector3& xyz);
    void Rotate(float rotationDegrees);
    void Rotate(float rotationDegrees, float x, float y, float z);
    void Scale(float x, float y, float z);
    unsigned char GetDrawMode(DrawMode mode) const;

    //STATE MODIFICATION//////////////////////////////////////////////////////////////////////////
    void EnableAdditiveBlending();
    void EnableAlphaBlending();
    void EnableInvertedBlending();
    void EnableDepthTest(bool enabled);
    void EnableDepthWrite();
    void DisableDepthWrite();
    void EnableFaceCulling(bool enabled);
    void BindTexture(const Texture& texture);
    void UnbindTexture();
    void SetOrtho(const Vector2& bottomLeft, const Vector2& topRight);
    void BeginOrtho(const Vector2& bottomLeft, const Vector2& topRight);
    void BeginOrtho(float width, float height, const Vector2& cameraOffset = Vector2::ZERO);
    void EndOrtho();
    void SetPerspective(float fovDegreesY, float aspect, float nearDist, float farDist);
    void BeginPerspective(float fovDegreesY, float aspect, float nearDist, float farDist);
    void EndPerspective();
    void SetViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    void SetPointSize(float size);
    void SetLineWidth(float width);

    //BUFFERS//////////////////////////////////////////////////////////////////////////
    int GenerateBufferID();
    void DeleteBuffers(int vboID);
    void BindAndBufferVBOData(int vboID, const Vertex_PCT* vertexes, int numVerts);
    void BindAndBufferVBOData(int vboID, const Vertex_PCUTB* vertexes, int numVerts);
    void DrawVertexArray(const Vertex_PCT* vertexes, int numVertexes, DrawMode drawMode = DrawMode::TRIANGLE_STRIP);
    void DrawVBO_PCT(unsigned int vboID, int numVerts, DrawMode drawMode = DrawMode::TRIANGLE_STRIP, Texture* texture = nullptr);
    void DrawVBO_PCUTB(unsigned int vboID, int numVerts, DrawMode drawMode = DrawMode::TRIANGLE_STRIP, Texture* texture = nullptr);

    //DRAWING//////////////////////////////////////////////////////////////////////////
    void DrawPoint(const Vector2& point, const RGBA& color = RGBA::WHITE, float pointSize = 1.0f);
    void DrawPoint(const Vector3& point, const RGBA& color = RGBA::WHITE, float pointSize = 1.0f);
    void DrawPoint(float x, float y, const RGBA& color = RGBA::WHITE, float pointSize = 1.0f);
    void DrawLine(const Vector2& start, const Vector2& end, const RGBA& color = RGBA::WHITE);
    void DrawLine(const Vector3& start, const Vector3& end, const RGBA& color = RGBA::WHITE);
    void DrawAABB(const AABB2& bounds, const RGBA& color = RGBA::WHITE);
    void DrawAABB(const AABB3& bounds, const RGBA& color = RGBA::WHITE);
    void DrawAABBBoundingBox(const AABB3& bounds, const RGBA& color = RGBA::WHITE);
    void DrawTexturedAABB3(const AABB3& bounds, const RGBA& color = RGBA::WHITE, const Vector2& texCoordMins = Vector2::ZERO, const Vector2& texCoordMaxs = Vector2::ONE, Texture* texture = nullptr);
    void DrawTexturedAABB(const AABB2& bounds, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture = nullptr, const RGBA& color = RGBA::WHITE);
    void DrawTexturedFace(const Face& face, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture = nullptr, const RGBA& color = RGBA::WHITE);
    void SetRenderTargets(size_t colorCount, Texture** inColorTargets, Texture* depthStencilTarget);
    void BindFramebuffer(Framebuffer* fbo);
    void FrameBufferCopyToBack(Framebuffer* fbo, uint32_t drawingWidth, uint32_t drawingHeight, uint32_t bottomLeftX = 0, uint32_t bottomLeftY = 0, int colorTargetNumber = NULL);
    void RenderFullScreenEffect(Material* material);
    void DrawPolygonOutline(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color = RGBA::WHITE);
    void DrawPolygon(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color = RGBA::WHITE);
    void DrawText2D(const Vector2& startBottomLeft, const std::string& asciiText, float cellWidth, float cellHeight, const RGBA& tint = RGBA::WHITE, bool drawShadow = false, const BitmapFont* font = nullptr);
    void DrawText2D(const Vector2& position, const std::string& asciiText, float scale, const RGBA& tint = RGBA::WHITE, bool drawShadow = false, const BitmapFont* font = nullptr, const Vector2& right = Vector2::UNIT_X, const Vector2& up = Vector2::UNIT_Y);

    //MODERN RENDERING (AKA: ORGANIZE THESE LATER)//////////////////////////////////////////////////////////////////////////
    static void GLCheckError(const char* file, size_t line);
    void UnbindIbo();
    void RenderBufferDestroy(GLuint buffer);                                    
    GLuint GenerateVAOHandle();
    GLuint RenderBufferCreate(void* data, size_t count, size_t elementSize, GLenum usage/* = GL_STATIC_DRAW*/);
    int CreateSampler(GLenum min_filter, GLenum magFilter, GLenum uWrap, GLenum vWrap);
    void DeleteSampler(GLuint id);
    inline void PushProjection(const Matrix4x4& proj) { m_projStack.Push(proj); };
    inline void PushView(const Matrix4x4& view) { m_viewStack.Push(view); };
    inline void PopProjection() { m_projStack.Pop(); };
    inline void PopView() { m_viewStack.Pop(); };
    inline Matrix4x4 GetProjection() { return m_projStack.GetTop(); };
    inline Matrix4x4 GetView() { return m_viewStack.GetTop(); };
    void RotateView(float degrees, const Vector3& axis);
    void TranslateView(const Vector3& translation);
    void DeleteVAOHandle(GLuint m_vaoID);
    void ClearDepth(float depthValue = 1.0f);
    void UseShaderProgram(GLuint shaderProgramID);
    GLuint CreateRenderBuffer(size_t size, void* data = nullptr);
    void BindUniform(unsigned int bindPoint, UniformBuffer& buffer);

    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const int CIRCLE_SIDES = 50;
    static const int HEXAGON_SIDES = 6;
    static const unsigned char plainWhiteTexel[3];
    static Renderer* instance;

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////v
    BitmapFont* m_defaultFont;
    Texture* m_defaultTexture;
    ShaderProgram* m_defaultShader;
    Material* m_defaultMaterial;
    MeshRenderer* m_fboFullScreenEffectQuad;
    Framebuffer* m_fbo;
    MatrixStack4x4 m_projStack;
    MatrixStack4x4 m_viewStack;
    RenderState::BlendMode m_blendMode;
    Vector2Int m_windowSize;
    bool m_faceCullingEnabled;
    bool m_depthTestingEnabled;
    bool m_depthWritingEnabled;
    float m_lineWidth = 0.0f;
    float m_pointSize = 0.0f;
    GLint m_viewportX = 0;
    GLint m_viewportY = 0;
    GLsizei m_viewportWidth = 0;
    GLsizei m_viewportHeight = 0;
    GLuint m_fboHandle = NULL;
    GLuint m_currentShaderProgramId = NULL;
};