#include "demo_data.h"

// OFFICIAL INPUTS: edit these values to run another scenario.
void loadInputs(Theatre& t, std::vector<Screen>& screens, std::vector<Movie>& movies, int& grid) {
    t = {"Revenue Growth Demo", 600, 1380, 15};
    grid = 15;
    screens = {{1,"Screen 1 (IMAX)",280,"IMAX",1.4,true}, {2,"Screen 2 (Premium)",200,"Premium",1.15,true},
               {3,"Screen 3",160,"Standard",1,true}, {4,"Screen 4",140,"Standard",1,true}};
    movies = {{1,"Drama",90,250,9,"Drama"}, {2,"Comedy",120,280,10,"Comedy"},
              {3,"Action",150,140,5,"Action"}, {4,"Thriller",170,180,2,"Thriller"},
              {5,"Family",130,120,3,"Family"}};
}

