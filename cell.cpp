#include "cell.h"
#include "formula.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <optional>


class Cell::Impl {
public:
    virtual ~Impl() {}
    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const {return {};};
    virtual void ClearCashe() const {};
};

class Cell::EmptyImpl : public Impl {
public:
    Value GetValue() const override {
        return Value();
    }

    std::string GetText() const override {
        return "";
    }
};

class Cell::TextImpl : public Impl {
public:
    explicit TextImpl(std::string text) : text_(std::move(text)) {}

   Value GetValue() const override {
       if (!text_.empty() && text_[0] == ESCAPE_SIGN) {
            return Value(text_.substr(1));
        }
        return Value(text_);
    }

    std::string GetText() const override {
        return  text_;
    }

private:
    std::string text_;
};

class Cell::FormulaImpl : public Impl {
public:
    explicit FormulaImpl(std::string text,  SheetInterface& sheet) : sheet_(sheet) {
        formula_ = ParseFormula(text.substr(1));
    }

  Value GetValue() const override {
        if(!cashe_) {
            cashe_ = formula_->Evaluate(sheet_);
        }

    return std::visit([](auto val) {
        return CellInterface::Value(val);
    }, cashe_.value());
} 
        
    std::string GetText() const override {
        return  FORMULA_SIGN + formula_->GetExpression() ;
    }

    std::vector<Position> GetReferencedCells() const override {
        return formula_->GetReferencedCells();
    }

    void ClearCashe() const override {
        cashe_.reset();
    }

private:
    std::unique_ptr<FormulaInterface> formula_;
    SheetInterface& sheet_;
    mutable std::optional<FormulaInterface::Value> cashe_;
    
};

Cell::Cell(SheetInterface& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {
}

Cell::~Cell() {}


void Cell::Set(std::string text) {

    std::unique_ptr<Impl> impl;
    if (text.empty()) {
        impl = std::make_unique<EmptyImpl>();
    } else if (text[0] == FORMULA_SIGN && text.size() > 1) {
        impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    } else {
        impl = std::make_unique<TextImpl>(std::move(text));
    }

    if (DetectCycleInDependencies(*impl)) {
        throw CircularDependencyException("Circular dependency detected");
    }

    impl_ = std::move(impl);
    for (const auto dependent: dependentCells_) {
        dependent->referencingCells_.erase(this);
    }

    dependentCells_.clear();

    for (const auto& pos : GetReferencedCells()) {
        Cell* dependentCell = dynamic_cast<Cell*>(sheet_.GetCell(pos));

        if (!dependentCell) {
            sheet_.SetCell(pos, "");
            dependentCell = dynamic_cast<Cell*>(sheet_.GetCell(pos));
        }
        dependentCell->referencingCells_.insert(this);
        dependentCells_.insert(dependentCell);
    }

    ClearCashe();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}


std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const
{
    return impl_->GetReferencedCells();
}



bool Cell::DetectCycleInDependencies(const Impl& new_impl) {
    std::unordered_set<Cell*> refs;
    for(const auto& pos : new_impl.GetReferencedCells()) {
        refs.insert(dynamic_cast<Cell*>(sheet_.GetCell(pos)));
    }

    std::unordered_set<Cell*> visited;

    return  FoundCycle(this, refs, visited);
}

bool Cell::FoundCycle( Cell *current, const std::unordered_set<Cell*> refs, std::unordered_set<Cell*> visisted)
{
    if (visisted.find(current) != visisted.end())
        return false;

    visisted.insert(current);

    if (refs.find(current) != refs.end()) {
        return true;
    }

    for (const auto cell : current->referencingCells_) {
        if (FoundCycle(cell, refs, visisted)) {
            return true;
        }
    }

    return false;
}

void Cell::ClearCashe()
{
    impl_->ClearCashe();
    for (auto cell : referencingCells_) {
        cell->ClearCashe();
    }

}




