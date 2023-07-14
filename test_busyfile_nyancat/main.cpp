#include <iostream>
#include <limits>

void printNyancat()
{
    std::cout << "\033[1;35m";
    std::cout << "⢀⡤⠤⠤⠤⢀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << std::endl;
    std::cout << "⠸⣿⠿⢿⠿⠛⠋⠉⠙⠛⠛⠛⠛⠛⠛⠻⢿⣿⡄⠀⠀⠀" << std::endl;
    std::cout << "⠀⠈⢱⠠⣴⠏⠀⢀⡀⠀⠀⠀⠀⠀⠀⠐⢲⠱⢿⡇⠀⠀⠀" << std::endl;
    std::cout << "⠀⠀⡸⠘⢼⣇⠀⠻⠿⢿⡿⠟⠁⠀⠀⢰⠃⢸⠿⠀⠀⠀" << std::endl;
    std::cout << "⠀⠀⢿⣶⣤⡇⠀⠀⣠⡄⠀⠀⠀⠀⢠⠏⠀⢸⠀⠀⠀⠀" << std::endl;
    std::cout << "⠀⠀⠈⠛⠉⠁⠀⠀⠈⠉⠀⠀⠀⠀⠈⠀⠀⠘⠃⠀⠀⠀⠀" << std::endl;
    std::cout << "\033[0m";
}

int main()
{
    int randomNumber;
    
    while (true)
    {
        // Prompt the user to enter a random number
        std::cout << "Enter a random number: ";
        // Read the input
        if (!(std::cin >> randomNumber))
        {
            // Input failed, clear error flags and discard invalid input
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            
            std::cout << "Invalid input. Please enter an integer." << std::endl;
        }
        else 
        {
            // Input was successful, print the Nyancat if the random number is even
            if (randomNumber % 2)
                printNyancat();
            else
                std::cout << "Bad luck, try again !" << std::endl;
            std::cin.clear();
            randomNumber = 0;
        }
    }
    return 0;
}