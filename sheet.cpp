#include "sheet.h"
using namespace std;

void Sheet::SetCell(Position pos, std::string text)
{
    if(!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    if(cells_.count(pos) == 0) {
       cells_.emplace(pos, std::make_unique<Cell>(*this));
    }
    cells_.at(pos)->Set(text);
}

const CellInterface *Sheet::GetCell(Position pos) const
{
    if(!pos.IsValid()) {
       throw InvalidPositionException("Invalid position");
    }
    if ( cells_.count(pos) == 0) {
       return nullptr;
    }
    return cells_.at(pos).get();
}

CellInterface *Sheet::GetCell(Position pos)
{
    if(!pos.IsValid()) {
       throw InvalidPositionException("Invalid position");
    }

    if ( cells_.count(pos) == 0) {
        return nullptr;
    }
    return cells_.at(pos).get();
}

void Sheet::ClearCell(Position pos)
{
    if(!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    if( auto cell = GetCell(pos); cell != nullptr) {
        cells_.erase(pos);
    }
}

Size Sheet::GetPrintableSize() const
{
    Size result {0,0};
    for(const auto& [position, cell ] : cells_) {
        if (cell != nullptr) {
        result.rows = std::max(result.rows, position.row + 1);
        result.cols = std::max(result.cols, position.col + 1);
        }
    }
    return {result.rows,result.cols};
}

void Sheet::PrintValues(std::ostream &output) const
{
    auto size = GetPrintableSize();

    for (auto row = 0; row < size.rows; ++row) {
        for (auto col = 0 ; col < size.cols; ++col) {
        auto it = cells_.find({ row, col });
        if (col > 0) {
            output << '\t';
        }
        if (it != cells_.end()) {
        std::visit([&](const auto value) {output << value; }, it->second->GetValue() );
        }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const
{
    auto size = GetPrintableSize();

    for (auto row = 0; row < size.rows; ++row) {
        for (auto col = 0; col < size.cols; ++col) {
            auto it = cells_.find({ row, col });
            if (col > 0) {
                output << '\t';
            }
            if (it != cells_.end()) {
                output << it->second->GetText() ;
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}


