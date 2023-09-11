#pragma once

#include "common.h"
#include <iostream>
#include <functional>
#include "cell.h"

template <>
struct std::hash<Position> {
    std::size_t operator()(const Position& pos) const {
        std::hash<int> hasher;
        auto rowHash = hasher(pos.row);
        auto colHash = hasher(pos.col);
        return rowHash ^ colHash;
    }
};

class Sheet : public SheetInterface {
public:
    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface *GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	// Можете дополнить ваш класс нужными полями и методами


private:
    std::unordered_map<Position, std::unique_ptr<Cell>> cells_;

	// Можете дополнить ваш класс нужными полями и методами
};
