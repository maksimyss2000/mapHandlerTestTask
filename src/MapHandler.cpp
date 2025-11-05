#include "MapHandler.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <iomanip>


namespace 
{

constexpr int MIN_LENGHT_NAME = 3;
constexpr int MAX_LENGHT_NAME = 8;
constexpr int MAX_ATTEMPTS_GENERATE = 100;
constexpr int MIN_HOUSE_WIDTH = 4;
constexpr int MIN_HOUSE_HEIGHT= 4;

std::string generateRandomString() 
{  
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static const std::string letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static std::uniform_int_distribution<> lenDist(MIN_LENGHT_NAME - 2, MAX_LENGHT_NAME - 2);  
    static std::uniform_int_distribution<> charDist(0, letters.size() - 1);
    
    int length = lenDist(gen);
    std::string result;
    for (int i = 0; i < length; ++i) 
    {
        result += letters[charDist(gen)];
    }
    return result;
}

// Вычисление геометрического центра дома
PointF calculateHouseCenter(int minX, int minY, int width, int height) 
{
    PointF center;
    center.x = minX + (width - 1) / 2.0;
    center.y = minY + (height - 1) / 2.0;
    return center;
}

// Вычисление геометрического центра станции
PointF calculateStationCenter(int x, int y) 
{
    PointF center;
    // Станция занимает одну ячейку, поэтому ее центр - середина этой ячейки
    center.x = x + 0.5;
    center.y = y + 0.5;
    return center;
}

// Вычисление расстояния между двумя вещественными точками
double calculateDistance(const PointF& p1, const PointF& p2) 
{
    return std::sqrt(std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2));
}

} // anonymous namespace

std::string MapHandler::generateUniqueHouseName() const
{    
    for (int attempt = 0; attempt < MAX_ATTEMPTS_GENERATE; ++attempt) 
    {
        std::string name = "H_" + generateRandomString();
        
        bool isUnique = std::none_of(houses.begin(), houses.end(),
            [&name](const House& house) { return house.name == name; }) &&
                       std::none_of(stations.begin(), stations.end(),
            [&name](const Station& station) { return station.name == name; });
        
        if (isUnique) 
        {
            return name;
        }
    }
    
    throw std::runtime_error("Не удалось сгенерировать уникальное имя для дома");
}

std::string MapHandler::generateUniqueStationName() const
{    
    for (int attempt = 0; attempt < MAX_ATTEMPTS_GENERATE; ++attempt) 
    {
        std::string name = "S_" + generateRandomString();
        
        bool isUnique = std::none_of(houses.begin(), houses.end(),
            [&name](const House& house) { return house.name == name; }) &&
                       std::none_of(stations.begin(), stations.end(),
            [&name](const Station& station) { return station.name == name; });
        
        if (isUnique) 
        {
            return name;
        }
    }
    
    throw std::runtime_error("Не удалось сгенерировать уникальное имя для станции");
}

std::vector<Point> MapHandler::getHousePixels(int startX, int startY, std::vector<std::vector<bool>>& visited) const
{
    std::vector<Point> pixels;
    std::vector<Point> stack;
    
    stack.push_back(Point{startX, startY});
    visited[startY][startX] = true;
    
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    
    while (!stack.empty()) 
    {
        Point p = stack.back();
        stack.pop_back();
        pixels.push_back(p);
        
        for (int i = 0; i < 4; ++i) 
        {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];
            
            if (nx >= 0 && nx < static_cast<int>(width) && ny >= 0 && ny < static_cast<int>(height) &&
                !visited[ny][nx] && mapData[ny][nx] == CellType::HOUSE) 
            {
                visited[ny][nx] = true;
                stack.push_back(Point{nx, ny});
            }
        }
    }
    
    return pixels;
}

House MapHandler::getHouseFromPixels(const std::vector<Point>& pixels) const
{
    if (pixels.empty()) 
    {
        throw std::invalid_argument("Пустой набор пикселей для анализа дома");
    }

    int minX = pixels[0].x, maxX = pixels[0].x;
    int minY = pixels[0].y, maxY = pixels[0].y;
    
    for (const auto& p : pixels) 
    {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }
    
    int houseWidth = maxX - minX + 1;
    int houseHeight = maxY - minY + 1;
    if (houseWidth < MIN_HOUSE_WIDTH || houseHeight < MIN_HOUSE_HEIGHT) 
    {
        throw std::invalid_argument(
            "Дом не соответствует требованиям: размер " + std::to_string(houseWidth) + "x" + 
            std::to_string(houseHeight) + ", минимальный размер " + 
            std::to_string(MIN_HOUSE_WIDTH) + "x" + std::to_string(MIN_HOUSE_HEIGHT)
        );
    }
    
    int expectedPixels = houseWidth * houseHeight;
    if (expectedPixels != static_cast<int>(pixels.size())) 
    {
        throw std::invalid_argument(
            "Область не является прямоугольником. Ожидалось " + 
            std::to_string(expectedPixels) + " пикселей, но найдено " + 
            std::to_string(pixels.size())
        );
    }
    
    for (int y = minY; y <= maxY; ++y) 
    {
        for (int x = minX; x <= maxX; ++x) 
        {
            if (mapData[y][x] != CellType::HOUSE) 
            {
                throw std::invalid_argument(
                    "Обнаружена дыра в прямоугольнике в позиции (" + 
                    std::to_string(x) + "," + std::to_string(y) + ")"
                );
            }
        }
    }
    
    Point position{minX, minY};  // Левая верхняя координата
    PointF center = calculateHouseCenter(minX, minY, houseWidth, houseHeight);  // Геометрический центр
    std::string name = generateUniqueHouseName();
    
    return House{name, position, center, houseWidth, houseHeight};
}

void MapHandler::findHouses() 
{
    if (mapData.empty()) 
    {
        throw std::runtime_error("Данные карты не загружены");
    }
    
    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
    
    for (uint32_t y = 0; y < height; ++y) 
    {
        for (uint32_t x = 0; x < width; ++x) 
        {
            if (mapData[y][x] == CellType::HOUSE && !visited[y][x]) 
            {
                std::vector<Point> housePixels = getHousePixels(x, y, visited);
                
                try 
                {
                    House house = getHouseFromPixels(housePixels);
                    houses.push_back(house);
                } 
                catch (const std::invalid_argument& e) 
                {
                    std::cout << "Пропущен невалидный дом: " << e.what() << std::endl;
                }
            }
        }
    }
}

void MapHandler::findStations() 
{
    if (mapData.empty()) 
    {
        throw std::runtime_error("Данные карты не загружены");
    }
    
    for (int y = 0; y < static_cast<int>(height); ++y) 
    {
        for (int x = 0; x < static_cast<int>(width); ++x) 
        {
            if (mapData[y][x] == CellType::STATION) 
            {
                std::string name = generateUniqueStationName();
                Point position{x, y};  // Целая позиция для вывода
                PointF center = calculateStationCenter(x, y);  // Геометрический центр для вычислений
                stations.push_back(Station{name, position, center}); 
            }
        }
    }
}

void MapHandler::connectHousesToStations() 
{
    if (houses.empty()) 
    {
        throw std::runtime_error("Нет домов для подключения");
    }
    
    if (stations.empty()) 
    {
        throw std::runtime_error("Нет станций для подключения домов");
    }
    
    for (auto& house : houses) 
    {
        std::string closestStation;
        double minDistance = std::numeric_limits<double>::max();
        
        for (auto& station : stations) 
        {
            double dist = calculateDistance(house.center, station.center);
            if (dist < minDistance) 
            {
                minDistance = dist;
                closestStation = station.name;
            }
        }
        
        if (!closestStation.empty()) 
        {
            connections.push_back(Connection{house.name, closestStation, minDistance});
        } 
        else 
        {
            throw std::runtime_error("Не удалось найти станцию для дома " + house.name);
        }
    }
}

void MapHandler::readMapData(const std::string& filename) 
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) 
    {
        throw std::runtime_error("Ошибка открытия файла: " + filename);
    }
    
    file.read(reinterpret_cast<char*>(&width), sizeof(width));
    if (file.fail()) 
    {
        throw std::runtime_error("Ошибка чтения ширины карты из файла: " + filename);
    }
    
    file.read(reinterpret_cast<char*>(&height), sizeof(height));
    if (file.fail()) 
    {
        throw std::runtime_error("Ошибка чтения высоты карты из файла: " + filename);
    }
    
    if (width == 0 || height == 0) 
    {
        throw std::runtime_error("Неверные размеры карты: " + std::to_string(width) + "x" + std::to_string(height));
    }
    
    mapData.resize(height, std::vector<CellType>(width));
    for (uint32_t y = 0; y < height; ++y) 
    {
        for (uint32_t x = 0; x < width; ++x) 
        {
            uint8_t rawValue;
            file.read(reinterpret_cast<char*>(&rawValue), sizeof(rawValue));
            if (file.fail()) 
            {
                throw std::runtime_error("Ошибка чтения данных карты в позиции (" + std::to_string(x) + "," + std::to_string(y) + ")");
            }
            mapData[y][x] = static_cast<CellType>(rawValue);
        }
    }
    
    file.close();
}

void MapHandler::handleObjects() 
{
    if (mapData.empty()) 
    {
        throw std::runtime_error("Данные карты не загружены");
    }
    
    houses.clear();
    stations.clear();
    
    findHouses();
    findStations();
}

void MapHandler::printObjects() const
{
    std::cout << "Дома: позиция - левый верхний угол (x, y), геометрический центр (x, y), размер [WxH]" << std::endl;
    for (const auto& house : houses) 
    {
        // Выводим левую верхнюю координату и геометрический центр
        std::cout << house.name << " " 
                  << "(" << house.position.x << ", " << house.position.y << ") "
                  << "(" << std::fixed << std::setprecision(1) 
                  << house.center.x << ", " << house.center.y << ") "
                  << "[" << house.width << "x" << house.height << "]" << std::endl;
    }
    
    
    std::cout << "\nСтанции: позиция (x, y)" << std::endl;
    for (const auto& station : stations) 
    {
        std::cout << station.name << " (" << station.position.x << ", " << station.position.y << ")" << std::endl;
    }

    std::cout << std::endl;
}

void MapHandler::groupSortedConnectionsByStation() 
{
    sortedConnectionsByStation.clear();

    for (const auto& conn : connections) 
    {
        sortedConnectionsByStation[conn.stationName].push_back(conn);
    }
    
    for (auto& [station, conns] : sortedConnectionsByStation)
    {
        std::sort(conns.begin(), conns.end(),
            [](const Connection& a, const Connection& b) {
                return a.distance < b.distance;
            });
    }
}


void MapHandler::handleConnections() 
{
    if (houses.empty()) 
    {
        std::cout << "Нет домов для подключения" << std::endl;
        return;
    }
    
    if (stations.empty()) 
    {
        std::cout << "Нет станций для подключения домов" << std::endl;
        return;
    }

    connections.clear();
    connectHousesToStations();
    groupSortedConnectionsByStation();
}

void MapHandler::printConnections() const
{
    if (connections.empty()) 
    {
        std::cout << "Нет подключений для вывода" << std::endl;
        return;
    }
    
    for (const auto& [stationName, stationConns] : sortedConnectionsByStation) 
    {
        std::cout << "\nСтанция '" << stationName << "' подключена к:" << std::endl;
        
        for (const auto& conn : stationConns) 
        {
            std::cout << "  - " << conn.houseName << " (расстояние: " 
                      << std::fixed << std::setprecision(1) << conn.distance << ")" << std::endl;
        }
    }
    std::cout << std::endl;
}
void MapHandler::process(const std::string& fileName) 
{
    try 
    {
        readMapData(fileName);
        
        handleObjects();
        printObjects();
        
        handleConnections();
        printConnections();
        
    } 
    catch (const std::exception& ex) 
    {
        std::cerr << "Ошибка обработки карты: " << ex.what() << std::endl;
        throw;
    }
}

void MapHandler::processQuery(const std::string& stationName) const
{
    auto stationIt = std::find_if(stations.begin(), stations.end(),
                                [&stationName](const Station& station) 
                                {
                                    return station.name == stationName;
                                });
                            
    if (stationIt == stations.end()) 
    {
        throw std::runtime_error("Станция '" + stationName + "' не найдена");
    }
    
    auto stationConnectionsIt = sortedConnectionsByStation.find(stationName);
    if (stationConnectionsIt == sortedConnectionsByStation.end() || stationConnectionsIt->second.empty()) 
    {
        std::cout << "К станции '" + stationName + "' не подключено ни одного дома" << std::endl;
        return;
    }
    
    const auto& connections = stationConnectionsIt->second;
    
    std::cout << stationName << ": ";
    for (size_t i = 0; i < connections.size(); ++i) 
    {
        std::cout << connections[i].houseName << "(" 
             << std::fixed << std::setprecision(1) << connections[i].distance << ")";
        if (i != connections.size() - 1) 
        {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;
}