#include "MapHandler.h"

#include <iostream>
#include <stdexcept>
#include "MapHandler.h"

#include <iostream>
#include <stdexcept>

namespace 
{

void runApplication(int argc, char** argv) 
{
    if (argc != 2) 
    {
        throw std::invalid_argument("Использование: " + std::string(argv[0]) + " <путь_к_файлу_карты>");
    }
    
    std::string filename = argv[1];
    MapHandler handler;
    
    handler.process(filename);
    
    std::cout << "Введите название станции для поиска подключенных домов (или 'exit' для выхода):" << std::endl;
    
    std::string input;
    while (true) 
    {
        std::cout << ">> ";
        if (!std::getline(std::cin, input)) 
        {
            std::cout << "\nВыход..." << std::endl;
            break;
        }         

        if (input == "exit")
        {
            break;
        }
        
        if (!input.empty()) 
        {
            try 
            {
                handler.processQuery(input);
            } 
            catch (const std::exception& e) 
            {
                std::cout << "Ошибка запроса: " << e.what() << std::endl;
            }
        }
    }
}

} // anonymous namespace

int main(int argc, char** argv) 
{
    try 
    {
        runApplication(argc, argv);
        return 0;
    } 
    catch (const std::exception& ex) 
    {
        std::cerr << "ошибка: " << ex.what() << std::endl;
        return 1;
    }    
}