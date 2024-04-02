#include <chrono> 
#include <iostream> 
#include <map> 
#include <mutex> 
#include <thread> 
#include <unordered_map> 

using namespace std; 
using namespace chrono; 

class MapRedis {

private:
    map<int, string> maps; 
    map<int, time_point<system_clock>> mapsExp; 
    mutex mtx; 

public:
    // Метод для установки значения по ключу
    void Set(int key, string value) 
    { 
        lock_guard<mutex> lock(mtx); 
        maps[key] = value; 
    }
    // Метод для получения значения по ключу
    string Get(int key) 
    { 
        lock_guard<mutex> lock(mtx); 
        return maps[key]; 
    }
    // Метод для добавления значения с истекающим сроком действия
    void AddDel(int key, string value) 
    { 
        lock_guard<mutex> lock(mtx); 
        maps[key] = value; 
        mapsExp[key] = system_clock::now() + seconds(2); 
    }
    // Метод для очистки устаревших значений
    void InitCleanup() 
    { 
        lock_guard<mutex> lock(mtx); 
        for (auto it = mapsExp.begin(); it != mapsExp.end(); ) 
        { 
            if (it->second <= system_clock::now()) 
            { 
                maps.erase(it->first); 
                it = mapsExp.erase(it); 
            }
            else {
                ++it; 
            }
        }
    }
    // Метод для получения копии map значений
    map<int, string> GetMap() 
    { 
        lock_guard<mutex> lock(mtx); 
        return maps; 
    }
};
// Функция для генерации неупорядоченной map значений
unordered_map<int, string> GenerateMap(int n, MapRedis& mr) 
{ 
    unordered_map<int, string> mapsGen; 
    for (int i = 2; i < n; i++) 
    { 
        mr.AddDel(i, to_string(i)); 
    }
    return mapsGen; 
}

int main() { 
    auto start = steady_clock::now(); 

    MapRedis mr; 
    mr.Set(1, "one"); 

    auto genMaps = GenerateMap(1000, mr); 

    this_thread::sleep_for(seconds(2)); 

    mr.InitCleanup(); 

    cout << "Maps:" << endl; 
    for (const auto& pair : mr.GetMap()) { 
        cout << pair.first << ": " << pair.second << endl; 
    }

    auto duration = steady_clock::now() - start; 
    cout << "?????????????????: " << duration_cast<milliseconds>(duration).count() << " ms" << endl; 

    return 0; 
}
