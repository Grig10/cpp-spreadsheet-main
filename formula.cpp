#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <set>

using namespace std::literals;


FormulaError::FormulaError(Category category): category_(category) {

}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
    case FormulaError::Category::Div0:
        return "#DIV/0!";
    case FormulaError::Category::Value:
        return "#VALUE!";
    case FormulaError::Category::Ref:
        return "#REF!";
    case FormulaError::Category::Unknown:
        return "";
    }
    return "";
}


std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) try : ast(ParseFormulaAST(expression)) {
    }
    catch (const std::exception& e) {
        std::throw_with_nested(FormulaException(e.what()));
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        const auto evaluator = [&sheet]( Position* p) -> double {
            if (!p->IsValid()) {
                throw FormulaError(FormulaError::Category::Ref); // Ошибка ссылки
            }

            auto cell = sheet.GetCell(*p);

            if (!cell) {
                return 0; // Пустая ячейка
            }

             auto cellValue = cell->GetValue();

            if (auto* numericValue = std::get_if<double>(&cellValue)) {
                return *numericValue; // Числовое значение
            }

            else if (auto* stringValue = std::get_if<std::string>(&cellValue)) {
                double result = 0;

                if (!stringValue->empty()) {
                    std::istringstream in(*stringValue);

                    if (!(in >> result) || !in.eof()) {
                        throw FormulaError(FormulaError::Category::Value); // Ошибка значения
                    }
                }
                return result;
            }

            else if (auto* formulaError = std::get_if<FormulaError>(&cellValue)) {
                throw *formulaError; // Передаем ошибку формулы
            }

            throw FormulaError(FormulaError::Category::Value); // Неизвестный тип значения
        };

        try {
            return ast.Execute(evaluator);
        } catch (FormulaError& e) {
            return e; // Обработка ошибок формулы
        }
    }




    std::string GetExpression() const override {
        std::ostringstream expression;
        ast.PrintFormula(expression);
        return expression.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        std::set<Position> uniqueCells;
        for (auto cell : ast.GetCells()) {
            if(cell.IsValid())
                uniqueCells.insert(cell);
        }
        std::vector<Position> cells(uniqueCells.begin(), uniqueCells.end());
        return cells;
    }

private:
    FormulaAST ast;
};

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
