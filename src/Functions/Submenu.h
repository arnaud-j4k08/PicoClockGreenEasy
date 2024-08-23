#pragma once

#include "AbstractFunction.h"

class Submenu : public AbstractFunction
{
public:
    Submenu(
        ClockUi *clockUi, 
        const std::string &name, 
        std::vector<std::unique_ptr<AbstractFunction>> *parentMenu) 
        : AbstractFunction(clockUi), m_name(name), m_parentMenu(parentMenu)
    {}

    template <class FunctionType, typename... CtorParams>
    FunctionType *addFunction(ClockUi *clockUi, CtorParams... ctorParams)
    {
        auto uniquePtr = std::make_unique<FunctionType>(clockUi, ctorParams...);
        FunctionType *rawPtr = uniquePtr.get();

        m_functions.push_back(std::move(uniquePtr));

        return rawPtr;
    }

    std::vector<std::unique_ptr<AbstractFunction>> *menu()
    {
        return &m_functions;
    }

protected:
    void setName(const std::string &name)
    {
        m_name = name;
    }

    void renderFrame(
        Bitmap &frame, int editedValueIndex, int blinkingCounter, bool fullRefresh) override;

private:
    void activate() override;
    int valueCount() const override
    {
        return 1;
    }

    void exit();

    std::string m_name;
    std::vector<std::unique_ptr<AbstractFunction>> *m_parentMenu;
    std::vector<std::unique_ptr<AbstractFunction>> m_functions;
    bool m_exitAdded = false;
};