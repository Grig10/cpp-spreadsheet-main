#pragma once

#include "common.h"
#include <set>
#include <functional>
#include <unordered_set>
#include <forward_list>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;



private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;
    std::set<Cell*> dependentCells_;
    std::set<Cell*> referencingCells_;

    bool DetectCycleInDependencies(const Impl& new_impl);
    bool FoundCycle( Cell* current, const std::unordered_set<Cell*> refs,
                    std::unordered_set<Cell*> visisted);
    void ClearCashe();
};
