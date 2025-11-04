#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <map> 

/// Типы ячеек карты
enum class CellType : uint8_t
{
    EMPTY = 0,   ///< Пустая ячейка
    HOUSE = 1,   ///< Ячейка дома
    STATION = 2  ///< Ячейка станции
};

/// Точка в 2D пространстве (целочисленные координаты)
struct Point
{
    int x;  ///< Координата X
    int y;  ///< Координата Y
};

/// Точка в 2D пространстве (вещественные координаты)
struct PointF
{
    double x;  ///< Координата X
    double y;  ///< Координата Y
};

/// Дом на карте
struct House
{
    std::string name;  ///< Уникальное имя дома
    Point position;    ///< Левая верхняя координата дома
    PointF center;     ///< Геометрический центр дома
    int width;         ///< Ширина дома в пикселях
    int height;        ///< Высота дома в пикселях
};

/// Станция на карте
struct Station
{
    std::string name;  ///< Уникальное имя станции
    Point position;    ///< Позиция станции (целые координаты для вывода)
    PointF center;     ///< Геометрический центр для вычислений
};

/// Подключение дома к станции
struct Connection
{
    std::string houseName;    ///< Имя подключенного дома
    std::string stationName;  ///< Имя станции
    double distance;          ///< Расстояние между домом и станцией
};

/// Обработчик карты для преобразования растровых данных в векторные
///
/// Класс предоставляет функциональность для чтения бинарных карт,
/// поиска домов и станций, установления связей между ними и выполнения запросов.
class MapHandler
{
private:
    uint32_t width;                                                             ///< Ширина карты
    uint32_t height;                                                            ///< Высота карты
    std::vector<std::vector<CellType>> mapData;                                 ///< Данные карты
    std::vector<House> houses;                                                  ///< Список найденных домов
    std::vector<Station> stations;                                              ///< Список найденных станций
    std::vector<Connection> connections;                                        ///< Список подключений домов к станциям
    std::map<std::string, std::vector<Connection>> sortedConnectionsByStation;  ///< Отсортированные подключения разделенные по станциям
    
    void readMapData(const std::string& fileName); 
    void connectHousesToStations();
    void findHouses();
    void findStations();
    void handleObjects();
    void handleConnections();
    void groupSortedConnectionsByStation();

    void printObjects() const;
    void printConnections() const;

    std::string generateUniqueHouseName() const;
    std::string generateUniqueStationName() const;

    House getHouseFromPixels(const std::vector<Point>& pixels) const;
    std::vector<Point> getHousePixels(int startX, int startY, std::vector<std::vector<bool>>& visited) const;
public:
    /// Выполняет полную обработку карты: чтение файла, поиск домов и станций, установление связей между ними.
    ///    
    /// @param filename Путь к бинарному файлу карты
    /// @throws std::runtime_error При ошибках чтения файла или обработки данных
    /// @throws std::invalid_argument При невалидных параметрах
    void process(const std::string& fileName);
    
    /// Обработка запроса пользователя.
    /// Выводит список домов, подключенных к указанной станции, отсортированный по расстоянию от ближайшего к дальнему.
    ///
    /// @param stationName Имя станции для поиска
    /// @throws std::runtime_error Если станция не найдена
    void processQuery(const std::string& stationName) const;
};