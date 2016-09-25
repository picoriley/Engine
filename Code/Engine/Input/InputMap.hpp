#pragma once
#include <map>

class InputValue;
class InputAxis;
class InputVector2;
class Vector2;

// CInputMap - tying this shit together
// Acts as a virtual device, but doesn't extend device
class InputMap
{
public:
    InputMap();
    ~InputMap();

    InputValue* AddInputValue(std::string const &name);
    InputValue* AddInputValue(std::string const &name, InputValue *other);
    InputValue* FindInputValue(std::string const &name);
    float GetValue(std::string const &name);
    InputAxis* AddInputAxis(std::string const &name);
    InputAxis* AddInputAxis(const std::string& name, InputValue* positiveInput, InputValue* negativeInput);
    InputAxis* FindInputAxis(std::string const &name);
    Vector2 GetVector2(const std::string& xName, const std::string& yName);

    std::map<std::string, InputValue*> m_values;
    std::map<std::string, InputAxis*> m_axies;
};