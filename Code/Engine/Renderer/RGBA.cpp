#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

const RGBA RGBA::WHITE(0xFFFFFFFF);
const RGBA RGBA::BLACK(0x000000FF);
const RGBA RGBA::RED(0xFF0000FF);
const RGBA RGBA::GREEN(0x00FF00FF);
const RGBA RGBA::BLUE(0x0000FFFF);
const RGBA RGBA::MAGENTA(0xFF00FFFF);
const RGBA RGBA::YELLOW(0xFFFF00FF);
const RGBA RGBA::CYAN(0x00FFFFFF);
const RGBA RGBA::ORANGE(0xFFBB00FF);
const RGBA RGBA::CORNFLOWER_BLUE(0x6495EDFF);
const RGBA RGBA::TURQUOISE(0x00868BFF);
const RGBA RGBA::SLATE(0x2F4F4FFF);
const RGBA RGBA::FOREST_GREEN(0x228B22FF);
const RGBA RGBA::SEA_GREEN(0x43CD80FF);
const RGBA RGBA::KHAKI(0xCDC673FF);
const RGBA RGBA::GOLD(0xCDAD00FF);
const RGBA RGBA::CHOCOLATE(0xD2691EFF);
const RGBA RGBA::SADDLE_BROWN(0x8B4513FF);
const RGBA RGBA::MAROON(0x800000FF);
const RGBA RGBA::LIGHT_GRAY(0xBBBBBBFF);
const RGBA RGBA::GRAY(0x808080FF);
const RGBA RGBA::DARK_GRAY(0x333333FF);
const RGBA RGBA::VAPORWAVE(0xFF819CFF);
const RGBA RGBA::DEADBEEF(0xDEADBEEF);
const RGBA RGBA::FEEDFACE(0xFEEDFACE);
const RGBA RGBA::DISEASED(0xD15EA5ED);
const RGBA RGBA::BADDAD(0xBADDADFF); 
const RGBA RGBA::CERULEAN(0x003FFFFF);
const RGBA RGBA::GBBLACK(0x0B1920FF);
const RGBA RGBA::GBLIGHTGREEN(0x86C270FF);
const RGBA RGBA::GBDARKGREEN(0x2F6957FF);
const RGBA RGBA::GBWHITE(0xF5FAEFFF);
const RGBA RGBA::MUDKIP_BLUE(0x0F9AD1FF);
const RGBA RGBA::MUDKIP_ORANGE(0xED8220FF);
const RGBA RGBA::JOLTIK_YELLOW(0xF9E27CFF);
const RGBA RGBA::JOLTIK_PURPLE(0x767EAFFF);
const RGBA RGBA::JOLTIK_DARK_PURPLE(0x575876FF);
const RGBA RGBA::PURPLE(0x8A1089FF);
const RGBA RGBA::EARTHBOUND_BLUE(0x68A8D8FF);
const RGBA RGBA::EARTHBOUND_GREEN(0x80D890FF);
const RGBA RGBA::CLEAR(0xFFFFFF00);

//-----------------------------------------------------------------------------------
RGBA::RGBA() : red(0x00), green(0x00), blue(0x00), alpha(0xFF)
{
}

//-----------------------------------------------------------------------------------
RGBA::RGBA(float Red, float Green, float Blue) 
    : red(static_cast<unsigned char>((255.0f * Red)))
    , green(static_cast<unsigned char>((255.0f * Green)))
    , blue(static_cast<unsigned char>((255.0f * Blue)))
    , alpha(0xFF)
{

}

//-----------------------------------------------------------------------------------
RGBA::RGBA(float Red, float Green, float Blue, float Alpha)
    : red(static_cast<unsigned char>((255.0f * Red)))
    , green(static_cast<unsigned char>((255.0f * Green)))
    , blue(static_cast<unsigned char>((255.0f * Blue)))
    , alpha(static_cast<unsigned char>((255.0f * Alpha)))
{

}

//-----------------------------------------------------------------------------------
RGBA::RGBA(unsigned int hexColor)
    : red(GetRed(hexColor))
    , green(GetGreen(hexColor))
    , blue(GetBlue(hexColor))
    , alpha(static_cast<unsigned char>((hexColor & ALPHA_MASK)))
{
}

//-----------------------------------------------------------------------------------
RGBA::RGBA(const Vector4& color)
    : red(static_cast<unsigned char>((255.0f * color.x)))
    , green(static_cast<unsigned char>((255.0f * color.y)))
    , blue(static_cast<unsigned char>((255.0f * color.z)))
    , alpha(static_cast<unsigned char>((255.0f * color.w)))
{

}

//-----------------------------------------------------------------------------------
RGBA::~RGBA()
{
}

//-----------------------------------------------------------------------------------
void RGBA::SetAlphaFloat(float newAlpha)
{
    this->alpha = static_cast<unsigned char>((255.0f * newAlpha));
}

//-----------------------------------------------------------------------------------
Vector4 RGBA::ToVec4() const
{
    return Vector4(static_cast<float>(red) / 255.0f, static_cast<float>(green) / 255.0f, static_cast<float>(blue) / 255.0f, static_cast<float>(alpha) / 255.0f);
}

//-----------------------------------------------------------------------------------
RGBA RGBA::GetInverse() const
{
    return CreateFromUChars(0xFF - red, 0xFF - green, 0xFF - blue, alpha);
}

//-----------------------------------------------------------------------------------
float RGBA::GetAlphaFloat() const
{
    return (static_cast<float>(alpha) / 255.0f);
}

//-----------------------------------------------------------------------------------
RGBA RGBA::CreateFromString(const std::string& textColor)
{
    if (textColor.empty())
    {
        return RGBA::WHITE;
    }

    if (textColor.at(1) == 'x') //Format 0xRRGGBBAA
    {
        unsigned int hexColor = stoul(textColor, nullptr, 0);
        return RGBA(hexColor);
    }
    else //Format R,G,B
    {
        std::vector<std::string>* strings = SplitString(textColor, ",");
        return RGBA(stoi(strings->at(0)) / 255.0f, stoi(strings->at(1)) / 255.0f, stoi(strings->at(2)) / 255.0f);
    }
}

//-----------------------------------------------------------------------------------
Vector4 RGBA::GetRandomVec4()
{
    return Vector4(MathUtils::GetRandomFloatFromZeroTo(1.0f), MathUtils::GetRandomFloatFromZeroTo(1.0f), MathUtils::GetRandomFloatFromZeroTo(1.0f), 1.0f);
}

//-----------------------------------------------------------------------------------
RGBA RGBA::GetRandom()
{
    return RGBA(Vector4(MathUtils::GetRandomFloatFromZeroTo(1.0f), MathUtils::GetRandomFloatFromZeroTo(1.0f), MathUtils::GetRandomFloatFromZeroTo(1.0f), 1.0f));
}

