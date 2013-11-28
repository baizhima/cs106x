/**
 * File: life.cpp
 * --------------
 * Implements the Game of Life.
 */

#include <iostream>  // for cout
#include <cctype> // for string format check
using namespace std;

#include "console.h" // required of all files that contain the main function
#include "simpio.h"  // for getLine
#include "random.h" // for randomization
#include "gevents.h" // for mouse events
#include "life-constants.h"  // for kMaxAge
#include "life-graphics.h"   // for class LifeDisplay


static void waitForEnter(string message);
static void welcome();
static void mainMenu(Grid<int> & grid, LifeDisplay &display);
static void configuration(Grid<int> & grid, LifeDisplay &display); // starting configuration
static void uiUpdate(const Grid<int> & grid, LifeDisplay & display); // updating UI
static int getSpeed();
static void gridUpdate(Grid<int> & grid);
static void runSimulation(Grid<int>& grid, LifeDisplay& display, int speed);
//static void animation(Grid<int> & grid,LifeDisplay & display, int speed);
static bool stableGeneration(const Grid<int> & grid, int maxAge);



int main() {
    Grid<int> grid;
    LifeDisplay display;
    display.setTitle("Game of Life");
    display.setDimensions(40, 70);
    welcome();
    mainMenu(grid,display);
    return 0;
}

static void waitForEnter(string message) {
    cout << message;
    (void) getLine();

}

static void welcome() {
    cout << "Welcome to the game of Life, a simulation of the lifecycle of a bacteria colony." << endl;
    cout << "Cells live and die by the following rules:" << endl << endl;
    cout << "\tA cell with 1 or fewer neighbors dies of loneliness" << endl;
    cout << "\tLocations with 2 neighbors remain stable" << endl;
    cout << "\tLocations with 3 neighbors will spontaneously create life" << endl;
    cout << "\tLocations with 4 or more neighbors die of overcrowding" << endl << endl;
    cout << "In the animation, new cells are dark and fade to gray as they age." << endl << endl;
    waitForEnter("Hit [enter] to continue....   ");
}

static void mainMenu(Grid<int> & grid, LifeDisplay &display)
{
    while (true)
    {
        bool wantExit = false;
        configuration(grid,display);
        uiUpdate(grid,display);
        int speed = getSpeed();
        runSimulation(grid,display,speed);
        string prompt = getLine("Would you like to run another? ");
        while (true)
        {
            if (equalsIgnoreCase(prompt,"yes"))
            {
                break;
            }
            else if (equalsIgnoreCase(prompt,"no")) // exit here
            {
                wantExit = true;
                cout << endl;
                cout << endl;
                (void)getLine("[Program finished -- Hit RETURN to exit] ");
                break;
            }
            else
            {
                cout << "Please enter \"yes\" or \"no\"." << endl;
                prompt = getLine("Would you like to run another? ");
            }
        }
        if (wantExit)
            break;
    }
}

static void configuration(Grid<int> &grid ,LifeDisplay &display)
{
    cout << "You can start your colony with random cells or read from a prepared file." << endl;
    while (true)
    {
        string filename = getLine("Enter name of colony file (or RETURN to seed randomly): ");
        if (filename == "") // randomly initiate a Grid, from here to exit the while loop
        {
            int nRow = randomInteger(40,60);
            int nCol = randomInteger(40,60);
            grid.resize(nRow,nCol);
            display.setDimensions(nRow,nCol);
            for (int i = 0; i < grid.numRows(); i++)
                for (int j = 0; j < grid.numCols(); j++)
                {
                    if (randomChance(0.5))
                        grid.set(i,j,randomInteger(1,kMaxAge));
                    else
                        grid.set(i,j,0);
                }
            break;
        }
        else // initiate a Grid by reading a colony configuration file
        {
            ifstream input;
            string path = "files/" + filename;
            input.open(path.c_str());
            if (input.fail()) // invalid filename
            {
                cout << "Unable to open the file named \"";
                cout << filename;
                cout << "\".  Please select another file." << endl;

            }
            else // valid filename, from here to exit the while loop
            {
                string line;
                getline(input,line);
                while(line[0] - '#' == 0)
                    getline(input,line);
                int nRow = stringToInteger(line);
                getline(input,line);
                int nCol = stringToInteger(line);
                grid.resize(nRow, nCol);
                display.setDimensions(nRow,nCol);
                for (int i = 0; i < grid.numRows(); i++) // grid initialization
                {
                    getline(input,line);
                    for (int j = 0; j < grid.numCols(); j++)
                    {
                        if (line[j] - '-' == 0)
                            grid.set(i,j,0);
                        else if (line[j] - 'X' == 0)
                            grid.set(i,j,1);
                        else
                            cout << "Error reading in grid entries" << endl;
                    }
                }
                break;
            }

        }
    }
}

static void uiUpdate(const Grid<int> &grid, LifeDisplay &display)
{

    for (int i = 0; i < grid.numRows(); i++)
        for (int j = 0; j < grid.numCols(); j++)
            display.drawCellAt(i,j,grid.get(i,j));

}

static int getSpeed()
{
    int option = -1;
    cout << "You choose how fast to run the simulation." << endl;
    cout << "\t1 = As fast as this chip can go!" << endl;
    cout << "\t2 = Not too fast, this is a school zone." << endl;
    cout << "\t3 = Nice and slow so I can watch everything that happens." << endl;
    cout << "\t4 = Require enter key be pressed before advancing to next generation." << endl;
    string input = getLine("Your choice: ");
    while (true) // check whether input is an integer
    {
        bool inputIsDigit = true;
        if(input == "")
            inputIsDigit = false;
        for (int i = 0; i < (int)input.length(); i++) // check whether input is a numeric string
            if (!isdigit(input[i]))
                inputIsDigit = false;

        if (inputIsDigit)
        {
           int num = stringToInteger(input);
           if (num >= 1 && num <= 4) // exit of while loop
           {
               option = num;
               break;
           }
           else
           {
               cout << "Please enter a number between 1 and 4!" << endl;
               input = getLine("Your choice: ");
           }
        }
        else
        {
            cout << "Illegal integer format. Try again." << endl;
            input = getLine("Enter an integer: ");
        }
    }
    if (option == -1) cout <<"error getting valid option" << endl;
    return option;
}

static void gridUpdate(Grid<int> &grid)
{
    Grid<int> newGrid(grid.numRows(),grid.numCols());
    for (int i = 0; i < grid.numRows(); i++)
        for (int j = 0; j < grid.numCols(); j++)
        {
            int neighborCount = 0;
            for (int p = -1; p <= 1; p++)
                for (int q = -1; q <= 1; q++)
                {
                    // Ignore counting either grids out of bounds or itself
                    if (!grid.inBounds(i+p,j+q) || (p == 0 && q == 0)) continue;
                    int neighborGridVal =  grid.get(i+p,j+q);
                    if (neighborGridVal >= 1)
                        neighborCount++;
                }
            // A location that has zero or one neighbors will be empty in the next generation.
            if (neighborCount == 0 || neighborCount == 1)
                newGrid.set(i,j,0);
            // A location with two neighbors is stable.
            else if (neighborCount == 2)
            {
                if (grid.get(i,j) == 0)
                    newGrid.set(i,j,0);
                else
                    newGrid.set(i,j,grid.get(i,j) + 1);
            }
            // A location with three neighbors will contain a cell in the next generation.
            else if (neighborCount == 3)
                newGrid.set(i,j,grid.get(i,j) + 1);
            // A location with four or more neighbors will be empty in the next generation.
            else if (neighborCount >= 4)
                newGrid.set(i,j,0);
        }
    grid = newGrid; // replace the old generation grid with the new one
}

static void runSimulation(Grid<int>& grid, LifeDisplay& display, int speed)
{
    while (true) {
        GMouseEvent me = getNextEvent(MOUSE_EVENT);
        if (me.getEventType() == MOUSE_CLICKED) {
            return;
        } else if (me.getEventType() == NULL_EVENT) {
            // only advance board if there aren’t any outstanding mouse events
            gridUpdate(grid);
            uiUpdate(grid,display);
            if (speed == 4)
            {
                waitForEnter("");
            }
            else
            {
                int pausetime = 0;
                pausetime = speed  * 25;
                pause(pausetime);
            }
            if (stableGeneration(grid,kMaxAge))
                break;
        }
    }

}
/*
static void animation(Grid<int> &grid,LifeDisplay &display, int speed)
{
        gridUpdate(grid);
        uiUpdate(grid,display);
        if (speed == 4)
        {
            waitForEnter("");
        }
        else
        {
            int pausetime = 0;
            pausetime = speed  * 25;
            pause(pausetime);
        }

}
*/
static bool stableGeneration(const Grid<int> & grid, int maxAge)
{
    Grid<int> newGrid;
    newGrid = grid;
    gridUpdate(newGrid);
    for (int i = 0; i < grid.numRows();i++)
        for (int j = 0; j < grid.numCols(); j++)
        {
            // die cell in both this and next generation, cell in entry(i,j) is stable
            if (grid.get(i,j) == 0 && newGrid.get(i,j) == 0)
                continue;
            // in either generation entry has living cell, cell in entry(i,j) is unstable
            else if ((grid.get(i,j) > 0 || newGrid.get(i,j) > 0)
                    && (grid.get(i,j) * newGrid.get(i,j) == 0))
                return false;
            // cell in entry(i,j) is under maxAge, which is unstable
            else if (grid.get(i,j) > 0 && newGrid.get(i,j) > 0)
            {
                if (grid.get(i,j) < maxAge)
                    return false;
            }
            // unexpected situation, left for debug
            else
            {
                cout << "unexpected condition like this: current(i,j),next(i,j):" << endl;
                cout << grid.get(i,j) << endl;
                cout << newGrid.get(i,j) << endl;
            }
        }
    return true;
}
