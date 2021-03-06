//---------------------------------------------------------------------------
// Based on code written by Squirrel Eiserloh
//
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"

#define STATIC // Do-nothing indicator that method/member is static in class definition

//---------------------------------------------------------------------------
STATIC std::map<size_t, Texture*, std::less<size_t>, UntrackedAllocator<std::pair<size_t, Texture*>>> Texture::s_textureRegistry;

//-----------------------------------------------------------------------------------
STATIC void Texture::CleanUpTextureRegistry()
{
    for (auto texturePair : s_textureRegistry)
    {
        delete texturePair.second;
    }
    s_textureRegistry.clear();
}

//---------------------------------------------------------------------------
Texture::Texture(const std::string& imageFilePath)
    : m_openglTextureID(0)
    , m_texelSize(0, 0)
    , m_imageData(nullptr)
    , m_initializationMethod(TextureInitializationMethod::FROM_DISK)
    , m_textureFormat(TextureFormat::NUM_FORMATS)
{
    int numComponents = 0; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
    int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)
    m_imageData = stbi_load( imageFilePath.c_str(), &m_texelSize.x, &m_texelSize.y, &numComponents, numComponentsRequested );
    ASSERT_OR_DIE(m_imageData != nullptr, Stringf("The texture at %s failed to load!", imageFilePath.c_str()));

    // Enable texturing
    //glEnable( GL_TEXTURE_2D );
    GL_CHECK_ERROR();

    // Tell OpenGL that our pixel data is single-byte aligned
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    GL_CHECK_ERROR();

    // Ask OpenGL for an unused texName (ID number) to use for this texture
    glGenTextures( 1, (GLuint*) &m_openglTextureID );
    GL_CHECK_ERROR();

    // Tell OpenGL to bind (set) this as the currently active texture
    glBindTexture( GL_TEXTURE_2D, m_openglTextureID );
    GL_CHECK_ERROR();

    // Set texture clamp vs. wrap (repeat)
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // one of: GL_CLAMP or GL_REPEAT
    GL_CHECK_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // one of: GL_CLAMP or GL_REPEAT
    GL_CHECK_ERROR();


    // Set magnification (texel > pixel) and minification (texel < pixel) filters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
    GL_CHECK_ERROR();
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );  // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR
    GL_CHECK_ERROR();

    GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
    if (numComponents == 3)
        bufferFormat = GL_RGB;
    else if (numComponents == 1)
        bufferFormat = GL_RED;

    // #FIXME: What happens if numComponents is neither 3 nor 4?

    GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

    glTexImage2D(			// Upload this pixel data to our new OpenGL texture
        GL_TEXTURE_2D,		// Creating this as a 2d texture
        0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
        internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
        m_texelSize.x,		// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
        m_texelSize.y,		// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
        0,					// Border size, in texels (must be 0 or 1)
        bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
        GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
        m_imageData );		// Location of the actual pixel data bytes/buffer
    GL_CHECK_ERROR();

    //glDisable(GL_TEXTURE_2D);
    GL_CHECK_ERROR();
}

//-----------------------------------------------------------------------------------
Texture::Texture(unsigned char* textureData, int numColorComponents, const Vector2Int& texelSize)
    : m_openglTextureID(0)
    , m_texelSize(texelSize.x, texelSize.y)
    , m_imageData(textureData)
    , m_initializationMethod(TextureInitializationMethod::FROM_MEMORY)
    , m_textureFormat(TextureFormat::NUM_FORMATS)
{
    int numComponents = numColorComponents; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)

    // Enable texturing
    //glEnable(GL_TEXTURE_2D);
    GL_CHECK_ERROR();

    // Tell OpenGL that our pixel data is single-byte aligned
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GL_CHECK_ERROR();

    // Ask OpenGL for an unused texName (ID number) to use for this texture
    glGenTextures(1, (GLuint*)&m_openglTextureID);
    GL_CHECK_ERROR();

    // Tell OpenGL to bind (set) this as the currently active texture
    glBindTexture(GL_TEXTURE_2D, m_openglTextureID);
    GL_CHECK_ERROR();

    // Set texture clamp vs. wrap (repeat)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // one of: GL_CLAMP or GL_REPEAT
    GL_CHECK_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // one of: GL_CLAMP or GL_REPEAT
    GL_CHECK_ERROR();

    // Set magnification (texel > pixel) and minification (texel < pixel) filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // one of: GL_NEAREST, GL_LINEAR
    GL_CHECK_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR
    GL_CHECK_ERROR();

    GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
    if (numComponents == 3)
        bufferFormat = GL_RGB;
    else if (numComponents == 1)
        bufferFormat = GL_RED;

    // #FIXME: What happens if numComponents is neither 3 nor 4?

    GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

    glTexImage2D(			// Upload this pixel data to our new OpenGL texture
        GL_TEXTURE_2D,		// Creating this as a 2d texture
        0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
        internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
        m_texelSize.x,		// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
        m_texelSize.y,		// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
        0,					// Border size, in texels (must be 0 or 1)
        bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
        GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
        m_imageData);		// Location of the actual pixel data bytes/buffer
    GL_CHECK_ERROR();

    //glDisable(GL_TEXTURE_2D);
    GL_CHECK_ERROR();
}

Texture::Texture(uint32_t width, uint32_t height, TextureFormat format, void* data)
    : m_initializationMethod(TextureInitializationMethod::FROM_MEMORY)
    , m_texelSize(width, height)
    , m_textureFormat(format)
{
    glGenTextures(1, &m_openglTextureID);
    GLenum bufferChannels = GL_RGBA;
    GLenum bufferFormat = GL_UNSIGNED_INT_8_8_8_8;
    GLenum internalFormat = GL_RGBA8;

    if (format == TextureFormat::RGBA8)
    {
        //Nothing changes
    }
    else if (format == TextureFormat::D24S8)
    {
        bufferChannels = GL_DEPTH_STENCIL;
        bufferFormat = GL_UNSIGNED_INT_24_8;
        internalFormat = GL_DEPTH24_STENCIL8;
    }
    else if (format == TextureFormat::R32UI)
    {
        bufferChannels = GL_RED_INTEGER;
        bufferFormat = GL_UNSIGNED_INT;
        internalFormat = GL_R32UI;
    }
    else
    {
        ERROR_AND_DIE("Unsupported texture enum");
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_openglTextureID);
    glTexImage2D(GL_TEXTURE_2D,
        0, //level - mipmaplevel, set to 0,
        internalFormat, //How texture is stored in memory
        width, height,
        0, //border, again set to 0, we want not 0
        bufferChannels, //channels used by image pass in
        bufferFormat, //format of data of image passed in
        data);	//no actual data passed in, defaults black/white

    GL_CHECK_ERROR();
}

//-----------------------------------------------------------------------------------
Texture::Texture(unsigned char* textureData, size_t bufferSize)
    : m_openglTextureID(0)
    , m_texelSize(0, 0)
    , m_imageData(textureData)
    , m_initializationMethod(TextureInitializationMethod::FROM_MEMORY)
    , m_textureFormat(TextureFormat::NUM_FORMATS)
{
    int numComponents = 0; // Filled in for us to indicate how many color/alpha components the image had (e.g. 3=RGB, 4=RGBA)
    int numComponentsRequested = 0; // don't care; we support 3 (RGB) or 4 (RGBA)
    m_imageData = stbi_load_from_memory(textureData, bufferSize, &m_texelSize.x, &m_texelSize.y, &numComponents, numComponentsRequested);
    ASSERT_OR_DIE(m_imageData != nullptr, "The texture failed to load!");

    // Enable texturing
    //glEnable(GL_TEXTURE_2D);
    GL_CHECK_ERROR();

    // Tell OpenGL that our pixel data is single-byte aligned
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GL_CHECK_ERROR();

    // Ask OpenGL for an unused texName (ID number) to use for this texture
    glGenTextures(1, (GLuint*)&m_openglTextureID);
    GL_CHECK_ERROR();

    // Tell OpenGL to bind (set) this as the currently active texture
    glBindTexture(GL_TEXTURE_2D, m_openglTextureID);
    GL_CHECK_ERROR();

    // Set texture clamp vs. wrap (repeat)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // one of: GL_CLAMP or GL_REPEAT
    GL_CHECK_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // one of: GL_CLAMP or GL_REPEAT
    GL_CHECK_ERROR();

    // Set magnification (texel > pixel) and minification (texel < pixel) filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // one of: GL_NEAREST, GL_LINEAR
    GL_CHECK_ERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR
    GL_CHECK_ERROR();

    GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
    if (numComponents == 3)
        bufferFormat = GL_RGB;
    else if (numComponents == 1)
        bufferFormat = GL_RED;

    // #FIXME: What happens if numComponents is neither 3 nor 4?

    GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

    glTexImage2D(			// Upload this pixel data to our new OpenGL texture
        GL_TEXTURE_2D,		// Creating this as a 2d texture
        0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
        internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
        m_texelSize.x,		// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
        m_texelSize.y,		// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
        0,					// Border size, in texels (must be 0 or 1)
        bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
        GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
        m_imageData);		// Location of the actual pixel data bytes/buffer
    GL_CHECK_ERROR();

    //glDisable(GL_TEXTURE_2D);
    GL_CHECK_ERROR();
}

//-----------------------------------------------------------------------------------
unsigned char* Texture::GetImageData()
{
    return m_imageData;
}

//-----------------------------------------------------------------------------------
Texture::~Texture()
{
    glDeleteTextures(1, &m_openglTextureID);
    if (m_imageData)
    {
        switch (m_initializationMethod)
        {
        //We have an additional step for STBI loaded images
        case TextureInitializationMethod::FROM_DISK:
            stbi_image_free(m_imageData);
            break;
        case TextureInitializationMethod::FROM_MEMORY:
            break;
        case TextureInitializationMethod::NUM_INITIALIZATION_METHODS:
            break;
        default:
            break;
        }
    }
}

//---------------------------------------------------------------------------
// Returns a pointer to the already-loaded texture of a given image file,
//	or nullptr if no such texture/image has been loaded.
//
STATIC Texture* Texture::GetTextureByName(const std::string& imageFilePath)
{
    size_t filePathHash = std::hash<std::string>{}(imageFilePath);
    auto iterator = Texture::s_textureRegistry.find(filePathHash);
    if (iterator == Texture::s_textureRegistry.end())
    {
        return nullptr;
    }
    else
    {
        return iterator->second;
    }
}


//---------------------------------------------------------------------------
// Finds the named Texture among the registry of those already loaded; if
//	found, returns that Texture*.  If not, attempts to load that texture,
//	and returns a Texture* just created (or nullptr if unable to load file).
//
STATIC Texture* Texture::CreateOrGetTexture(const std::string& imageFilePath)
{
    Texture* texture = GetTextureByName(imageFilePath);
    if (texture != nullptr)
    {
        return texture;
    }
    else
    {
        texture = new Texture(imageFilePath);
        size_t filePathHash = std::hash<std::string>{}(imageFilePath);
        Texture::s_textureRegistry[filePathHash] = texture;
        return texture;
    }
}

//-----------------------------------------------------------------------------------
void Texture::RegisterTexture(const std::string& textureName, Texture* texture)
{
    //TODO: If we have a collision and someone was using that texture, we're in trouble :T

    Texture* foundTexture = GetTextureByName(textureName);
    if (foundTexture)
    {
        delete foundTexture;
    }
    size_t stringHash = std::hash<std::string>{}(textureName);
    Texture::s_textureRegistry[stringHash] = texture;
}

//-----------------------------------------------------------------------------------
bool Texture::CleanUpTexture(const std::string& textureName)
{
    size_t textureNameHash = std::hash<std::string>{}(textureName);
    auto iterator = Texture::s_textureRegistry.find(textureNameHash);
    if (iterator == Texture::s_textureRegistry.end())
    {
        return false;
    }
    else
    {
        delete iterator->second;
        Texture::s_textureRegistry.erase(iterator);
        return true;
    }
}

//-----------------------------------------------------------------------------------
Texture* Texture::CreateTextureFromImageFileData(const std::string& textureName, unsigned char* textureData, size_t bufferLength)
{
    //Buffer Length in indices, not bytes.
    Texture* texture = new Texture(textureData, bufferLength);
    texture->m_initializationMethod = TextureInitializationMethod::FROM_DISK; //Need to do the STBI cleanup.
    size_t textureNameHash = std::hash<std::string>{}(textureName);
    Texture::s_textureRegistry[textureNameHash] = texture;
    return texture;
}

//-----------------------------------------------------------------------------------
Texture* Texture::CreateTextureFromData(const std::string& textureName, unsigned char* textureData, int numComponents, const Vector2Int& texelSize)
{
    Texture* texture = new Texture(textureData, numComponents, texelSize);
    size_t stringHash = std::hash<std::string>{}(textureName);
    Texture::s_textureRegistry[stringHash] = texture;
    return texture;
}

//-----------------------------------------------------------------------------------
TexturePool::TexturePool()
{

}

//-----------------------------------------------------------------------------------
TexturePool::~TexturePool()
{
    FlushPool();
}

//-----------------------------------------------------------------------------------
void TexturePool::AddToPool(Texture* texture)
{
    m_availableTextures.push_back(texture);
}

//-----------------------------------------------------------------------------------
Texture* TexturePool::GetUnusedTexture()
{
    ASSERT_OR_DIE(m_availableTextures.size() > 0, "No more available textures in the texture pool");
    Texture* texture = m_availableTextures.back();
    m_availableTextures.pop_back();
    m_texturesInUse.push_back(texture);
    return texture;
}

//-----------------------------------------------------------------------------------
void TexturePool::ReturnToPool(Texture* texture)
{
    m_availableTextures.push_back(texture);

    unsigned int numTexInUse = m_texturesInUse.size();
    for (unsigned int i = 0; i < numTexInUse; ++i)
    {
        if (m_texturesInUse[i] == texture)
        {
            m_texturesInUse[i] = m_texturesInUse[numTexInUse - 1];
            m_texturesInUse.pop_back();
            return;
        }
    }
    ERROR_AND_DIE("Returned a texture to the texture pool that wasn't in use");
}

//-----------------------------------------------------------------------------------
void TexturePool::FlushPool()
{
    for (Texture* tex : m_availableTextures)
    {
        delete tex;
    }
    for (Texture* tex : m_texturesInUse)
    {
        delete tex;
    }
    m_availableTextures.clear();
    m_texturesInUse.clear();
}
