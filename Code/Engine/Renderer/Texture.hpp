#pragma once

#include <map>
#include <vector>
#include "Engine/Math/Vector2Int.hpp"
#include "../Core/Memory/UntrackedAllocator.hpp"
#include "RGBA.hpp"

class Texture
{
public:
    //ENUMS//////////////////////////////////////////////////////////////////////////
    enum class TextureFormat
    {
        RGBA8, //RGBA, 8 bits per channel
        D24S8, //Depth 24, Stencil 8
        R32UI, 
        NUM_FORMATS
    };

    enum class TextureInitializationMethod
    {
        FROM_DISK,
        FROM_MEMORY,
        NUM_INITIALIZATION_METHODS
    };

    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    Texture(uint32_t width, uint32_t height, TextureFormat format, void* data = nullptr);
    ~Texture();
    static Texture* CreateOrGetTexture(const std::string& imageFilePath);
    static Texture* CreateTextureFromData(const std::string& textureName, unsigned char* textureData, int numComponents, const Vector2Int& texelSize);
    static Texture* CreateTextureFromImageFileData(const std::string& textureName, unsigned char* textureData, size_t bufferLength);
    static void RegisterTexture(const std::string& textureName, Texture* texture); //If you create a texture by rendering to an FBO, save it here so we can delete it safely later.
    static bool CleanUpTexture(const std::string& textureName);
    static void CleanUpTextureRegistry();

    //GETTERS//////////////////////////////////////////////////////////////////////////
    static Texture* GetTextureByName(const std::string& imageFilePath);
    unsigned char* GetImageData();

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    unsigned int m_openglTextureID;
    Vector2Int m_texelSize;
    TextureFormat m_textureFormat;
    TextureInitializationMethod m_initializationMethod;
    unsigned char* m_imageData;

private:
    Texture(const std::string& imageFilePath);
    Texture(unsigned char* textureData, int numColorComponents, const Vector2Int& texelSize);
    Texture(unsigned char* textureData, size_t bufferSize);
    static std::map<size_t, Texture*, std::less<size_t>, UntrackedAllocator<std::pair<size_t, Texture*>>> s_textureRegistry;
};

//-----------------------------------------------------------------------------------
class TexturePool
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    TexturePool();
    ~TexturePool();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void AddToPool(Texture* texture);
    Texture* GetUnusedTexture();
    void ReturnToPool(Texture* texture);
    void FlushPool();

    //QUERIES/////////////////////////////////////////////////////////////////////
    inline bool HasAnyTextures() { return (m_texturesInUse.size() + m_availableTextures.size()) > 0; };
    inline bool HasUnusedTextures() { return (m_availableTextures.size() > 0); };
    inline bool HasInUseTextures() { return (m_texturesInUse.size() > 0); };

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Texture*> m_texturesInUse;
    std::vector<Texture*> m_availableTextures;
};