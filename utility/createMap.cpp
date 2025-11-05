#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>


int main() 
{
    std::ofstream file("test_map.bin", std::ios::binary);
    
    uint32_t width = 20;
    uint32_t height = 15;
    
    file.write(reinterpret_cast<const char*>(&width), sizeof(width));
    file.write(reinterpret_cast<const char*>(&height), sizeof(height));

    std::vector<std::vector<uint8_t>> mapData = {
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0}, 
        {0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1}, 
        {0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1},
        {1,1,1,1,0,2,0,0,1,1,1,1,1,1,0,0,0,0,0,0},  
        {1,1,1,1,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0},
        {1,1,1,1,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  
        {1,1,1,1,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,2,0,0},  
        {0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,2,0,0,0,0,0,0,0,0,2}   
    };
    
    for (uint32_t y = 0; y < height; ++y) 
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            file.write(reinterpret_cast<const char*>(&mapData[y][x]), sizeof(uint8_t));
        }
    }
    
    file.close();
    for (uint32_t y = 0; y < height; ++y) 
    {
        for (uint32_t x = 0; x < width; ++x) 
        {
            if (mapData[y][x] == 0) std::cout << ". ";
            else if (mapData[y][x] == 1) std::cout << "* ";
            else if (mapData[y][x] == 2) std::cout << "S ";
        }
        std::cout << std::endl;
    }
    
    return 0;
}