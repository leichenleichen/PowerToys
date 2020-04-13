#include "pch.h"

#include "util.h"
#include "lib/ZoneSet.h"
#include "lib/RegistryHelpers.h"

#include <common/dpi_aware.h>

namespace
{
    constexpr int C_MULTIPLIER = 10000;

    /*
      struct GridLayoutInfo {
        int rows;
        int columns;
        int rowsPercents[MAX_ZONE_COUNT];
        int columnsPercents[MAX_ZONE_COUNT];
        int cellChildMap[MAX_ZONE_COUNT][MAX_ZONE_COUNT];
      };
    */

    auto l = JSONHelpers::GridLayoutInfo(JSONHelpers::GridLayoutInfo::Minimal{ .rows = 1, .columns = 1 });
    // PriorityGrid layout is unique for zoneCount <= 11. For zoneCount > 11 PriorityGrid is same as Grid
    JSONHelpers::GridLayoutInfo predefinedPriorityGridLayouts[11] = {
        /* 1 */
        JSONHelpers::GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
            .rows = 1,
            .columns = 1,
            .rowsPercents = { 10000 },
            .columnsPercents = { 10000 },
            .cellChildMap = { { 0 } } }),
        /* 2 */
        JSONHelpers::GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
            .rows = 1,
            .columns = 2,
            .rowsPercents = { 10000 },
            .columnsPercents = { 6667, 3333 },
            .cellChildMap = { { 0, 1 } } }),
        /* 3 */
        JSONHelpers::GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
            .rows = 1,
            .columns = 3,
            .rowsPercents = { 10000 },
            .columnsPercents = { 2500, 5000, 2500 },
            .cellChildMap = { { 0, 1, 2 } } }),
        /* 4 */
        JSONHelpers::GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
            .rows = 2,
            .columns = 3,
            .rowsPercents = { 5000, 5000 },
            .columnsPercents = { 2500, 5000, 2500 },
            .cellChildMap = { { 0, 1, 2 }, { 0, 1, 3 } } }),
        /* 5 */
        JSONHelpers::GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
            .rows = 2,
            .columns = 3,
            .rowsPercents = { 5000, 5000 },
            .columnsPercents = { 2500, 5000, 2500 },
            .cellChildMap = { { 0, 1, 2 }, { 3, 1, 4 } } }),
        /* 6 */
        JSONHelpers::GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
            .rows = 3,
            .columns = 3,
            .rowsPercents = { 3333, 3334, 3333 },
            .columnsPercents = { 2500, 5000, 2500 },
            .cellChildMap = { { 0, 1, 2 }, { 0, 1, 3 }, { 4, 1, 5 } } }),
        /* 7 */
        JSONHelpers::GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
            .rows = 3,
            .columns = 3,
            .rowsPercents = { 3333, 3334, 3333 },
            .columnsPercents = { 2500, 5000, 2500 },
            .cellChildMap = { { 0, 1, 2 }, { 3, 1, 4 }, { 5, 1, 6 } } }),
        /* 8 */
        JSONHelpers::GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
            .rows = 3,
            .columns = 4,
            .rowsPercents = { 3333, 3334, 3333 },
            .columnsPercents = { 2500, 2500, 2500, 2500 },
            .cellChildMap = { { 0, 1, 2, 3 }, { 4, 1, 2, 5 }, { 6, 1, 2, 7 } } }),
        /* 9 */
        JSONHelpers::GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
            .rows = 3,
            .columns = 4,
            .rowsPercents = { 3333, 3334, 3333 },
            .columnsPercents = { 2500, 2500, 2500, 2500 },
            .cellChildMap = { { 0, 1, 2, 3 }, { 4, 1, 2, 5 }, { 6, 1, 7, 8 } } }),
        /* 10 */
        JSONHelpers::GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
            .rows = 3,
            .columns = 4,
            .rowsPercents = { 3333, 3334, 3333 },
            .columnsPercents = { 2500, 2500, 2500, 2500 },
            .cellChildMap = { { 0, 1, 2, 3 }, { 4, 1, 5, 6 }, { 7, 1, 8, 9 } } }),
        /* 11 */
        JSONHelpers::GridLayoutInfo(JSONHelpers::GridLayoutInfo::Full{
            .rows = 3,
            .columns = 4,
            .rowsPercents = { 3333, 3334, 3333 },
            .columnsPercents = { 2500, 2500, 2500, 2500 },
            .cellChildMap = { { 0, 1, 2, 3 }, { 4, 1, 5, 6 }, { 7, 8, 9, 10 } } }),
    };
}

struct ZoneSet : winrt::implements<ZoneSet, IZoneSet>
{
public:
    ZoneSet(ZoneSetConfig const& config) :
        m_config(config)
    {
    }

    ZoneSet(ZoneSetConfig const& config, std::vector<winrt::com_ptr<IZone>> zones) :
        m_config(config),
        m_zones(zones)
    {
    }

    IFACEMETHODIMP_(GUID)
    Id() noexcept { return m_config.Id; }
    IFACEMETHODIMP_(JSONHelpers::ZoneSetLayoutType)
    LayoutType() noexcept { return m_config.LayoutType; }
    IFACEMETHODIMP AddZone(winrt::com_ptr<IZone> zone) noexcept;
    IFACEMETHODIMP_(std::vector<int>)
    ZonesFromPoint(POINT pt) noexcept;
    IFACEMETHODIMP_(int)
    GetZoneIndexFromWindow(HWND window) noexcept;
    IFACEMETHODIMP_(std::vector<winrt::com_ptr<IZone>>)
    GetZones() noexcept { return m_zones; }
    IFACEMETHODIMP_(void)
    MoveWindowIntoZoneByIndex(HWND window, HWND zoneWindow, int index, bool stampZone) noexcept;
    IFACEMETHODIMP_(void)
    MoveWindowIntoZoneByIndexSet(HWND window, HWND windowZone, const std::vector<int>& indexSet, bool stampZone) noexcept;
    IFACEMETHODIMP_(bool)
    MoveWindowIntoZoneByDirection(HWND window, HWND zoneWindow, DWORD vkCode, bool cycle) noexcept;
    IFACEMETHODIMP_(void)
    MoveWindowIntoZoneByPoint(HWND window, HWND zoneWindow, POINT ptClient) noexcept;
    IFACEMETHODIMP_(bool)
    CalculateZones(MONITORINFO monitorInfo, int zoneCount, int spacing) noexcept;

private:
    bool CalculateFocusLayout(Rect workArea, int zoneCount) noexcept;
    bool CalculateColumnsAndRowsLayout(Rect workArea, JSONHelpers::ZoneSetLayoutType type, int zoneCount, int spacing) noexcept;
    bool CalculateGridLayout(Rect workArea, JSONHelpers::ZoneSetLayoutType type, int zoneCount, int spacing) noexcept;
    bool CalculateUniquePriorityGridLayout(Rect workArea, int zoneCount, int spacing) noexcept;
    bool CalculateCustomLayout(Rect workArea, int spacing) noexcept;

    bool CalculateGridZones(Rect workArea, JSONHelpers::GridLayoutInfo gridLayoutInfo, int spacing);

    winrt::com_ptr<IZone> ZoneFromWindow(HWND window) noexcept;

    std::vector<winrt::com_ptr<IZone>> m_zones;
    ZoneSetConfig m_config;
};

IFACEMETHODIMP ZoneSet::AddZone(winrt::com_ptr<IZone> zone) noexcept
{
    m_zones.emplace_back(zone);

    // Important not to set Id 0 since we store it in the HWND using SetProp.
    // SetProp(0) doesn't really work.
    zone->SetId(m_zones.size());
    return S_OK;
}

IFACEMETHODIMP_(std::vector<int>)
ZoneSet::ZonesFromPoint(POINT pt) noexcept
{
    const int SENSITIVITY_RADIUS = 20;
    std::vector<int> capturedZones;
    std::vector<int> strictlyCapturedZones;
    for (size_t i = 0; i < m_zones.size(); i++)
    {
        auto zone = m_zones[i];
        RECT newZoneRect = zone->GetZoneRect();
        if (newZoneRect.left < newZoneRect.right && newZoneRect.top < newZoneRect.bottom) // proper zone
        {
            if (newZoneRect.left - SENSITIVITY_RADIUS <= pt.x && pt.x <= newZoneRect.right + SENSITIVITY_RADIUS &&
                newZoneRect.top - SENSITIVITY_RADIUS <= pt.y && pt.y <= newZoneRect.bottom + SENSITIVITY_RADIUS)
            {
                capturedZones.emplace_back(static_cast<int>(i));
            }
            
            if (newZoneRect.left <= pt.x && pt.x < newZoneRect.right &&
                newZoneRect.top <= pt.y && pt.y < newZoneRect.bottom)
            {
                strictlyCapturedZones.emplace_back(static_cast<int>(i));
            }
        }
    }

    // If only one zone is captured, but it's not strictly captured
    // don't consider it as captured
    if (capturedZones.size() == 1 && strictlyCapturedZones.size() == 0)
    {
        return {};
    }

    // If captured zones do not overlap, return all of them
    // Otherwise, return the smallest one

    bool overlap = false;
    for (size_t i = 0; i < capturedZones.size(); ++i)
    {
        for (size_t j = i + 1; j < capturedZones.size(); ++j)
        {
            auto rectI = m_zones[capturedZones[i]]->GetZoneRect();
            auto rectJ = m_zones[capturedZones[j]]->GetZoneRect();
            if (max(rectI.top, rectJ.top) < min(rectI.bottom, rectJ.bottom) &&
                max(rectI.left, rectJ.left) < min(rectI.right, rectJ.right))
            {
                overlap = true;
                i = capturedZones.size() - 1;
                break;
            }
        }
    }

    if (overlap)
    {
        size_t smallestIdx = 0;
        for (size_t i = 1; i < capturedZones.size(); ++i)
        {
            auto rectS = m_zones[capturedZones[smallestIdx]]->GetZoneRect();
            auto rectI = m_zones[capturedZones[i]]->GetZoneRect();
            int smallestSize = (rectS.bottom - rectS.top) * (rectS.right - rectS.left);
            int iSize = (rectI.bottom - rectI.top) * (rectI.right - rectI.left);

            if (iSize <= smallestSize)
            {
                smallestIdx = i;
            }
        }

        capturedZones = { capturedZones[smallestIdx] };
    }

    return capturedZones;
}

IFACEMETHODIMP_(int)
ZoneSet::GetZoneIndexFromWindow(HWND window) noexcept
{
    int zoneIndex = 0;
    for (auto iter = m_zones.begin(); iter != m_zones.end(); iter++, zoneIndex++)
    {
        if (winrt::com_ptr<IZone> zone = iter->try_as<IZone>())
        {
            if (zone->ContainsWindow(window))
            {
                return zoneIndex;
            }
        }
    }
    return -1;
}

IFACEMETHODIMP_(void)
ZoneSet::MoveWindowIntoZoneByIndex(HWND window, HWND windowZone, int index, bool stampZone) noexcept
{
    if (m_zones.empty())
    {
        return;
    }

    if (index >= int(m_zones.size()))
    {
        return;
    }

    while (auto zoneDrop = ZoneFromWindow(window))
    {
        zoneDrop->RemoveWindowFromZone(window, !IsZoomed(window));
    }

    if (auto zone = m_zones.at(index))
    {
        zone->AddWindowToZone(window, windowZone, stampZone);
    }
}

IFACEMETHODIMP_(void)
ZoneSet::MoveWindowIntoZoneByIndexSet(HWND window, HWND windowZone, const std::vector<int>& indexSet, bool stampZone) noexcept
{
    if (m_zones.empty())
    {
        return;
    }

    while (auto zoneDrop = ZoneFromWindow(window))
    {
        zoneDrop->RemoveWindowFromZone(window, !IsZoomed(window));
    }

    if (indexSet.size() == 1)
    {
        MoveWindowIntoZoneByIndex(window, windowZone, indexSet[0], stampZone);
        return;
    }

    RECT size;
    bool sizeEmpty = true;

    for (int index : indexSet)
    {
        if (index < static_cast<int>(m_zones.size()))
        {
            RECT newSize = m_zones.at(index)->ComputeActualZoneRect(window, windowZone);
            if (!sizeEmpty)
            {
                size.left = min(size.left, newSize.left);
                size.top = min(size.top, newSize.top);
                size.right = max(size.right, newSize.right);
                size.bottom = max(size.bottom, newSize.bottom);
            }
            else
            {
                size = newSize;
                sizeEmpty = false;
            }
        }
    }

    if (!sizeEmpty)
    {
        SizeWindowToRect(window, size);
    }
}

IFACEMETHODIMP_(bool)
ZoneSet::MoveWindowIntoZoneByDirection(HWND window, HWND windowZone, DWORD vkCode, bool cycle) noexcept
{
    if (m_zones.empty())
    {
        return false;
    }

    winrt::com_ptr<IZone> oldZone = nullptr;
    winrt::com_ptr<IZone> newZone = nullptr;

    auto iter = std::find(m_zones.begin(), m_zones.end(), ZoneFromWindow(window));
    if (iter == m_zones.end())
    {
        iter = (vkCode == VK_RIGHT) ? m_zones.begin() : m_zones.end() - 1;
    }
    else if (oldZone = iter->as<IZone>())
    {
        if (vkCode == VK_LEFT)
        {
            if (iter == m_zones.begin())
            {
                if (!cycle)
                {
                    oldZone->RemoveWindowFromZone(window, false);
                    return false;
                }
                iter = m_zones.end();
            }
            iter--;
        }
        else if (vkCode == VK_RIGHT)
        {
            iter++;
            if (iter == m_zones.end())
            {
                if (!cycle)
                {
                    oldZone->RemoveWindowFromZone(window, false);
                    return false;
                }
                iter = m_zones.begin();
            }
        }
    }

    if (newZone = iter->as<IZone>())
    {
        if (oldZone)
        {
            oldZone->RemoveWindowFromZone(window, false);
        }
        newZone->AddWindowToZone(window, windowZone, true);
        return true;
    }
    return false;
}

IFACEMETHODIMP_(void)
ZoneSet::MoveWindowIntoZoneByPoint(HWND window, HWND zoneWindow, POINT ptClient) noexcept
{
    while (auto zoneDrop = ZoneFromWindow(window))
    {
        zoneDrop->RemoveWindowFromZone(window, !IsZoomed(window));
    }

    auto zones = ZonesFromPoint(ptClient);
    MoveWindowIntoZoneByIndexSet(window, zoneWindow, zones, true);
}

IFACEMETHODIMP_(bool)
ZoneSet::CalculateZones(MONITORINFO monitorInfo, int zoneCount, int spacing) noexcept
{
    Rect const workArea(monitorInfo.rcWork);
    //invalid work area
    if (workArea.width() == 0 || workArea.height() == 0)
    {
        return false;
    }

    //invalid zoneCount, may cause division by zero
    if (zoneCount <= 0 && m_config.LayoutType != JSONHelpers::ZoneSetLayoutType::Custom)
    {
        return false;
    }

    bool success = true;
    switch (m_config.LayoutType)
    {
    case JSONHelpers::ZoneSetLayoutType::Focus:
        success = CalculateFocusLayout(workArea, zoneCount);
        break;
    case JSONHelpers::ZoneSetLayoutType::Columns:
    case JSONHelpers::ZoneSetLayoutType::Rows:
        success = CalculateColumnsAndRowsLayout(workArea, m_config.LayoutType, zoneCount, spacing);
        break;
    case JSONHelpers::ZoneSetLayoutType::Grid:
    case JSONHelpers::ZoneSetLayoutType::PriorityGrid:
        success = CalculateGridLayout(workArea, m_config.LayoutType, zoneCount, spacing);
        break;
    case JSONHelpers::ZoneSetLayoutType::Custom:
        success = CalculateCustomLayout(workArea, spacing);
        break;
    }

    return success;
}

bool ZoneSet::CalculateFocusLayout(Rect workArea, int zoneCount) noexcept
{
    bool success = true;

    long left{ long(workArea.width() * 0.1) };
    long top{ long(workArea.height() * 0.1) };
    long right{ long(workArea.width() * 0.6) };
    long bottom{ long(workArea.height() * 0.6) };

    RECT focusZoneRect{ left, top, right, bottom };

    long focusRectXIncrement = (zoneCount <= 1) ? 0 : (int)(workArea.width() * 0.2) / (zoneCount - 1);
    long focusRectYIncrement = (zoneCount <= 1) ? 0 : (int)(workArea.height() * 0.2) / (zoneCount - 1);

    if (left >= right || top >= bottom || left < 0 || right < 0 || top < 0 || bottom < 0)
    {
        success = false;
    }

    for (int i = 0; i < zoneCount; i++)
    {
        AddZone(MakeZone(focusZoneRect));
        focusZoneRect.left += focusRectXIncrement;
        focusZoneRect.right += focusRectXIncrement;
        focusZoneRect.bottom += focusRectYIncrement;
        focusZoneRect.top += focusRectYIncrement;
    }

    return success;
}

bool ZoneSet::CalculateColumnsAndRowsLayout(Rect workArea, JSONHelpers::ZoneSetLayoutType type, int zoneCount, int spacing) noexcept
{
    bool success = true;

    long totalWidth;
    long totalHeight;

    if (type == JSONHelpers::ZoneSetLayoutType::Columns)
    {
        totalWidth = workArea.width() - (spacing * (zoneCount + 1));
        totalHeight = workArea.height() - (spacing * 2);
    }
    else
    { //Rows
        totalWidth = workArea.width() - (spacing * 2);
        totalHeight = workArea.height() - (spacing * (zoneCount + 1));
    }

    long top = spacing;
    long left = spacing;
    long bottom;
    long right;

    // Note: The expressions below are NOT equal to total{Width|Height} / zoneCount and are done
    // like this to make the sum of all zones' sizes exactly total{Width|Height}.
    for (int zone = 0; zone < zoneCount; zone++)
    {
        if (type == JSONHelpers::ZoneSetLayoutType::Columns)
        {
            right = left + (zone + 1) * totalWidth / zoneCount - zone * totalWidth / zoneCount;
            bottom = totalHeight + spacing;
        }
        else
        { //Rows
            right = totalWidth + spacing;
            bottom = top + (zone + 1) * totalHeight / zoneCount - zone * totalHeight / zoneCount;
        }
        
        if (left >= right || top >= bottom || left < 0 || right < 0 || top < 0 || bottom < 0)
        {
            success = false;
        }

        RECT focusZoneRect{ left, top, right, bottom };
        AddZone(MakeZone(focusZoneRect));

        if (type == JSONHelpers::ZoneSetLayoutType::Columns)
        {
            left = right + spacing;
        }
        else
        { //Rows
            top = bottom + spacing;
        }
    }

    return success;
}

bool ZoneSet::CalculateGridLayout(Rect workArea, JSONHelpers::ZoneSetLayoutType type, int zoneCount, int spacing) noexcept
{
    const auto count = sizeof(predefinedPriorityGridLayouts) / sizeof(JSONHelpers::GridLayoutInfo);
    if (type == JSONHelpers::ZoneSetLayoutType::PriorityGrid && zoneCount < count)
    {
        return CalculateUniquePriorityGridLayout(workArea, zoneCount, spacing);
    }

    int rows = 1, columns = 1;
    while (zoneCount / rows >= rows)
    {
        rows++;
    }
    rows--;
    columns = zoneCount / rows;
    if (zoneCount % rows == 0)
    {
        // even grid
    }
    else
    {
        columns++;
    }

    JSONHelpers::GridLayoutInfo gridLayoutInfo(JSONHelpers::GridLayoutInfo::Minimal{ .rows = rows, .columns = columns });

    // Note: The expressions below are NOT equal to C_MULTIPLIER / {rows|columns} and are done
    // like this to make the sum of all percents exactly C_MULTIPLIER
    for (int row = 0; row < rows; row++)
    {
        gridLayoutInfo.rowsPercents()[row] = C_MULTIPLIER * (row + 1) / rows - C_MULTIPLIER * row / rows;
    }
    for (int col = 0; col < columns; col++)
    {
        gridLayoutInfo.columnsPercents()[col] = C_MULTIPLIER * (col + 1) / columns - C_MULTIPLIER * col / columns;
    }

    for (int i = 0; i < rows; ++i)
    {
        gridLayoutInfo.cellChildMap()[i] = std::vector<int>(columns);
    }

    int index = 0;
    for (int col = columns - 1; col >= 0; col--)
    {
        for (int row = rows - 1; row >= 0; row--)
        {
            gridLayoutInfo.cellChildMap()[row][col] = index++;
            if (index == zoneCount)
            {
                index--;
            }
        }
    }
    return CalculateGridZones(workArea, gridLayoutInfo, spacing);
}

bool ZoneSet::CalculateUniquePriorityGridLayout(Rect workArea, int zoneCount, int spacing) noexcept
{
    if (zoneCount <= 0 || zoneCount >= sizeof(predefinedPriorityGridLayouts))
    {
        return false;
    }

    return CalculateGridZones(workArea, predefinedPriorityGridLayouts[zoneCount - 1], spacing);
}

bool ZoneSet::CalculateCustomLayout(Rect workArea, int spacing) noexcept
{
    wil::unique_cotaskmem_string guuidStr;
    if (SUCCEEDED_LOG(StringFromCLSID(m_config.Id, &guuidStr)))
    {
        const std::wstring guuid = guuidStr.get();

        const auto zoneSetSearchResult = JSONHelpers::FancyZonesDataInstance().FindCustomZoneSet(guuid);

        if (!zoneSetSearchResult.has_value())
        {
            return false;
        }

        const auto& zoneSet = *zoneSetSearchResult;
        if (zoneSet.type == JSONHelpers::CustomLayoutType::Canvas && std::holds_alternative<JSONHelpers::CanvasLayoutInfo>(zoneSet.info))
        {
            const auto& zoneSetInfo = std::get<JSONHelpers::CanvasLayoutInfo>(zoneSet.info);
            for (const auto& zone : zoneSetInfo.zones)
            {
                int x = zone.x;
                int y = zone.y;
                int width = zone.width;
                int height = zone.height;

                if (x < 0 || y < 0 || width < 0 || height < 0)
                {
                    return false;
                }

                DPIAware::Convert(m_config.Monitor, x, y);
                DPIAware::Convert(m_config.Monitor, width, height);

                AddZone(MakeZone(RECT{ x, y, x + width, y + height }));
            }

            return true;
        }
        else if (zoneSet.type == JSONHelpers::CustomLayoutType::Grid && std::holds_alternative<JSONHelpers::GridLayoutInfo>(zoneSet.info))
        {
            const auto& info = std::get<JSONHelpers::GridLayoutInfo>(zoneSet.info);
            return CalculateGridZones(workArea, info, spacing);
        }
    }

    return false;
}

bool ZoneSet::CalculateGridZones(Rect workArea, JSONHelpers::GridLayoutInfo gridLayoutInfo, int spacing)
{
    bool success = true;

    long totalWidth = workArea.width() - (spacing * (gridLayoutInfo.columns() + 1));
    long totalHeight = workArea.height() - (spacing * (gridLayoutInfo.rows() + 1));
    struct Info
    {
        long Extent;
        long Start;
        long End;
    };
    std::vector<Info> rowInfo(gridLayoutInfo.rows());
    std::vector<Info> columnInfo(gridLayoutInfo.columns());

    // Note: The expressions below are carefully written to 
    // make the sum of all zones' sizes exactly total{Width|Height}
    int totalPercents = 0;
    for (int row = 0; row < gridLayoutInfo.rows(); row++)
    {
        rowInfo[row].Start = totalPercents * totalHeight / C_MULTIPLIER + (row + 1) * spacing;
        totalPercents += gridLayoutInfo.rowsPercents()[row];
        rowInfo[row].End = totalPercents * totalHeight / C_MULTIPLIER + (row + 1) * spacing;
        rowInfo[row].Extent = rowInfo[row].End - rowInfo[row].Start;
    }

    totalPercents = 0;
    for (int col = 0; col < gridLayoutInfo.columns(); col++)
    {
        columnInfo[col].Start = totalPercents * totalWidth / C_MULTIPLIER + (col + 1) * spacing;
        totalPercents += gridLayoutInfo.columnsPercents()[col];
        columnInfo[col].End = totalPercents * totalWidth / C_MULTIPLIER + (col + 1) * spacing;
        columnInfo[col].Extent = columnInfo[col].End - columnInfo[col].Start;
    }

    for (int row = 0; row < gridLayoutInfo.rows(); row++)
    {
        for (int col = 0; col < gridLayoutInfo.columns(); col++)
        {
            int i = gridLayoutInfo.cellChildMap()[row][col];
            if (((row == 0) || (gridLayoutInfo.cellChildMap()[row - 1][col] != i)) &&
                ((col == 0) || (gridLayoutInfo.cellChildMap()[row][col - 1] != i)))
            {
                long left = columnInfo[col].Start;
                long top = rowInfo[row].Start;

                int maxRow = row;
                while (((maxRow + 1) < gridLayoutInfo.rows()) && (gridLayoutInfo.cellChildMap()[maxRow + 1][col] == i))
                {
                    maxRow++;
                }
                int maxCol = col;
                while (((maxCol + 1) < gridLayoutInfo.columns()) && (gridLayoutInfo.cellChildMap()[row][maxCol + 1] == i))
                {
                    maxCol++;
                }

                long right = columnInfo[maxCol].End;
                long bottom = rowInfo[maxRow].End;

                if (left >= right || top >= bottom || left < 0 || right < 0 || top < 0 || bottom < 0)
                {
                    success = false;
                }

                AddZone(MakeZone(RECT{ left, top, right, bottom }));
            }
        }
    }

    return success;
}

winrt::com_ptr<IZone> ZoneSet::ZoneFromWindow(HWND window) noexcept
{
    for (auto iter = m_zones.begin(); iter != m_zones.end(); iter++)
    {
        if (winrt::com_ptr<IZone> zone = iter->try_as<IZone>())
        {
            if (zone->ContainsWindow(window))
            {
                return zone;
            }
        }
    }
    return nullptr;
}

winrt::com_ptr<IZoneSet> MakeZoneSet(ZoneSetConfig const& config) noexcept
{
    return winrt::make_self<ZoneSet>(config);
}
